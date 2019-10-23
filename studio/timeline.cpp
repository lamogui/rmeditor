
#include "timeline.hpp"

#include "logwidget.hpp"
#include "music.hpp"


Timeline::Timeline(Music & _music, double _height, double _framerate):
	QGraphicsScene(&_music),
	m_height(_height),
	m_framerate(_framerate),
	m_music(&_music)
{
	setSceneRect(0,0,m_music->length()*_framerate,_height);

}

qint64 Timeline::currentFrame() const
{
  return (qint64)(music()->getTime()*m_framerate);
}

qint64 Timeline::length() const
{
  return (qint64)(m_music->length()*m_framerate);
}

void Timeline::requestFramePosition(qint64 frame)
{
  m_music->setPosition(((double)frame/m_framerate) + (0.5/m_framerate));
}
