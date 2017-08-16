
#include "timeline.hpp"

#include "logwidget.hpp"


Timeline::Timeline(Music& music):
  QGraphicsScene(&music),
  music(music),
  framerate(0)
{
  //setSceneRect(0,0,m->getLength()*framerate,height);

  CONNECT_XML_SAVED_OBJECT(Timeline);
}

void Timeline::setFramerate(double newFramerate)
{
  if (newFramerate <= 0)
  { 
    emit error(objectName() + ": invalide framerate: " + newFramerate);
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
  return (qint64)(getMusic().getTime()*framerate);
}

qint64 Timeline::getLength() const
{
  return (qint64)(getMusic().getLength()*framerate);
}

void Timeline::requestFramePosition(qint64 frame)
{
  getMusic().setPosition(((double)frame/framerate) + (0.5/framerate));
}
