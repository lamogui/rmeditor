
#include "timeline.hpp"

#include "logwidget.hpp"


Timeline::Timeline(Music* music):
  QGraphicsScene(music),
  framerate(60.0)
{
  setSceneRect(0,0,m_music->length()*framerate,height);

  CONNECT_XML_SAVED_OBJECT(Timeline);
}

void Timeline::setFramerate(double newFramerate)
{
  if (newFramerate <= 0)
  { 
    Log::Error(objectName() + ": invalid framerate: " + newFramerate);
    newFramerate = 0;
  }
  if (framerate != framerate)
  {
    double oldFramerate = framerate;
    framerate = newFramerate;
    emit propertyChanged(this, "framerate", oldFramerate, framerate);
    emit framerateChanged(framerate);
  }
}

qint64 Timeline::currentFrame() const
{
  return (qint64)(music()->getTime()*m_framerate);
}

qint64 Timeline::getLength() const
{
  return (qint64)(m_music->length()*m_framerate);
}

void Timeline::requestFramePosition(qint64 frame)
{
  m_music->setPosition(((double)frame/m_framerate) + (0.5/m_framerate));
}
