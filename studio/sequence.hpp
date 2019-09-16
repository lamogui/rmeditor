#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <QObject>
#include <QPointer>

#include "keyframe.hpp"
#include "scene.hpp"

class DemoTimeline;
class Sequence : public QObject
{
  Q_OBJECT
  
public:
	Sequence( DemoTimeline & _parent );

	struct Block_t
	{
		qint64 m_startFrame;
		quint64 m_length;

		Block_t() : m_startFrame(0), m_length(60 * 5) {}
		qint64 getEndFrame() const { return m_startFrame + static_cast<qint64>(m_length) - 1; }
		bool isInside( qint64 _frame )  const { return (_frame >= m_startFrame && _frame <= getEndFrame()); }
		bool overlap(const Block_t& _seq) const
		{
			return (isInside(_seq.m_startFrame)||
							isInside(_seq.getEndFrame())||
							(_seq.m_startFrame < m_startFrame && _seq.getEndFrame() > getEndFrame()));
		}
	};

	enum class DiffFlags
	{
		SEQUENCE_BLOCK = 1 << 0,
		SCENE = 1 << 1,
		CAMERA_KEYFRAMES_DELETED = 1 << 2,
		CAMERA_KEYFRAMES_INSERTED = 1 << 3,
		CAMERA_KEYFRAMES_MODIFIED = 1 << 4
	};

	// Data
	bool loadFromFileStream( quint16 _version, QDataStream & _stream );
	void writeToFileStream( QDataStream & _stream ) const;
	bool loadOrControlFromDiffStream(QDataStream & _stream, QDataStream * _undoStream = nullptr , const std::function<bool(Block_t&, Block_t&)> * _controlMoveSequence = nullptr);
	void writeLocalDiffToStream( DiffFlags _flags, QDataStream & _stream ) const; // This can't exist and must be deleted

	// Control
	bool controlDeleteCameraKeyframe(qint64 _frame, QList<qint64>* _deletedKeyframes = nullptr) const;
	bool controlInsertCameraKeyframe( qint64 _frame, QList<qint64>* _insertedKeyframes = nullptr ) const;
	bool controlMoveCameraKeyframe(qint64 _before, qint64 _after, QList<qint64>* _deletedKeyframes = nullptr, QList<qint64>* _insertedKeyframes = nullptr) const;

	// Accesssors
	qint64 getStartFrame() const { return m_sequenceBlock.m_startFrame; }
	quint64 getLength() const { return m_sequenceBlock.m_length; }

	// Utils
	qint64 getEndFrame() const { return m_sequenceBlock.getEndFrame(); } // in parent space
	bool isInside(qint64 _frame) const { return m_sequenceBlock.isInside( _frame ); }

	// Time position
	void setFramePosition(qint64 _framePosition); // local space ?

signals:
	void sequenceBlockChanged( const Sequence & );
	void sceneChanged( const Sequence & );

	// diff
	void cameraKeyframeInserted( const Sequence &, const Keyframe & );
	void cameraKeyframeDeleted( const Sequence &, const Keyframe & );

	// make a pass on all keyframes for view no search
	void cameraKeyframesLoaded( const Sequence & );
	void cameraKeyframesAllDeleted( const Sequence & );

private:
	// Data
	void loadSceneFromIndex( qint16 _index );
	bool loadOrControlInsertCameraKeyframe(QDataStream & _stream,
	 quint16 _version = static_cast<quint16>(-1),  // -1 mean loading from diff
	 QDataStream * _undoStream = nullptr,					 // not nullptr = control
	 QList<qint64>* _insertedKeyframes = nullptr); // should probably be setted in case of control

	// Find
	QList<Keyframe *>::const_iterator upperCameraKeyframe( qint64 _value ) const;
	Keyframe* findCameraKeyframe( qint64 _value ) const;

	// attributes
	Block_t m_sequenceBlock;
	QPointer<const RaymarchingScene> m_scene; // TODO make this editable and notify...
	QList<Keyframe*> m_cameraKeyframes; // only usage for now is to avoid duplicate
};

QDataStream& operator<<( QDataStream & _stream, const Sequence::Block_t& _seq  ) { _stream << _seq.m_length << _seq.m_startFrame; return _stream; }
QDataStream& operator>>( QDataStream & _stream, Sequence::Block_t& _seq ) { _stream >> _seq.m_length >> _seq.m_startFrame; return _stream; }

PROUT_DECLARE_DIFFFLAGS(Sequence)

#include <QWidget>
class DemoTimelineWidget;
class SequenceWidget : public QWidget
{
	Q_OBJECT

public:

	SequenceWidget( DemoTimelineWidget & _parent, const Sequence & _target );

	// Graphics
	QBrush selectedBrush() { return QBrush(QColor(200, 200, 255)); }
	QBrush idleBrush() { return QBrush(QColor(200, 200, 200)); }

	// GraphicsItem
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *_option, QWidget *_widget) override;
	QRectF boundingRect() const override;

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




	// Internal
	Renderer* m_renderer;

	// Graphical
	QImage m_preview;


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
