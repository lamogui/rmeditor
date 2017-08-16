#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <QGraphicsObject>

#include "xmlsavedobject.hpp"
#include "undocommands.hpp"

class CameraKeyframe;
class MediaFile;
class Renderer;

template <class cl>
using Int64Map = QMap<qint64, cl>; // Compiler bug with macros because of ',' ! 
class Sequence : public QGraphicsObject 
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT
  UNDOCOMMANDS_SENDER_OBJECT

public:
  Sequence(QGraphicsObject* parent = nullptr);

  // Utils 
  inline qint64 getEndFrame() const { getStartFrame() + getLength(); }
  inline bool isInside(qint64 frame) const { return (frame >= getStartFrame() && frame < getEndFrame()); }
  inline bool overlap(const Sequence& seq) const { return (isInside(seq.getStartFrame())||
                                                           isInside(seq.getEndFrame())||
                                                           (seq.getStartFrame() < getStartFrame() && seq.getEndFrame() > getEndFrame())); }
  

  // Accessors
  void insertCameraKeyframe(CameraKeyframe& keyframe);
  void removeCameraKeyframe(CameraKeyframe& keyframe);


  // Graphcis
  QBrush selectedBrush() { return QBrush(QColor(200, 200, 255)); }
  QBrush idleBrush() { return QBrush(QColor(200, 200, 200)); }


  // GraphicsItem
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;


  /*
  void setCamera(qint64 relative_frame, Camera& cam) const;
  void insertCameraKeyframe(qint64 rel_frame, const QVector3D& pos, const QQuaternion& rot);
  */

  typedef enum
  {
    Move,
    LeftExtend,
    RightExtend
  } MouseAction;

  //Do not use this is only used by DemoTimeline (I know it's ugly)
  inline void forceSetStartFrame(qint64 frame) { this->setPos(QPointF((qreal)frame,this->pos().y())); }
  void deleteCameraKeyframe(CameraKeyframe* key); // key must be correctly in sequence
  
  //Used by keyframes
  qint64 nearestFrameAvailableForKeyframe(qint64 rel_frame) const;
  void keyframePositionChanged(qint64 previous_frame, Keyframe* keyframe);

protected:
  void create(int start, int length);
  void load();
  void renderImages();

  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override; 


  void 	hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
  void 	hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
  void 	hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

  bool isInsideRightExtend(QPointF rel_pos, qreal scale) const;
  qreal getScaleFromWidget(const QWidget *widget) const;


private:
  // properties
  DECLARE_PROPERTY_REFERENCE(QPointer<MediaFile>, media, Media)
  DECLARE_PROPERTY(qint64, startFrame, StartFrame)
  DECLARE_PROPERTY(quint64, length, Length)
  DECLARE_PROPERTY_REFERENCE(Int64Map<CameraKeyframe*>, cameraKeyframes, CameraKeyframes)

  // Internal 
  QSharedPointer<Renderer> renderer;

  // Graphical
  QImage preview;

  // Movement
  QPointF mousePressPos;
  qint64 mousePressStartFrame;
  qint64 mousePressLength;
  MouseAction currentAction;
};


#endif
