
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
  m_sequence(seq),
  m_project(&project),
  m_node(node),
  m_color(255,195,77,255),
  m_selectedColor(255,105,0,255),
  m_mouseCapture(false)
{
  this->setFlags(QGraphicsItem::ItemIsSelectable);
  setAcceptHoverEvents(true);
}

Keyframe::Keyframe(qint64 rel_frame, Project &project, Sequence* seq, QDomElement &node):
  QObject(NULL), //lol
  QGraphicsItem(seq),
  m_sequence(seq),
  m_project(&project),
  m_node(node),
  m_color(255,195,77,255),
  m_selectedColor(255,105,0,255),
  m_mouseCapture(false)
{
  m_node.setTagName("keyframe");
  setRelativeFrame(rel_frame);
  this->setFlags(QGraphicsItem::ItemIsSelectable);
  setAcceptHoverEvents(true);
}

void Keyframe::load()
{
  qint64 frame = m_node.attribute("frame","0").toLongLong();
  setPos(checkFrameAvailable(frame),pos().y());
}


#if defined(Q_OS_WIN) && defined(QT_NO_DEBUG)
Keyframe::~Keyframe()
{
  QApplication::restoreOverrideCursor();
}
#endif

void Keyframe::notifyChanged()
{
  m_project->notifyDocumentChanged();
}

void Keyframe::setRelativeFrame(qint64 frame, bool notify)
{
  qint64 previous_frame = relativeFrame();

  if (previous_frame != frame && frame >=0)
  {
    qint64 new_frame = checkFrameAvailable(frame);
    setPos(new_frame,this->pos().y());
    m_node.setAttribute(QString("frame"),relativeFrame());
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

  m_mousePressPos = event->scenePos();
  if (boundingRect().contains(event->pos()))
  {
    m_mouseCapture = true;
    m_originalAbsoluteFrame = absoluteFrame();
  }

}

void Keyframe::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (m_mouseCapture)
  {
    setAbsoluteFrame(event->scenePos().x() - m_mousePressPos.x()  + m_originalAbsoluteFrame,false);
  }
}


void Keyframe::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  if (m_mouseCapture)
  {
    setAbsoluteFrame(event->scenePos().x() - m_mousePressPos.x()  + m_originalAbsoluteFrame,true);
  }
  m_mouseCapture = false;
}

qint64 Keyframe::checkFrameAvailable(qint64 rel_frame)
{
  if (m_sequence)
  {
    return m_sequence->nearestFrameAvailableForKeyframe(rel_frame);
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
  QBrush fillBrush(m_color);
  QPen pen(QColor(255,64,35));
  if (m_mouseCapture)
  {
    fillBrush.setColor(m_selectedColor);
  }

  painter->setPen(pen);
  painter->fillRect(boundingRect(),fillBrush);
  painter->drawRect(boundingRect());
}

void Keyframe::positionChanged(qint64 previous)
{
  if (m_sequence)
  {
    m_sequence->keyframePositionChanged(previous,this);
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
