#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

#include "mainwindow.hpp"

#include "demotimeline.hpp"
#include "editorwidget.hpp"
#include "ffmpegencoder.hpp"
#include "logdockwidget.hpp"
#include "music.hpp"
#include "project.hpp"
#include "timelinewidget.hpp"
#include "timelinedockwidget.hpp"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_project(nullptr)
{
	  m_ui.setupUi(this);
		m_ui.progressBar->setVisible(false);
		m_ui.cancelButton->setVisible(false);

    m_info = new LogDockWidget(this);
    m_info->getLogWidget()->setPrintTime(false);

		m_editor = new EditorWidget(this);
    m_timeline = new TimelineDockWidget(this);
    addDockWidget(Qt::BottomDockWidgetArea, m_info);
    addDockWidget(Qt::BottomDockWidgetArea, m_timeline);
    addDockWidget(Qt::LeftDockWidgetArea, m_editor);

		connect(m_editor,SIGNAL(rendererChanged(Renderer*)),m_ui.renderWidget,SLOT(setRenderer(Renderer*)));
		connect(m_timeline,SIGNAL(rendererChanged(Renderer*)),m_ui.renderWidget,SLOT(setRenderer(Renderer*)));

    //Main window actions
		m_ui.toolBar->addAction(m_ui.actionNew);
		m_ui.toolBar->addAction(m_ui.actionOpen);
		m_ui.toolBar->addAction(m_ui.actionSaveAllShaders);
		m_ui.toolBar->addSeparator();
		m_ui.toolBar->addAction(m_ui.actionTake_screenshot);
		m_ui.toolBar->addSeparator();
		m_ui.toolBar->addAction(m_ui.actionInsert_camera_keyframe);
		m_ui.toolBar->addAction(m_ui.actionReset_camera);


		connect(m_ui.actionNew,SIGNAL(triggered()), this, SLOT(newProject()));
		connect(m_ui.actionOpen,SIGNAL(triggered()), this, SLOT(open()));
		connect(m_ui.actionSaveAllShaders, SIGNAL(triggered()), this, SLOT(saveAllShaders()));
		connect(m_ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));
		connect(m_ui.actionReset_camera, SIGNAL(triggered()), m_ui.renderWidget, SLOT(resetCamera()));
		connect(m_ui.actionTake_screenshot, SIGNAL(triggered()), m_ui.renderWidget, SLOT(takeScreenshot()));
		connect(m_ui.actionExport_as_video,SIGNAL(triggered()),this,SLOT(exportAsVideo()));
		connect(m_ui.actionExport_as_Linux_Demo,SIGNAL(triggered()),this,SLOT(exportAsLinuxDemo()));
		connect(m_ui.actionInsert_camera_keyframe,SIGNAL(triggered()),this,SLOT(insertCameraKeyframe()));

    showFullScreen();
}

MainWindow::~MainWindow()
{
  if (m_project)
  {
    delete m_project;
	}
}


void MainWindow::newProject()
{
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
}

void MainWindow::open()
{
  QString f = QFileDialog::getOpenFileName(this,tr("Open project file"),QString(),"*.xml");
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
      m_project = new Project(dir,QFileInfo(f).fileName(),*(m_info->getLogWidget()),this);
      connectProject();
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
	Camera* cam =m_ui.renderWidget->camera();
  if (cam)
  {
    m_timeline->insertCameraKeyframe(cam);
  }
}


void MainWindow::exportAsVideo()
{
	if (m_project && m_project->demoTimeline())
	{
		freezeAll();

		m_ui.renderWidget->setRenderer(m_project->demoTimeline()->getRenderer());

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
															 QSize(1920,1080));

		connect(m_ui.cancelButton,SIGNAL(clicked(bool)),encoder,SLOT(cancel()));

		connect(encoder,SIGNAL(newFrameEncoded(int)),m_ui.progressBar,SLOT(setValue(int)));
		connect(encoder,SIGNAL(finished()),this,SLOT(unfreezeAll()));
		connect(encoder,SIGNAL(finished()),encoder,SLOT(deleteLater()));
		encoder->start(QThread::HighPriority);
	}
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

	// m_timeline->getTimelineWidget()->stopUpdateLoop();

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
