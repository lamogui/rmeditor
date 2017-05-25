
#include "timeline.hpp"

#include "logwidget.hpp"
#include "music.hpp"


Timeline::Timeline(Music &music, double height, double framerate, LogWidget &log):
  QGraphicsScene(&music),
  height(height),
  framerate(framerate),
  music(&music)
{
  setSceneRect(0,0,this->music->getLength()*framerate,height);

  connectLog(log);
}


void Timeline::connectLog(LogWidget &log)
{
  connect(this,SIGNAL(error(QString)),&log,SLOT(writeError(QString)));
  connect(this,SIGNAL(warning(QString)),&log,SLOT(writeWarning(QString)));
  connect(this,SIGNAL(info(QString)),&log,SLOT(writeInfo(QString)));
}


qint64 Timeline::currentFrame() const
{
  return (qint64)(getMusic()->getTime()*framerate);
}

qint64 Timeline::length() const
{
  return (qint64)(music->getLength()*framerate);
}

void Timeline::requestFramePosition(qint64 frame)
{
  music->setPosition(((double)frame/framerate) + (0.5/framerate));
}
