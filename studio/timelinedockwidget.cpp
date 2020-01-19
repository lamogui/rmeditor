#include "timelinedockwidget.hpp"


#include "demotimeline.hpp"
#include "music.hpp"
#include "timelinewidget.hpp"
#include "project.hpp"
#include "styles.hpp"

TimelineDockWidget::TimelineDockWidget(QWidget * _parent):
	QDockWidget(_parent),
  m_project(nullptr)
{
	m_ui.setupUi(this);
	connect(m_ui.timelineView,SIGNAL(rendererChanged(Renderer*)),this,SLOT(emitRendererChanged(Renderer*)));
	this->setEnabled(false);

	SetupDockWidgetStyle( *this );

	SetupPushButtonWidgetStyle( *m_ui.rewindButton );
	SetupPushButtonWidgetStyle( *m_ui.stopButton );
	SetupPushButtonWidgetStyle( *m_ui.playPauseButton );
}

TimelineDockWidget::~TimelineDockWidget()
{
}

TimelineWidget* TimelineDockWidget::getTimelineWidget() const
{
	return m_ui.timelineView;
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
    getTimelineWidget()->setTimeline(nullptr);
    this->setEnabled(false);
  }
}

void TimelineDockWidget::on_playPauseButton_clicked(bool)
{
	if (m_ui.timelineView->timeline())
  {
		emitRendererChanged(m_ui.timelineView->timeline()->getRenderer());
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
	if (m_ui.timelineView->timeline())
  {
		emitRendererChanged(m_ui.timelineView->timeline()->getRenderer());
  }
  m_project->music()->setPosition(0);
}

void TimelineDockWidget::on_stopButton_clicked(bool)
{
	if (m_ui.timelineView->timeline())
  {
		emitRendererChanged(m_ui.timelineView->timeline()->getRenderer());
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
	if (m_ui.timelineView->timeline())
  {
		emitRendererChanged(m_ui.timelineView->timeline()->getRenderer());
  }
}

void TimelineDockWidget::insertCameraKeyframe(Camera *cam)
{
	DemoTimeline* t = qobject_cast<DemoTimeline*>(m_ui.timelineView->timeline());
  if (t)
  {
    t->insertCameraKeyframe(cam);
  }
}
