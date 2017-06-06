#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include <QVector> //Mysterious bug
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

#include "xmlsavedobject.hpp"
#include "music.hpp" // QPointer incomplete type is not allowed !

class Music;
class Render;
class Timeline : public QGraphicsScene
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  Q_PROPERTY(double framerate MEMBER framerate READ getFramerate WRITE setFramerate NOTIFY framerateChanged)
  XML_SAVED_OBJECT

public:
  Timeline(const Music& music);

  // Property 
  inline double getFramerate() const { return framerate; }
  void setFramerate(double newFramerate);

  // For now this should never be null
  const QPointer<Music>& getMusic() const { return music; }

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
  void framerateChanged(double);

public slots:
  void requestFramePosition(qint64 frame); // shortcut

protected:
  // links
  QPointer<Music> music; // should never be null

private:
  // property                      
  double framerate;

};

#endif // !TIMELINE_HPP
