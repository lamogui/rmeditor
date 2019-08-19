#include "demotimelinedockwidget.hpp"

#include "music.hpp"
#include "demotimelinewidget.hpp"
#include "project.hpp"

TimelineDockWidget::TimelineDockWidget(QWidget *parent):
	QDockWidget(parent),
  m_project(nullptr)
{
		m_ui.setupUi(this);
		connect(m_ui.timelineView, &TimelineWidget::currentRendererChanged,this, &TimelineDockWidget::emitCurrentRendererChanged);
    this->setEnabled(false);
}

TimelineWidget* TimelineDockWidget::getTimelineWidget() const
{
	return m_ui.timelineView;
}

void TimelineDockWidget::setProject(Project *_project)
{
  DemoTimeline* demoTimeline = qobject_cast<DemoTimeline*>(timeline);
	if (_project)
  {
		getTimelineWidget()->setTimeline();
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
			emitCurrentRendererChanged(m_ui.timelineView->getTimeline()->getRenderer());

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
	DemoTimeline* t = qobject_cast<DemoTimeline*>(m_ui.timelineView->timeline());
  if (t)
  {
//    t->insertCameraKeyframe(cam);
    jassertfalse; // todo
  }
}
