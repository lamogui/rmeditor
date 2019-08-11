
#include "timeline.hpp"
#include "project.hpp"
#include "logwidget.hpp"


Timeline::Timeline(Project& _parent):
	QGraphicsScene( &_parent ),
	m_framerate(60.0),
	m_project( _parent )
{
  setSceneRect(0,0,m_music->length()*framerate,height);
}

void Timeline::setFramerate(double _newFramerate)
{
	if (_newFramerate <= 0)
	{
		Log::Error(objectName() + ": invalid framerate: " + _newFramerate);
		_newFramerate = 0;
	}
	m_framerate = _newFramerate;
	if ( m_framerate != _newFramerate )
	{
		emit framerateChanged(m_framerate);
	}
}

qint64 Timeline::currentFrame() const
{
	return static_cast<qint64>(m_project.getMusic().getLength() * m_framerate);
}

qint64 Timeline::getLength() const
{
	return static_cast<qint64>(m_project->length()*m_framerate);
}

void Timeline::requestFramePosition(qint64 frame)
{
	m_music->setPosition(((double)frame/m_framerate) + (0.5/m_framerate));
}
