#ifndef TIMELINETRACK_HPP
#define TIMELINETRACK_HPP

#include "xmlsavedobject.hpp"
#include "undocommands.hpp"

#include <QMap>
#include <QGraphicsObject>

class Renderer;
class Sequence; 
class DemoTimeline;

/*
** TimelineTrack : a timeline track of a DemoTimeline
*/
class TimelineTrack : public QGraphicsObject
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT
  UNDOCOMMANDS_SENDER_OBJECT

public:
  TimelineTrack(QGraphicsObject* parent = nullptr); 
  
  // GL
  void initializeGL(RenderFunctionsCache& cache);

  // utils
  Sequence* isInsideSequence(qint64 frame) const;
  quint64 getLength() const;

  // Graphics
  float getHeight() const { return height; }

  // GraphicsItem
  QRectF boundingRect() const;

signals:
  void requestFramePosition(qint64 position);

protected slots:
  void sequencePropertyChanged(QObject* owner, const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
  void sequenceRequestFramePosition(const Sequence* source, qint64 position);

protected:
  bool makeSequenceFit(Sequence& sequence);

private:
  // Properties
  DECLARE_PROPERTY_CONTAINER(Int64Map, Sequence*, sequences, Sequences, sequence, Sequence)

  // Graphics
  float height;

  // Render
  RenderFunctionsCache* renderCache;
};



#endif // !TIMELINETRACK_HPP