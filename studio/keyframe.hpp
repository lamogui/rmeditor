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

  signals:// BUG : Qt doesn't support signals declarations inside macros 
    void propertyChanged(QObject* owner, QString propertyName, QVariant oldValue, QVariant newValue);
    void xmlPropertyChanged(QDomElement node, QString propertyName, QVariant newValue);
  
  public:
    Keyframe(QGraphicsObject* parent = nullptr);
#if defined(Q_OS_WIN) && defined(QT_NO_DEBUG) //Bug mingw ?
    virtual ~Keyframe();
#endif
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

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
    DECLARE_PROPERTY(qint64, relativeFrame, RelativeFrame)
};

#endif
