
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

Keyframe::Keyframe(QGraphicsObject* parent) :
  QGraphicsObject(parent),  // set GraphicsItem parent
  color(255, 195, 77, 255),
  selectedColor(255, 105, 0, 255),
  mouseCapture(false),
  relativeFrame(-1)
{
  float l = 5;
  shape.append(QPointF(-l, 0));
  shape.append(QPointF(0, -l));
  shape.append(QPointF(l, 0));
  shape.append(QPointF(0, l));
  painterPath.addPolygon(shape);

  setParent(parent); // set Object parent 
}


#if defined(Q_OS_WIN) && defined(QT_NO_DEBUG)
Keyframe::~Keyframe()
{
  // QApplication::restoreOverrideCursor();
}
#endif

void Keyframe::setRelativeFrame(qint64 newrelativeFrame)
{
  if (newrelativeFrame != relativeFrame)
  {
    qint64 oldValue = relativeFrame;
    relativeFrame = newrelativeFrame;
    setX(relativeFrame); // set graphical position
    emit propertyChanged(this, "relativeFrame", oldValue, relativeFrame);
  }
}

void Keyframe::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mousePressEvent(event);
  
  m_mousePressPos = event->scenePos();
  if (shape.containsPoint(event->pos(), Qt::OddEvenFill))
  {
    mousePressRelativeFrame = getRelativeFrame();
    mouseCapture = true;
  }
}

void Keyframe::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (m_mouseCapture)
    setRelativeFrame(event->scenePos().x() - mousePressPos.x() + parentItem()->scenePos().x());
}

void Keyframe::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  if (m_mouseCapture)
  {
    ASSERT_IF_UNIQUE_RECEIVER(sendUndoCommand(QUndoCommand*))
    {
      emit sendUndoCommand(new ModifyPropertyCommand(*this, "relativeFrame", mousePressRelativeFrame, (qint64)(event->scenePos().x() - mousePressPos.x() + parentItem()->scenePos().x())));
    }
  }
  m_mouseCapture = false;
}

QRectF Keyframe::boundingRect() const
  {return painterPath.boundingRect();}

void Keyframe::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  (void)option; (void) widget;
  QBrush fillBrush(mouseCapture || isSelected() ? selectedColor : color);
  QPen pen(QColor(255,64,35));

  painter->fillPath(painterPath, fillBrush);
  painter->strokePath(painterPath, pen);
  //painter->fillRect(boundingRect(),fillBrush);
  //painter->drawRect(boundingRect());
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
