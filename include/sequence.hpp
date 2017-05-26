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
  ~Sequence() override;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

  inline bool isInside(qint64 frame) const { return (frame >= startFrame() && frame < endFrame()); }
  inline bool overlap(const Sequence& seq) const { return (isInside(seq.startFrame())||
                                                           isInside(seq.endFrame())||
                                                           (seq.startFrame() < startFrame() && seq.endFrame() > endFrame())); }

  inline qint64 startFrame() const { return pos().x(); }
  inline qint64 endFrame() const { return pos().x() + length(); }
  inline qint64 length() const { return rect().width(); }

  inline QMap<qint64, CameraKeyframe*> getCameraKeyframes() const { return cameraKeyframes; }

  QBrush selectedBrush() { return QBrush(QColor(200,200,255)); }
  QBrush idleBrush() { return QBrush(QColor(200,200,200)); }

  inline Scene* glScene() { return scene; }

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
  inline qint64 getOrginalLength() const { return orginalLength; }
  void deleteCameraKeyframe(CameraKeyframe* key); // key must be correctly in sequence
  inline QDomElement& getNode() { return node; }

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


protected:
  QDomElement node;
  QDomElement cameraNode;
  Project* project;
  DemoTimeline* timeline;
  Scene* scene;
  QImage preview;
  QPointF mousePressPos;
  qint64 mousePressStartFrame;
  qint64 orginalLength;
  MouseAction currentAction;
  QMap<qint64, CameraKeyframe*> cameraKeyframes;

};


#endif
