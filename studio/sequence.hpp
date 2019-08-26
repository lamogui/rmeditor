#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <QGraphicsObject>
#include <QBrush>

#include "undocommands.hpp"
#include "renderfunctionscache.hpp"


class CameraKeyframe;
class MediaFile;
class Renderer;
class Camera;
class Sequence : public QGraphicsObject 
{
  Q_OBJECT
  
public:
	Sequence(QGraphicsObject* parent = nullptr);

	// MediaFile
	QPointer<MediaFile> m_media; // TODO make this editable and notify..., maybe use an intex ?

	// Utils
	inline qint64 getEndFrame() const { return m_startFrame; }
	inline bool isInside(qint64 _frame) const { return (_frame >= m_startFrame && _frame < getEndFrame()); }
	inline bool overlap(const Sequence& _seq) const { return (isInside(_seq.m_startFrame)||
																													 isInside(_seq.getEndFrame())||
																													 (_seq.m_startFrame < m_startFrame && _seq.getEndFrame() > getEndFrame())); }
	// Time position
	void setFramePosition(qint64 _framePosition);

	// Graphics
	QBrush selectedBrush() { return QBrush(QColor(200, 200, 255)); }
	QBrush idleBrush() { return QBrush(QColor(200, 200, 200)); }

	// GraphicsItem
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *_option, QWidget *_widget) override;
	QRectF boundingRect() const override;

	inline QMap<qint64, CameraKeyframe*> getCameraKeyframes() const { return m_cameraKeyframes; }
	// GL
	void initializeGL(RenderFunctionsCache& _renderCache);

	/*
	void setCamera(qint64 relative_frame, Camera& cam) const;
	void insertCameraKeyframe(qint64 rel_frame, const QVector3D& pos, const QQuaternion& rot, float fov);
	*/

	typedef enum
	{
		Move,
		LeftExtend,
		RightExtend
	} MouseAction;

	//Do not use this is only used by DemoTimeline (I know it's ugly)
	//inline void forceSetStartFrame(qint64 frame) { this->setPos(QPointF((qreal)frame,this->pos().y())); }
	//void deleteCameraKeyframe(CameraKeyframe* key); // key must be correctly in sequence

	//Used by keyframes
	//qint64 nearestFrameAvailableForKeyframe(qint64 rel_frame) const;

signals:
	void requestFramePosition(const Sequence* source, qint64 position);

protected:
	void renderImages();

	void mousePressEvent(QGraphicsSceneMouseEvent* _event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* _event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* _event) override;

	void 	hoverEnterEvent(QGraphicsSceneHoverEvent* _event) override;
	void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* _event) override;
	void 	hoverMoveEvent(QGraphicsSceneHoverEvent* _event) override;

	bool isInsideRightExtend(QPointF _rel_pos, qreal _scale) const;
	bool isInsideLeftExtend(QPointF _rel_pos, qreal _scale) const;
	qreal getScaleFromWidget(const QWidget* _widget) const;


protected slots:
	void keyframePropertyChanged(QObject* _owner, const QString& _propertyName, const QVariant& _oldValue, const QVariant& _newValue);
	void keyframeRequestFramePosition(qint64 _position);

private:
	// properties
	qint64 m_startFrame;
	quint64 m_length;
	QMap<qint64, CameraKeyframe*> m_cameraKeyframes;

	// Internal
	Renderer* m_renderer;

	// Graphical
	QImage m_preview;
	qreal m_height;

	// Render
	RenderFunctionsCache* m_renderCache; // fucking opengl context

	// Movement
	QPointF m_mousePressPos;
	qint64 m_mousePressStartFrame;
	qint64 m_mousePressLength;
	MouseAction m_currentAction;

private:
	typedef QGraphicsObject BaseClass;
};

#endif
