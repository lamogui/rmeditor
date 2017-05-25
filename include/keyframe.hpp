#ifndef KEYFRAME_HPP
#define KEYFRAME_HPP

#include <QColor>
#include <QDomElement>
#include <QGraphicsItem>
#include <QObject>


class Project;
class Sequence;

class Keyframe : public QObject, public QGraphicsItem
{

  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
  
  public:
    Keyframe(qint64 rel_frame,Project& project, Sequence *seq, QDomElement& node);
    Keyframe(Project& project, Sequence *seq, QDomElement& node);

    ~Keyframe() override;

    virtual void load();

    inline Sequence* getSequence() const { return sequence; }
    inline qint64 relativeFrame() const { return (qint64)pos().x(); }
    inline qint64 absoluteFrame() const { return (qint64)scenePos().x(); }

    void setRelativeFrame(qint64 frame, bool notify = true);
    void setAbsoluteFrame(qint64 frame, bool notify = true);

    // QGraphicsItem
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;

    void notifyChanged();

    qint64 checkFrameAvailable(qint64 rel_frame);

    QDomElement& getNode() { return node; }


  signals:
    void requestFramePosition(qint64 frame);

  protected:

    // QGraphicsItem
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void 	hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void 	hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void positionChanged(qint64 previous);

  protected:
    Sequence* sequence;
    Project* project;
    QDomElement node;
    QColor color;
    QColor selectedColor;
    qint64 originalAbsoluteFrame;
    QPointF mousePressPos;
    bool mouseCapture;

};

#endif // !KEYFRAME_HPP
