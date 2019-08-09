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

void TimelineDockWidget::setProject(Project *project)
{
  m_project = project;
  if (m_project)
  {
    //getTimelineWidget()->setTimeline(this->project->getDemoTimeline());
    jassertfalse;
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
  if (ui->timelineView->timeline())
  {
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
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
  if (ui->timelineView->timeline())
  {
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
  m_project->music()->setPosition(0);
}

void TimelineDockWidget::on_stopButton_clicked(bool)
{
  if (ui->timelineView->timeline())
  {
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
  m_project->music()->pause();
  m_project->music()->setPosition(0);
}

void TimelineDockWidget::emitCurrentRendererChanged(const QWeakPointer<Renderer>& renderer)
{
  emit currentRendererChanged(renderer);
}

void TimelineDockWidget::focusInEvent(QFocusEvent* event)
{
  if (ui->timelineView->timeline())
  {
    emitCurrentRendererChanged(ui->timelineView->getTimeline()->getRenderer());
  }
}

void TimelineDockWidget::insertCameraKeyframe(Camera *cam)
{
  DemoTimeline* t = qobject_cast<DemoTimeline*>(ui->timelineView->timeline());
  if (t)
  {
//    t->insertCameraKeyframe(cam);
    jassertfalse;
  }
}
