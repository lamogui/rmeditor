
#include "keyframe.hpp"

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

#include "project.hpp"
#include "sequence.hpp"

/*
** Keyframe
*/

/*

Keyframe::Keyframe(Project &project, Sequence* seq, QDomElement &node):
  QObject(nullptr), //lol
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
  QObject(nullptr), //lol
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
*/

Keyframe::Keyframe(KeyframeTrack* parent) :
  QGraphicsObject(parent),  // set GraphicsItem parent
  color(255, 195, 77, 255),
  selectedColor(255, 105, 0, 255),
  mouseCapture(false),
  relativeFrame(-1)
{
  setParent(parent); // set Object parent
}

Keyframe::~Keyframe()
{
  // QApplication::restoreOverrideCursor();
}

GENERATE_PROPERTY_SETTER_NOTIFY(Keyframe, qint64, relativeFrame, RelativeFrame)

/*
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
*/

void Keyframe::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mousePressEvent(event);

  mousePressPos = event->scenePos();
  if (boundingRect().contains(event->pos()))
  {
    mouseCapture = true;
    mousePressRelativeFrame = (qint64)(mousePressPos.x() - parentItem()->scenePos().x());
    Q_ASSERT(mousePressRelativeFrame >= 0);
  }

}

void Keyframe::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (mouseCapture)
  {
    setX(event->scenePos().x() - mousePressPos.x() + parentItem()->scenePos().x());
  }
}

void Keyframe::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  if (mouseCapture)
  {
    setX(event->scenePos().x() - mousePressPos.x() + parentItem()->scenePos().x()); // probably useless
    if (receivers(SIGNAL()) == 1)
  }
  mouseCapture = false;
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

void Keyframe::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseDoubleClickEvent(event);
  emit requestFramePosition((qint64)scenePos().x()); // FIXME : use parent
}

void Keyframe::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  QGraphicsItem::hoverEnterEvent(event);
 // QApplication::setOverrideCursor(Qt::UpArrowCursor);
}

void Keyframe::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  QGraphicsItem::hoverLeaveEvent(event);
 // QApplication::restoreOverrideCursor();
}
