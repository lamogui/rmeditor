#ifndef KEYFRAME_HPP
#define KEYFRAME_HPP

#include <QObject>
class Sequence;
class Keyframe : public QObject
{
	Q_OBJECT

public:
	Keyframe(Sequence * _parent, qint64 _relativePosition);

	bool loadFromData(quint16 _version, QByteStream & _buffer);
	qint64 getRelativePosition() const { return m_relativePosition; }

signals:
	// update the view
	void relativePositionChanged(Keyframe*);

protected:
	// local control
	void setRelativePosition(qint64 _position);

	// property
	qint64 m_relativePosition;
};


#include <QWidget>
class KeyframeWidget : public QWidget
{
	Q_OBJECT

public:


};

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
