#include "timelinedockwidget.hpp"
#include "ui_timelinedockwidget.h"

#include "demotimeline.hpp"
#include "music.hpp"
#include "timelinewidget.hpp"
#include "project.hpp"

TimelineDockWidget::TimelineDockWidget(QWidget *parent):
  QDockWidget(parent),
  ui(new Ui::TimelineDockWidget),
  m_project(nullptr)
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

void TimelineDockWidget::setTargetTimeline(Timeline* timeline)
{
  DemoTimeline* demoTimeline = qobject_cast<DemoTimeline*>(timeline);
  if (demoTimeline)
  {
    getTimelineWidget()->setTimeline(demoTimeline);
    this->setEnabled(true);
  }
  else
  {
    getTimelineWidget()->setTimeline(nullptr);
    this->setEnabled(false);
  }
}

Timeline* TimelineDockWidget::getTargetTimeline() const
{
  TimelineWidget* widget = getTimelineWidget();
  return widget ? widget->getTimeline() : nullptr;
}

void TimelineDockWidget::on_playPauseButton_clicked(bool)
{
  Timeline* timeline = getTargetTimeline();
  if (timeline)
  {
    if (timeline->getRenderer())
    {
      emitCurrentRendererChanged(timeline->getRenderer());
    }
    Music& music = timeline->getMusic();
    if (music.isPlaying())
    {
      music.pause();
    }
    else
    {
      music.play();
    }
  }
  if (m_project->music()->playing())
  {
    m_project->music()->pause();
  }
  else
  {
    m_project->music()->play();
  }
}

void TimelineDockWidget::on_rewindButton_clicked(bool)
{
  Timeline* timeline = getTargetTimeline();
  if (timeline)
  {
    if (timeline->getRenderer())
      emitCurrentRendererChanged(timeline->getRenderer());
    timeline->getMusic().setPosition(0);
  }
  else
    jassertfalse; //button should be locked

}

void TimelineDockWidget::on_stopButton_clicked(bool)
{
  Timeline* timeline = getTargetTimeline();
  if (timeline)
  {
    if (timeline->getRenderer())
      emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());

    Music& music = timeline->getMusic();
    music.pause();
    music.setPosition(0);
  }
}

void TimelineDockWidget::emitCurrentRendererChanged(const QWeakPointer<Renderer>& renderer)
{
  emit currentRendererChanged(renderer);
}

void TimelineDockWidget::focusInEvent(QFocusEvent* event)
{
  Timeline* timeline = getTargetTimeline();
  if (timeline && timeline->getRenderer())
  {
    emitCurrentRendererChanged(timeline->getRenderer());
  }
}

void TimelineDockWidget::insertCameraKeyframe(Camera *cam)
{
  DemoTimeline* t = qobject_cast<DemoTimeline*>(ui->timelineView->timeline());
  if (t)
  {
//    t->insertCameraKeyframe(cam);
    jassertfalse; // todo
  }
}
