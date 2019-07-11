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

  public:
    Keyframe(qint64 rel_frame,Project& project, Sequence *seq, QDomElement& node);
    Keyframe(Project& project, Sequence *seq, QDomElement& node);

#if defined(Q_OS_WIN) && defined(QT_NO_DEBUG) //Bug mingw ?
    virtual ~Keyframe();
#endif
    virtual void load();

    inline Sequence* sequence() const { return m_sequence; }
    inline qint64 relativeFrame() const { return (qint64)pos().x(); }
    inline qint64 absoluteFrame() const { return (qint64)scenePos().x(); }

    void setRelativeFrame(qint64 frame, bool notify = true);
    void setAbsoluteFrame(qint64 frame, bool notify = true);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;


    void notifyChanged();

    qint64 checkFrameAvailable(qint64 rel_frame);

    QDomElement& node() { return m_node; }


  signals:
    void requestFramePosition(qint64 frame);

  protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void positionChanged(qint64 previous);

  protected:
    Sequence* m_sequence;
    Project* m_project;
    QDomElement m_node;
    QColor m_color;
    QColor m_selectedColor;
    qint64 m_originalAbsoluteFrame;
    QPointF m_mousePressPos;
    bool m_mouseCapture;

};

#endif
