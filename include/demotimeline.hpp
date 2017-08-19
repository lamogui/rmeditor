﻿
#ifndef DEMOTIMELINE_HPP
#define DEMOTIMELINE_HPP

#include <QVector>

#include "timeline.hpp"


/*
** DemoTimelineRenderer : know how to render a DemoTimeline
*/
/*
class DemoTimelineRenderer : public Renderer
{
public:
  DemoTimelineRenderer(DemoTimeline&  timeline, const QSize& initialSize); // Timeline is the parent

  // Rendering
  void initializeGL(RenderFunctionsCache& gl) override;
  void renderChildrens(RenderFunctionsCache& gl) override;
  void glRender(RenderFunctionsCache& gl, Render& render) override; 

protected:
  DemoTimeline& timeline; 
};
*/

class TimelineTrack;
class DemoTimeline : public Timeline
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT

public:
  DemoTimeline(QObject* parentMusic);
  ~DemoTimeline() override;

  void initializeGL(RenderFunctionsCache& cache);
  /*
  qint64 sequenceStartFrameChanged(qint64 previous_frame,  Sequence* seq); //return the correct start frame of the seq
  qint64 maxSequenceLengthBeforeOverlap(Sequence* seq) const;


  qint64 addSequence(Sequence* seq); //Return the correct start frame of the seq
  Sequence* isInsideSequence(qint64 frame) const;

  inline Render* getRender() override { return render; }

  void updateTime() override;
  
public slots:
  void updateCamera(qint64 frame, Camera& cam);
  inline void updateCamera(Camera& cam) { updateCamera(currentFrame(),cam);}
  inline void updateCamera(qint64 frame) { updateCamera(frame,camera); }
  inline void updateCamera() { updateCamera(currentFrame(),camera); }

  void insertCameraKeyframe(qint64 frame, const QVector3D& pos, const QQuaternion& rot);
  void insertCameraKeyframe(const QVector3D& pos, const QQuaternion& rot);
  void insertCameraKeyframe(qint64 frame, Camera* cam);
  void insertCameraKeyframe(Camera* cam);

  void deleteSelectedItems();
  void deleteItems(const QList<QGraphicsItem*> items);
  void deleteSequence(Sequence* seq);

  void renderImage(const QSize& resolution, QImage* target);

  void exportSources(const QDir& dir) const;*/

protected slots:
  //void addSequenceAction(QAction* action);
  void trackRequestFramePosition(qint64 position);
  /*
protected:
  void keyReleaseEvent(QKeyEvent *keyEvent) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

  void load();
  bool correctStartFrame(Sequence* seq); //seq must not be in sequences !


  bool parseTrackNode(QDomElement& node);

  QDomElement node;
  QSharedPointer<Camera> camera;
  QPointF mousePressPos;
  */
private:
  // properties
  DECLARE_PROPERTY_CONTAINER(QVector, TimelineTrack*, tracks, Tracks, track, Track)

  // render
  RenderFunctionsCache* renderCache;
};

#endif
