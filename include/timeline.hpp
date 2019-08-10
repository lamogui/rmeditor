#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include <QVector> //Mysterious bug
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

#include "xmlsavedobject.hpp"
#include "music.hpp" // QPointer incomplete type is not allowed !

class Music;
class Renderer;
/*
** Timeline : Any "frame precision" seekable timeline which is linked to the master = the music should inherit from this
*/
class Timeline : public QGraphicsScene
{
  Q_OBJECT
  XML_SAVED_OBJECT

signals : // BUG : Qt doesn't support signals declarations inside macros 
  void propertyChanged(QObject* owner, QString propertyName, QVariant oldValue, QVariant newValue);
  void xmlPropertyChanged(QDomElement node, QString propertyName, QVariant newValue); 

public:
  Timeline(Music* parentMusic); // The music is the parent of the timeline, if it's doesn't exists anymore the timeline should be destroyed ! 

  // For now this should never be null
  const Music& getMusic() const { return *qobject_cast<Music*>(parent()); }
  Music& getMusic() { return *qobject_cast<Music*>(parent()); }

  // frame timings shortcuts
  qint64 getLength() const; 
  qint64 currentFrame() const;

  virtual QWeakPointer<Renderer> getRenderer() { return QWeakPointer<Renderer>(); }
  virtual void updateTime() = 0;


signals:
  // property 
  void framerateChanged(double); // meta compiler doesn't support signal declaration inside macros...
  void requestPosition(double position);

protected:
  double m_height;
  double m_framerate;
  Music& m_music;

private:
  DECLARE_PROPERTY_NOTIFY(double, framerate, Framerate);

};



#endif
