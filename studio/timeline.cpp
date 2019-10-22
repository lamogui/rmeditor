
#include "timeline.hpp"

#include "logwidget.hpp"
#include "music.hpp"


Timeline::Timeline(Music &music, double height, double framerate, LogWidget &log):
  QGraphicsScene(&music),
  m_height(height),
  m_framerate(framerate),
  m_music(&music)
{
  setSceneRect(0,0,m_music->length()*framerate,height);

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
