#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <QBrush>
#include <QColor>
#include <QGraphicsRectItem>
#include <QDomElement>


class DemoTimeline;
class Camera;
class CameraKeyframe;
class Keyframe;
class Project;
class Scene;

class Sequence : public QGraphicsRectItem
{


public:
  Sequence(Project& project, DemoTimeline& timeline, QDomElement& node, qreal height = 60.0);
  Sequence(Project& project, DemoTimeline& timeline, QDomElement node, Scene& scene, int start, int length=600, qreal height = 60.0);
  virtual ~Sequence();



  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  inline bool isInside(qint64 frame) const { return (frame >= startFrame() && frame < endFrame()); }
  inline bool overlap(const Sequence& seq) const { return (isInside(seq.startFrame())||
                                                           isInside(seq.endFrame())||
                                                           (seq.startFrame() < startFrame() && seq.endFrame() > endFrame())); }

  inline qint64 startFrame() const { return pos().x(); }
  inline qint64 endFrame() const { return pos().x() + length(); }
  inline qint64 length() const { return rect().width(); }

  inline QMap<qint64, CameraKeyframe*> cameraKeyframes() const { return m_cameraKeyframes; }

  QBrush selectedBrush() { return QBrush(QColor(200,200,255)); }
  QBrush idleBrush() { return QBrush(QColor(200,200,200)); }

  inline Scene* glScene() { return m_scene; }

  void setCamera(qint64 relative_frame, Camera& cam) const;
  void insertCameraKeyframe(qint64 rel_frame, const QVector3D& pos, const QQuaternion& rot);

  typedef enum
  {
    Move,
    LeftExtend,
    RightExtend
  } MouseAction;


  void setStartFrame(qint64 frame, bool notify=true); //don't change the length
  void setLength(qint64 length, bool notify=true);

  //Do not use this is only used by DemoTimeline (I know it's ugly)
  inline void forceSetStartFrame(qint64 frame) { this->setPos(QPointF((qreal)frame,this->pos().y())); }
  inline qint64 orginalLength() const { return m_orginalLength; }
  void deleteCameraKeyframe(CameraKeyframe* key); // key must be correctly in sequence
  inline QDomElement& node() { return m_node; }

  //Used by keyframes
  qint64 nearestFrameAvailableForKeyframe(qint64 rel_frame) const;
  void keyframePositionChanged(qint64 previous_frame, Keyframe* keyframe);

protected:
  void create(int start, int length);
  void load();
  void renderImages();

  virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);


  virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
  virtual void 	hoverMoveEvent(QGraphicsSceneHoverEvent *event);

  bool isInsideRightExtend(QPointF rel_pos, qreal scale) const;
  qreal getScaleFromWidget(const QWidget *widget) const;


protected:
  QDomElement m_node;
  QDomElement m_cameraNode;
  Project* m_project;
  DemoTimeline* m_timeline;
  Scene* m_scene;
  QImage m_preview;
  QPointF m_mousePressPos;
  qint64 m_mousePressStartFrame;
  qint64 m_orginalLength;
  MouseAction m_currentAction;
  QMap<qint64, CameraKeyframe*> m_cameraKeyframes;

};


#endif
