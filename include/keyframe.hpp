﻿#ifndef KEYFRAME_HPP
#define KEYFRAME_HPP

#include <QColor>
#include <QGraphicsObject>

#include "xmlsavedobject.hpp"

class KeyframeTrack; // Should be the parent
class Keyframe : public QGraphicsObject
{

  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT
  
  public:
    //Keyframe(qint64 rel_frame,Project& project, Sequence *seq, QDomElement& node);
    //Keyframe(Project& project, Sequence *seq, QDomElement& node);

    Keyframe(KeyframeTrack* parent = nullptr);
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

    // Movement
    qint64 mousePressRelativeFrame;
    QPointF mousePressPos;
    bool mouseCapture;

  private:
    // Property
    DECLARE_PROPERTY_NOTIFY(qint64, relativeFrame, RelativeFrame)
};

#include <QMap>
class Sequence;
class KeyframeTrack : public QGraphicsObject
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT

public:
  KeyframeTrack();


private:
  QMap<qint64, Keyframe*> keyframes;
};

#endif // !KEYFRAME_HPP
