#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

#include "mainwindow.hpp"


#include "logmanager.hpp"
#include "mediafileseditorwidget.hpp"
#include "logdockwidget.hpp"
#include "project.hpp"
#include "render.hpp"   // for connect
#include "renderer.hpp" // for connect
#include "demotimeline.hpp" // for QMetaType (so connect I imagine ?)
#include "demotimelinewidget.hpp"
#include "demotimelinedockwidget.hpp"

/*
#include "ffmpegencoder.hpp"
#include "music.hpp"
#include "timelinewidget.hpp"
#include "timelinedockwidget.hpp"
*/


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
		m_project(nullptr)
{
    m_ui.setupUi(this);
    m_ui.progressBar->setVisible(false);
    m_ui.cancelButton->setVisible(false);

    m_info = new LogDockWidget(this);
    m_info->getLogWidget()->setPrintTime(false);
    connect(LogManager::get(), &LogManager::assertion, info->getLogWidget(), &LogWidget::writeError);
    connect(LogManager::get(), &LogManager::error, info->getLogWidget(), &LogWidget::writeError);
    connect(LogManager::get(), &LogManager::warning, info->getLogWidget(), &LogWidget::writeWarning);
    connect(LogManager::get(), &LogManager::info, info->getLogWidget(), &LogWidget::writeInfo);

    m_editor = new EditorWidget(*(m_info->getLogWidget()),this);
    m_timeline = new TimelineDockWidget(this);
    addDockWidget(Qt::BottomDockWidgetArea, m_info);
    addDockWidget(Qt::BottomDockWidgetArea, m_timeline);
    addDockWidget(Qt::LeftDockWidgetArea, m_editor);

    connect(m_editor,SIGNAL(rendererChanged(Renderer*)),this->m_ui.widget,SLOT(setRenderer(Renderer*)));
    connect(m_timeline,SIGNAL(rendererChanged(Renderer*)),this->m_ui.widget,SLOT(setRenderer(Renderer*)));

    //Main window actions
    m_ui.toolBar->addAction(m_ui.actionNew);
    m_ui.toolBar->addAction(m_ui.actionOpen);
    m_ui.toolBar->addAction(m_ui.actionSaveAllShaders);
    m_ui.toolBar->addSeparator();
    m_ui.toolBar->addAction(m_ui.actionTake_screenshot);
    m_ui.toolBar->addSeparator();
    m_ui.toolBar->addAction(m_ui.actionInsert_camera_keyframe);
    m_ui.toolBar->addAction(m_ui.actionReset_camera);

    connect(m_ui.actionNew, &QAction::triggered, this, &MainWindow::newProject);
    connect(m_ui.actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(m_ui.actionSaveAllShaders, &QAction::triggered, this, &MainWindow::saveAllShaders);
    connect(m_ui.actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui.actionReset_camera, &QAction::triggered, m_ui.renderWidget, &RenderWidget::resetCamera);
    connect(m_ui.actionTake_screenshot, &QAction::triggered, m_ui.renderWidget, &RenderWidget::takeScreenshot);
    connect(m_ui.actionExport_as_video, &QAction::triggered, this, &MainWindow::exportAsVideo);
    connect(m_ui.actionExport_as_Linux_Demo, &QAction::triggered, this, &MainWindow::exportAsLinuxDemo);
    //connect(m_ui.actionInsert_camera_keyframe, &QAction::triggered, this, &MainWindow::insertCameraKeyframe);
    connect(m_ui.actionSwitch_displayed_texture, &QAction::triggered, m_ui.renderWidget, &RenderWidget::switchDisplayedTexture);

    showFullScreen();
}

MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::newProject()
{
  /*
  QString d = QFileDialog::getExistingDirectory(this,tr("Select project directory"),QDir::currentPath());
  if (!d.isEmpty())
  {
    QDir dir = QDir(d);
    if (!QDir::setCurrent(dir.path()))
    {
      m_info->getLogWidget()->writeError("Fatal Error: Cannot set the current directory to " + dir.path());
    }
    else
    {
      if (m_project)
      {
        delete m_project;
      }
      m_project = new Project(dir,"demo.xml",*(m_info->getLogWidget()),this);
      connectProject();
      m_project->build(Project::getDefaultProjectText());
    }
  }
  */
}

#include "scene.hpp"

void MainWindow::open()
{ 
  QString f = QFileDialog::getOpenFileName(this, tr("Open project file"), QString(), "*.xml");
  if (!f.isEmpty())
  {
    QDir dir = QFileInfo(f).dir();
    if (!QDir::setCurrent(dir.path()))
    {
      m_info->getLogWidget()->writeError("Fatal Error: Cannot set the current directory to " + dir.path());
    }
    else
    {
      resetProject();
      project = new Project(this);
      project->setPath(QFileInfo(f));
      initializeProject();
    }
  }
  
}

void MainWindow::saveAllShaders()
{
  m_editor->saveAllShaders();
}


void MainWindow::resetProject()
{
  if (project)
  {
    delete project;
  }
}

void MainWindow::initializeProject()
{
  m_ui.renderWidget->makeCurrent();
  project->initializeGL(m_ui.renderWidget->getRenderFunctions());
  m_ui.renderWidget->doneCurrent();

  editor->appendMediaFile(project);
  StringMap<MediaFile*>::const_iterator it;
  for (it = project->getMediaFiles().constBegin(); it != project->getMediaFiles().constEnd(); ++it)
  {
    editor->appendMediaFile(it.value());
  }

  Music* music = project->getMusic();
  Timeline* timeline = music ? music->getMainTimeline() : nullptr;
  timelineWidget->setTargetTimeline(timeline);

  connect(project, &Project::mediaFileInserted, editor, &MediaFilesEditorWidget::appendMediaFile);
  connect(project, &Project::mainTimelineChanged, timelineWidget, &TimelineDockWidget::setTargetTimeline);
}

void MainWindow::insertCameraKeyframe()
{
  Renderer* currentRenderer = m_ui.renderWidget->getCurrentRenderer().data();
  Camera* cam = currentRenderer ? currentRenderer->getCurrentCamera() : nullptr;
  if (cam)
  {
    m_timeline->insertCameraKeyframe(cam);
  }
}


void MainWindow::exportAsVideo()
{/*
  if (m_project && m_project->demoTimeline())
  {
    freezeAll();

    m_ui.widget->setRenderer(m_project->demoTimeline()->getRenderer());

    m_ui.progressBar->setMinimum(0);
    m_ui.progressBar->setMaximum(m_project->demoTimeline()->length());
    m_ui.progressBar->setValue(0);
    m_ui.progressBar->setFormat("Rendering %v/%m (%p%)");
    m_ui.progressBar->setVisible(true);
    m_ui.cancelButton->setVisible(true);

    disconnect(m_ui.cancelButton,SIGNAL(clicked(bool)),0,0);


    FFmpegEncoder* encoder = new FFmpegEncoder(
                               this,
                               QString("render.mp4"),
                               *(m_project->demoTimeline()),
                               QSize(1920,1080),
                               *(m_info->getLogWidget()));

    connect(m_ui.cancelButton,SIGNAL(clicked(bool)),encoder,SLOT(cancel()));

    connect(encoder,SIGNAL(newFrameEncoded(int)),m_ui.progressBar,SLOT(setValue(int)));
    connect(encoder,SIGNAL(finished()),this,SLOT(unfreezeAll()));
    connect(encoder,SIGNAL(finished()),encoder,SLOT(deleteLater()));
    encoder->start(QThread::HighPriority);
  }*/
}


void MainWindow::freezeAll()
{
  m_ui.renderWidget->setOnlyShowTexture(true);

  m_ui.menuBar->setEnabled(false);
  m_ui.menuCamera->setEnabled(false);
  m_ui.menuFile->setEnabled(false);
  m_ui.menuExport->setEnabled(false);

  m_timeline->setEnabled(false);
  m_editor->setEnabled(false);

 // timeline->getTimelineWidget()->stopUpdateLoop();
}

void MainWindow::unfreezeAll()
{
  m_ui.menuBar->setEnabled(true);
  m_ui.menuCamera->setEnabled(true);
  m_ui.menuFile->setEnabled(true);
  m_ui.menuExport->setEnabled(true);

  m_timeline->setEnabled(true);
  m_editor->setEnabled(true);

  m_ui.renderWidget->setOnlyShowTexture(false);

  m_ui.progressBar->setVisible(false);
  m_ui.cancelButton->setVisible(false);
  //m_timeline->getTimelineWidget()->startUpdateLoop();
}

void MainWindow::exportAsLinuxDemo()
{
  if (m_project)
  {
    QString d = QFileDialog::getExistingDirectory(this,tr("Select demo build directory"));
    if (!d.isEmpty())
    {
      m_project->exportAsLinuxDemo(QDir(d));
    }
  }
}
