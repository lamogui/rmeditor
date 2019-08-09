
#include "timeline.hpp"

#include "logwidget.hpp"


Timeline::Timeline(Music* music):
  QGraphicsScene(music),
  framerate(60.0)
{
  //setSceneRect(0,0,m->getLength()*framerate,height);

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
  return (qint64)(getMusic().getPosition()*framerate);
}

qint64 Timeline::getLength() const
{
  return (qint64)(getMusic().getLength()*framerate);
}
