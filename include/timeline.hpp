﻿#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include <QVector> //Mysterious bug
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

class LogWidget;
class Music;
class Render;

class Timeline : public QGraphicsScene
{
  Q_OBJECT

public:
  Timeline(Music& music, double height, double framerate, LogWidget& log);

  inline double getFramerate() const { return framerate; }
  inline Music* getMusic() const { return music; }
  qint64 length() const; //in frames

  //Do the connections to log
  void connectLog(LogWidget& log);

  qint64 currentFrame() const;

  virtual Render* getRender() { return nullptr; }
  virtual void updateTime() = 0;

signals:
  void error(QString err) const;
  void warning(QString warn) const;
  void info(QString txt) const;

public slots:
  void requestFramePosition(qint64 frame);

protected:
  double height;
  double framerate;
  Music* music;

};

#endif // !TIMELINE_HPP
