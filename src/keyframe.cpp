
#include "keyframe.hpp"

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

#include "project.hpp"
#include "sequence.hpp"


Keyframe::Keyframe(Project &project, Sequence* seq, QDomElement &node):
  QObject(NULL), //lol
  QGraphicsItem(seq),
  sequence(seq),
  project(&project),
  node(node),
  color(255,195,77,255),
  selectedColor(255,105,0,255),
  mouseCapture(false)
{
  this->setFlags(QGraphicsItem::ItemIsSelectable);
  setAcceptHoverEvents(true);
}

Keyframe::Keyframe(qint64 rel_frame, Project &project, Sequence* seq, QDomElement &node):
  QObject(NULL), //lol
  QGraphicsItem(seq),
  sequence(seq),
  project(&project),
  node(node),
  color(255,195,77,255),
  selectedColor(255,105,0,255),
  mouseCapture(false)
{
  node.setTagName("keyframe");
  setRelativeFrame(rel_frame);
  this->setFlags(QGraphicsItem::ItemIsSelectable);
  setAcceptHoverEvents(true);
}

void Keyframe::load()
{
  qint64 frame = node.attribute("frame","0").toLongLong();
  setPos(checkFrameAvailable(frame),pos().y());
}

Keyframe::~Keyframe()
{
  QApplication::restoreOverrideCursor();
}

void Keyframe::notifyChanged()
{
  project->notifyDocumentChanged();
}

void Keyframe::setRelativeFrame(qint64 frame, bool notify)
{
  qint64 previous_frame = relativeFrame();

  if (previous_frame != frame && frame >=0)
  {
    qint64 new_frame = checkFrameAvailable(frame);
    setPos(new_frame,this->pos().y());
    node.setAttribute(QString("frame"),relativeFrame());
    positionChanged(previous_frame);
  }
  if (notify)
  {
    notifyChanged();
  }
}

void Keyframe::setAbsoluteFrame(qint64 frame, bool notify)
{
  if (this->parentItem())
  {
    setRelativeFrame(frame - this->parentItem()->pos().x(),notify);
  }
  else
  {
    setRelativeFrame(frame,notify);
  }
}



void Keyframe::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mousePressEvent(event);

  mousePressPos = event->scenePos();
  if (boundingRect().contains(event->pos()))
  {
    mouseCapture = true;
    originalAbsoluteFrame = absoluteFrame();
  }

}

void Keyframe::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (mouseCapture)
  {
    setAbsoluteFrame(event->scenePos().x() - mousePressPos.x()  + originalAbsoluteFrame,false);
  }
}


void Keyframe::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  if (mouseCapture)
  {
    setAbsoluteFrame(event->scenePos().x() - mousePressPos.x()  + originalAbsoluteFrame,true);
  }
  mouseCapture = false;
}

qint64 Keyframe::checkFrameAvailable(qint64 rel_frame)
{
  if (sequence)
  {
    return sequence->nearestFrameAvailableForKeyframe(rel_frame);
  }
  else
  {
    Q_ASSERT(0 && "Not supported yet");
    return rel_frame;
  }
}

QRectF Keyframe::boundingRect() const
{
  qreal l = 5;
  return QRectF(-l,-l,2*l,2*l);
}

void Keyframe::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  (void)option; (void) widget;
  QBrush fillBrush(color);
  QPen pen(QColor(255,64,35));
  if (mouseCapture)
  {
    fillBrush.setColor(selectedColor);
  }

  painter->setPen(pen);
  painter->fillRect(boundingRect(),fillBrush);
  painter->drawRect(boundingRect());
}

void Keyframe::positionChanged(qint64 previous)
{
  if (sequence)
  {
    sequence->keyframePositionChanged(previous,this);
  }
  else
  {
    Q_ASSERT(0 && "Not supported yet");
  }

}

void Keyframe::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseDoubleClickEvent(event);
  emit requestFramePosition(this->absoluteFrame());
}

void Keyframe::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  QGraphicsItem::hoverEnterEvent(event);
  QApplication::setOverrideCursor(Qt::UpArrowCursor);
}

void Keyframe::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  QGraphicsItem::hoverLeaveEvent(event);
  QApplication::restoreOverrideCursor();
}
