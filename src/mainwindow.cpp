﻿#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

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
    project(nullptr),
    ui(new Ui::MainWindow)

{

    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->cancelButton->setVisible(false);

    info = new LogDockWidget(this);
    info->getLogWidget()->setPrintTime(false);
    ui->renderWidget->setLogWidget((info->getLogWidget()));

    editor = new EditorWidget(*(info->getLogWidget()),this);
    timelineWidget = new TimelineDockWidget(this);
    addDockWidget(Qt::BottomDockWidgetArea, info);
    addDockWidget(Qt::BottomDockWidgetArea, timelineWidget);
    addDockWidget(Qt::LeftDockWidgetArea, editor);

    connect(editor,SIGNAL(rendererChanged(Render*)),this->ui->renderWidget,SLOT(setRender(Render*)));
    connect(timelineWidget,SIGNAL(rendererChanged(Render*)),this->ui->renderWidget,SLOT(setRender(Render*)));

    //Main window actions
    ui->toolBar->addAction(ui->actionNew);
    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addAction(ui->actionSaveAllShaders);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionTake_screenshot);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionInsert_camera_keyframe);
    ui->toolBar->addAction(ui->actionReset_camera);


    connect(ui->actionNew,SIGNAL(triggered()), this, SLOT(newProject()));
    connect(ui->actionOpen,SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionSaveAllShaders, SIGNAL(triggered()), this, SLOT(saveAllShaders()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionReset_camera, SIGNAL(triggered()), ui->renderWidget, SLOT(resetCamera()));
    connect(ui->actionTake_screenshot, SIGNAL(triggered()), ui->renderWidget, SLOT(takeScreenshot()));
    connect(ui->actionExport_as_video,SIGNAL(triggered()),this,SLOT(exportAsVideo()));
    connect(ui->actionExport_as_Linux_Demo, SIGNAL(triggered()), this, SLOT(exportAsLinuxDemo()));
    connect(ui->actionInsert_camera_keyframe,SIGNAL(triggered()),this,SLOT(insertCameraKeyframe()));

    
}

MainWindow::~MainWindow()
{
  if (project)
  {
    delete project;
  }
  delete ui;
}


void MainWindow::newProject()
{

  QString d = QFileDialog::getExistingDirectory(this,tr("Select project directory"),QDir::currentPath());
  if (!d.isEmpty())
  {
    QDir dir = QDir(d);
    if (!QDir::setCurrent(dir.path()))
    {
      info->getLogWidget()->writeError("Fatal Error: Cannot set the current directory to " + dir.path());
    }
    else
    {
      if (project)
      {
        delete project;
      }
      project = new Project(dir,"demo.xml",*(info->getLogWidget()),this);
      connectProject();
      project->build(Project::getDefaultProjectText());
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
      info->getLogWidget()->writeError("Fatal Error: Cannot set the current directory to " + dir.path());
    }
    else
    {
      if (project)
      {
        delete project;
      }
      project = new Project(dir,QFileInfo(f).fileName(),*(info->getLogWidget()),this);
      connectProject();
    }
  }
}

void MainWindow::saveAllShaders()
{
  editor->saveAllShaders();
}

void MainWindow::connectProject()
{
  editor->loadProject(*project);
  connect(project,SIGNAL(appendTextEditable(TextEditable*)),editor,SLOT(appendTextEditable(TextEditable*)));
  connect(project,SIGNAL(demoTimelineChanged(Timeline*)),this,SLOT(setTimeline(Timeline*)));
  timelineWidget->setProject(project);
}

void MainWindow::setTimeline(Timeline *t)
{
  (void)t; // oh !
  Q_ASSERT(t == project->getDemoTimeline() || !t); // Don't support another case yet 
  timelineWidget->setProject(project);
}

void MainWindow::insertCameraKeyframe()
{
  Camera* cam = ui->renderWidget->camera();
  if (cam)
  {
    timelineWidget->insertCameraKeyframe(cam);
  }
}


void MainWindow::exportAsVideo()
{
  if (project && project->getDemoTimeline())
  {
    freezeAll();

    ui->renderWidget->setRender(project->getDemoTimeline()->getRender());

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(project->getDemoTimeline()->length());
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat("Rendering %v/%m (%p%)");
    ui->progressBar->setVisible(true);
    ui->cancelButton->setVisible(true);

    disconnect(ui->cancelButton,SIGNAL(clicked(bool)),0,0);


    FFmpegEncoder* encoder = new FFmpegEncoder(
                               this,
                               QString("render.mp4"),
                               *(project->getDemoTimeline()),
                               QSize(3840,2160),
                               *(info->getLogWidget()));

    connect(ui->cancelButton,SIGNAL(clicked(bool)),encoder,SLOT(cancel()));

    connect(encoder,SIGNAL(newFrameEncoded(int)),ui->progressBar,SLOT(setValue(int)));
    connect(encoder,SIGNAL(finished()),this,SLOT(unfreezeAll()));
    connect(encoder,SIGNAL(finished()),encoder,SLOT(deleteLater()));
    encoder->start(QThread::HighPriority);
  }
}


void MainWindow::freezeAll()
{
  ui->renderWidget->setOnlyShowTexture(true);

  ui->menuBar->setEnabled(false);
  ui->menuCamera->setEnabled(false);
  ui->menuFile->setEnabled(false);
  ui->menuExport->setEnabled(false);

  timelineWidget->setEnabled(false);
  editor->setEnabled(false);

 // timeline->getTimelineWidget()->stopUpdateLoop();

}

void MainWindow::unfreezeAll()
{
  ui->menuBar->setEnabled(true);
  ui->menuCamera->setEnabled(true);
  ui->menuFile->setEnabled(true);
  ui->menuExport->setEnabled(true);

  timelineWidget->setEnabled(true);
  editor->setEnabled(true);

  ui->renderWidget->setOnlyShowTexture(false);

  ui->progressBar->setVisible(false);
  ui->cancelButton->setVisible(false);
  //timeline->getTimelineWidget()->startUpdateLoop();
}

void MainWindow::exportAsLinuxDemo()
{
  if (project)
  {
    QString d = QFileDialog::getExistingDirectory(this,tr("Select demo build directory"));
    if (!d.isEmpty())
    {
      project->exportAsLinuxDemo(QDir(d));
    }
  }
}
