#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "logmanager.hpp"
#include "mediafileseditorwidget.hpp"
#include "logdockwidget.hpp"
#include "project.hpp"
#include "render.hpp"   // for connect
#include "renderer.hpp" // for connect
#include "timelinewidget.hpp"
#include "timelinedockwidget.hpp"

/*
#include "ffmpegencoder.hpp"
#include "music.hpp"
#include "timelinewidget.hpp"
#include "timelinedockwidget.hpp"
*/


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_project(nullptr),
    ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->cancelButton->setVisible(false);

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

    connect(m_editor,SIGNAL(rendererChanged(Renderer*)),this->ui->widget,SLOT(setRenderer(Renderer*)));
    connect(m_timeline,SIGNAL(rendererChanged(Renderer*)),this->ui->widget,SLOT(setRenderer(Renderer*)));

    //Main window actions
    ui->toolBar->addAction(ui->actionNew);
    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addAction(ui->actionSaveAllShaders);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionTake_screenshot);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionInsert_camera_keyframe);
    ui->toolBar->addAction(ui->actionReset_camera);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newProject);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionSaveAllShaders, &QAction::triggered, this, &MainWindow::saveAllShaders);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionReset_camera, &QAction::triggered, ui->renderWidget, &RenderWidget::resetCamera);
    connect(ui->actionTake_screenshot, &QAction::triggered, ui->renderWidget, &RenderWidget::takeScreenshot);
    connect(ui->actionExport_as_video, &QAction::triggered, this, &MainWindow::exportAsVideo);
    connect(ui->actionExport_as_Linux_Demo, &QAction::triggered, this, &MainWindow::exportAsLinuxDemo);
    //connect(ui->actionInsert_camera_keyframe, &QAction::triggered, this, &MainWindow::insertCameraKeyframe);
    connect(ui->actionSwitch_displayed_texture, &QAction::triggered, ui->renderWidget, &RenderWidget::switchDisplayedTexture);

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
      if (m_project)
      {
        delete m_project;
      }
      project = new Project(this);
      project->setPath(QFileInfo(f));
      ui->renderWidget->makeCurrent();
      project->initializeGL(ui->renderWidget->getRenderFunctions());
      ui->renderWidget->doneCurrent();

      editor->appendMediaFile(project);
      StringMap<MediaFile*>::const_iterator it; 
      for (it = project->getMediaFiles().constBegin(); it != project->getMediaFiles().constEnd(); ++it)
      {
        editor->appendMediaFile(it.value());
      }

      connect(project, &Project::mediaFileInserted, editor, &MediaFilesEditorWidget::appendMediaFile);

    }
  }
  
}

void MainWindow::saveAllShaders()
{
  m_editor->saveAllShaders();
}


void MainWindow::connectProject()
{
  m_editor->loadProject(*m_project);
  connect(m_project,SIGNAL(appendTextEditable(TextEditable*)),m_editor,SLOT(appendTextEditable(TextEditable*)));
  connect(m_project,SIGNAL(demoTimelineChanged(Timeline*)),this,SLOT(setTimeline(Timeline*)));
  m_timeline->setProject(m_project);
}

void MainWindow::setTimeline(Timeline *t)
{
  (void)t;
  m_timeline->setProject(m_project);
}

void MainWindow::insertCameraKeyframe()
{
  Renderer* currentRenderer = ui->renderWidget->getCurrentRenderer().data();
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

    ui->widget->setRenderer(m_project->demoTimeline()->getRenderer());

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(m_project->demoTimeline()->length());
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat("Rendering %v/%m (%p%)");
    ui->progressBar->setVisible(true);
    ui->cancelButton->setVisible(true);

    disconnect(ui->cancelButton,SIGNAL(clicked(bool)),0,0);


    FFmpegEncoder* encoder = new FFmpegEncoder(
                               this,
                               QString("render.mp4"),
                               *(m_project->demoTimeline()),
                               QSize(1920,1080),
                               *(m_info->getLogWidget()));

    connect(ui->cancelButton,SIGNAL(clicked(bool)),encoder,SLOT(cancel()));

    connect(encoder,SIGNAL(newFrameEncoded(int)),ui->progressBar,SLOT(setValue(int)));
    connect(encoder,SIGNAL(finished()),this,SLOT(unfreezeAll()));
    connect(encoder,SIGNAL(finished()),encoder,SLOT(deleteLater()));
    encoder->start(QThread::HighPriority);
  }*/
}


void MainWindow::freezeAll()
{
  ui->renderWidget->setOnlyShowTexture(true);

  ui->menuBar->setEnabled(false);
  ui->menuCamera->setEnabled(false);
  ui->menuFile->setEnabled(false);
  ui->menuExport->setEnabled(false);

  m_timeline->setEnabled(false);
  m_editor->setEnabled(false);

 // timeline->getTimelineWidget()->stopUpdateLoop();
}

void MainWindow::unfreezeAll()
{
  ui->menuBar->setEnabled(true);
  ui->menuCamera->setEnabled(true);
  ui->menuFile->setEnabled(true);
  ui->menuExport->setEnabled(true);

  m_timeline->setEnabled(true);
  m_editor->setEnabled(true);

  ui->renderWidget->setOnlyShowTexture(false);

  ui->progressBar->setVisible(false);
  ui->cancelButton->setVisible(false);
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
  */
}
