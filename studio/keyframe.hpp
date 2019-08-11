#ifndef KEYFRAME_HPP
#define KEYFRAME_HPP

#include <QColor>
#include <QGraphicsObject>
#include <QPolygonF>
#include <QPainterPath>

#include "undocommands.hpp"

class QDomElement;
class Keyframe : public QGraphicsObject
{
	Q_OBJECT

  public:
		Keyframe(QGraphicsObject* parent = nullptr);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

		void load(const QDomElement& _node); // deprecated !!!

  protected:
    virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

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
		qint64 m_relativeFrame;
};

#endif
