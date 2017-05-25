#include "timelinedockwidget.hpp"
#include "ui_timelinedockwidget.h"

#include "demotimeline.hpp"
#include "music.hpp"
#include "timelinewidget.hpp"
#include "project.hpp"

TimelineDockWidget::TimelineDockWidget(QWidget *parent):
  QDockWidget(parent),
  ui(new Ui::TimelineDockWidget),
  project(NULL)
{
    ui->setupUi(this);
    connect(ui->timelineView,SIGNAL(rendererChanged(Renderer*)),this,SLOT(emitRendererChanged(Renderer*)));
    this->setEnabled(false);
}

TimelineDockWidget::~TimelineDockWidget()
{
    delete ui;
}

TimelineWidget* TimelineDockWidget::getTimelineWidget() const
{
  return ui->timelineView;
}

void TimelineDockWidget::setProject(Project *project)
{
  project = project;
  if (project)
  {
    getTimelineWidget()->setTimeline(project->getDemoTimeline());
    this->setEnabled(true);
  }
  else
  {
    getTimelineWidget()->setTimeline(NULL);
    this->setEnabled(false);
  }
}

void TimelineDockWidget::on_playPauseButton_clicked(bool)
{
  if (ui->timelineView->getTimeline())
  {
    emitRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
  if (project->getMusic()->isPlaying())
  {
    project->getMusic()->pause();
  }
  else
  {
    project->getMusic()->play();
  }
}

void TimelineDockWidget::on_rewindButton_clicked(bool)
{
  if (ui->timelineView->getTimeline())
  {
    emitRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
  project->getMusic()->setPosition(0);
}

void TimelineDockWidget::on_stopButton_clicked(bool)
{
  if (ui->timelineView->getTimeline())
  {
    emitRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
  project->getMusic()->pause();
  project->getMusic()->setPosition(0);
}

void TimelineDockWidget::emitRendererChanged(Renderer *renderer)
{
  emit rendererChanged(renderer);
}

void TimelineDockWidget::focusInEvent(QFocusEvent* event)
{
  if (ui->timelineView->getTimeline())
  {
    emitRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
}

void TimelineDockWidget::insertCameraKeyframe(Camera *cam)
{
  DemoTimeline* t = qobject_cast<DemoTimeline*>(ui->timelineView->getTimeline());
  if (t)
  {
    t->insertCameraKeyframe(cam);
  }
}
