#include "timelinedockwidget.hpp"
#include "ui_timelinedockwidget.h"

#include "demotimeline.hpp"
#include "music.hpp"
#include "timelinewidget.hpp"
#include "project.hpp"

TimelineDockWidget::TimelineDockWidget(QWidget *parent):
  QDockWidget(parent),
  ui(new Ui::TimelineDockWidget),
  m_project(NULL)
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
  m_project = project;
  if (m_project)
  {
    getTimelineWidget()->setTimeline(m_project->demoTimeline());
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
  if (ui->timelineView->timeline())
  {
    emitRendererChanged(ui->timelineView->timeline()->getRenderer());
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
    emitRendererChanged(ui->timelineView->timeline()->getRenderer());
  }
  m_project->music()->setPosition(0);
}

void TimelineDockWidget::on_stopButton_clicked(bool)
{
  if (ui->timelineView->timeline())
  {
    emitRendererChanged(ui->timelineView->timeline()->getRenderer());
  }
  m_project->music()->pause();
  m_project->music()->setPosition(0);
}

void TimelineDockWidget::emitRendererChanged(Renderer *renderer)
{
  emit rendererChanged(renderer);
}

void TimelineDockWidget::focusInEvent(QFocusEvent* event)
{
  if (ui->timelineView->timeline())
  {
    emitRendererChanged(ui->timelineView->timeline()->getRenderer());
  }
}

void TimelineDockWidget::insertCameraKeyframe(Camera *cam)
{
  DemoTimeline* t = qobject_cast<DemoTimeline*>(ui->timelineView->timeline());
  if (t)
  {
    t->insertCameraKeyframe(cam);
  }
}
