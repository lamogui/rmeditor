﻿#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include <QVector> //Mysterious bug
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

#include "xmlsavedobject.hpp"
#include "music.hpp" // QPointer incomplete type is not allowed !

class Music;
class Render;
/*
** Timeline : Any "frame precision" seekable timeline which is linked to the master = the music should inherit from this
*/
class Timeline : public QGraphicsScene
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT

public:
  Timeline(Music& music); // The music is the parent of the timeline, if it's doesn't exists anymore the timeline should be destroyed ! 

  // For now this should never be null
  const Music& getMusic() const { return music; }
  Music& getMusic() { return music; }

  // frame timings shortcuts
  qint64 getLength() const; 
  qint64 currentFrame() const;

  virtual Render* getRender() { return nullptr; }
  virtual void updateTime() = 0;

signals:
  // Log
  void error(QString);
  void warning(QString);
  void info(QString);

  // property 
  void framerateChanged(double); // meta compiler doesn't support signal declaration inside macros...

public slots:
  void requestFramePosition(qint64 frame); // shortcut

protected:
  // links
  Music& music; // shortcut to casted parent

private:
  DECLARE_PROPERTY_NOTIFY(double, framerate, Framerate);

};

#endif // !TIMELINE_HPP
