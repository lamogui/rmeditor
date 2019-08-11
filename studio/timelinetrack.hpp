#ifndef TIMELINETRACK_HPP
#define TIMELINETRACK_HPP

#include "xmlsavedobject.hpp"
#include "undocommands.hpp"
#include "renderfunctionscache.hpp"

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

public:
  Q_INVOKABLE TimelineTrack(QGraphicsObject* parent = nullptr); 
  
  // GL
  void initializeGL(RenderFunctionsCache& cache);

  // utils
  Sequence* isInsideSequence(qint64 frame) const;
  quint64 getLength() const;

  // Graphics
  float getHeight() const { return height; }

  // GraphicsItem
  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
  void requestFramePosition(qint64 position);

protected slots:
  void sequencePropertyChanged(QObject* owner, const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
  void sequenceRequestFramePosition(const Sequence* source, qint64 position);

protected:
  bool makeSequenceFit(Sequence& sequence);

  //Graphics
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

  QPointF mousePressPos;

private:
  // Properties
	QMap<qint64, Sequence*> m_sequences;

  // Graphics
	float m_height;

  // Render
  RenderFunctionsCache* renderCache;
};



#endif // !TIMELINETRACK_HPP
