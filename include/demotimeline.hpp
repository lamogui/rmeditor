﻿
#ifndef DEMOTIMELINE_HPP
#define DEMOTIMELINE_HPP

#include <QAction>
#include <QDir>
#include <QDomElement>
#include <QMap>


#include "camera.hpp"
#include "renderer.hpp"
#include "timeline.hpp"

class DemoTimeline;
class Project;
class Sequence;


class DemoRenderer : public Renderer
{
public:
  DemoRenderer(DemoTimeline&  timeline, size_t w, size_t h);

protected:
  void glRender() override;

  DemoTimeline* timeline;


};


class DemoTimeline : public Timeline
{

  Q_OBJECT

public:
  DemoTimeline(QDomElement& node, Project& project, double fps, LogWidget& log);
  ~DemoTimeline() override;

  inline QDomElement getNode() const { return node; }

  qint64 sequenceStartFrameChanged(qint64 previous_frame,  Sequence* seq); //return the correct start frame of the seq
  qint64 maxSequenceLengthBeforeOverlap(Sequence* seq) const;


  qint64 addSequence(Sequence* seq); //Return the correct start frame of the seq
  Sequence* isInsideSequence(qint64 frame) const;

  inline Renderer* getRenderer() override { return renderer; }

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

  void exportSources(const QDir& dir) const;

protected slots:
  void addSequenceAction(QAction* action);

protected:
  void keyReleaseEvent(QKeyEvent *keyEvent) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

  void load();
  bool correctStartFrame(Sequence* seq); //seq must not be in sequences !


  bool parseTrackNode(QDomElement& node);

  QDomElement node;
  Camera camera;
  double trackHeight;
  Project* project;
  DemoRenderer* renderer;
  QMap<qint64, Sequence*> sequences;
  QPointF mousePressPos;

};

#endif
