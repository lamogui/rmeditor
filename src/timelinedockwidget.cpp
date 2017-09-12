﻿#include "timelinedockwidget.hpp"
#include "ui_timelinedockwidget.h"

#include "demotimeline.hpp"
#include "music.hpp"
#include "timelinewidget.hpp"
#include "project.hpp"

TimelineDockWidget::TimelineDockWidget(QWidget *parent):
  QDockWidget(parent),
  ui(new Ui::TimelineDockWidget),
  project(nullptr)
{
    ui->setupUi(this);
    connect(ui->timelineView, &TimelineWidget::currentRendererChanged,this, &TimelineDockWidget::emitCurrentRendererChanged);
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
  this->project = project;
  if (this->project)
  {
    //getTimelineWidget()->setTimeline(this->project->getDemoTimeline());
    assert(false);
    this->setEnabled(true);
  }
  else
  {
    getTimelineWidget()->setTimeline(nullptr);
    this->setEnabled(false);
  }
}

void TimelineDockWidget::on_playPauseButton_clicked(bool)
{
  if (ui->timelineView->getTimeline())
  {
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
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
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
  project->getMusic()->setPosition(0);
}

void TimelineDockWidget::on_stopButton_clicked(bool)
{
  if (ui->timelineView->getTimeline())
  {
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
  project->getMusic()->pause();
  project->getMusic()->setPosition(0);
}

void TimelineDockWidget::emitCurrentRendererChanged(const QWeakPointer<Renderer>& renderer)
{
  emit currentRendererChanged(renderer);
}

void TimelineDockWidget::focusInEvent(QFocusEvent* event)
{
  if (ui->timelineView->getTimeline())
  {
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
}

void TimelineDockWidget::insertCameraKeyframe(Camera *cam)
{
  DemoTimeline* t = qobject_cast<DemoTimeline*>(ui->timelineView->getTimeline());
  if (t)
  {
//    t->insertCameraKeyframe(cam);
    assert(false);
  }
}
