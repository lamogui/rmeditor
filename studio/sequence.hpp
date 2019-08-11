#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <QGraphicsObject>
#include <QBrush>

#include "xmlsavedobject.hpp"
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
  Q_INVOKABLE Sequence(QGraphicsObject* parent = nullptr);

  // Utils 
	inline qint64 getEndFrame() const { return m_startFrame; }
	inline bool isInside(qint64 frame) const { return (frame >= m_startFrame && frame < getEndFrame()); }
	inline bool overlap(const Sequence& seq) const { return (isInside(seq.m_startFrame)||
                                                           isInside(seq.getEndFrame())||
																													 (seq.m_startFrame < m_startFrame && seq.getEndFrame() > getEndFrame())); }
  
  // Time position 
  void setFramePosition(qint64 framePosition);

  // Graphics
  QBrush selectedBrush() { return QBrush(QColor(200, 200, 255)); }
  QBrush idleBrush() { return QBrush(QColor(200, 200, 200)); }
  float getHeight() const { return height; }

  // GraphicsItem
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *_option, QWidget *_widget) override;
  QRectF boundingRect() const override;

  inline QMap<qint64, CameraKeyframe*> getCameraKeyframes() const { return m_cameraKeyframes; }
  // GL
  void initializeGL(RenderFunctionsCache& renderCache);

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

  virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

  virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
  virtual void 	hoverMoveEvent(QGraphicsSceneHoverEvent *event);

  bool isInsideRightExtend(QPointF rel_pos, qreal scale) const;
  bool isInsideLeftExtend(QPointF rel_pos, qreal scale) const;
  qreal getScaleFromWidget(const QWidget *widget) const;


protected slots:
  void keyframePropertyChanged(QObject* owner, const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
  void keyframeRequestFramePosition(qint64 position);

private:
  // properties
	QPointer<MediaFile> m_media;
	qint64 m_startFrame;
	quint64 m_length;
	QMap<qint64, CameraKeyframe*> m_cameraKeyframes;

  // Internal 
  QSharedPointer<Renderer> renderer;
  QSharedPointer<Camera> camera; // sadly because of QWeakPointer we need to do that, maybe Camera should inherit from QObject ?  

  // Graphical
  QImage preview;
  float height;

  // Render
  RenderFunctionsCache* renderCache; // fucking opengl context

  // Movement
  QPointF mousePressPos;
  qint64 mousePressStartFrame;
  qint64 mousePressLength;
  MouseAction currentAction;

private:
  typedef QGraphicsObject BaseClass;
};

#endif
