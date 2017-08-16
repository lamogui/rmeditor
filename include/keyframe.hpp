#ifndef KEYFRAME_HPP
#define KEYFRAME_HPP

#include <QColor>
#include <QGraphicsObject>
#include <QPolygonF>
#include <QPainterPath>

#include "xmlsavedobject.hpp"
#include "undocommands.hpp"

class Keyframe : public QGraphicsObject
{

  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT
  UNDOCOMMANDS_SENDER_OBJECT
  
  public:
    Keyframe(QGraphicsObject* parent = nullptr);
    ~Keyframe() override;

    // QGraphicsItem
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;

  protected:
    // QGraphicsItem
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void 	hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void 	hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  signals:
    void requestFramePosition(qint64 absoluteFrame);

  protected:
    // Aspect 
    QColor color;
    QColor selectedColor;
    QPolygonF shape;
    QPainterPath painterPath;

    // Movement
    qint64 mousePressRelativeFrame;
    QPointF mousePressPos;
    bool mouseCapture;

  private:
    // Property
    DECLARE_PROPERTY(qint64, relativeFrame, RelativeFrame)
};

#endif // !KEYFRAME_HPP
