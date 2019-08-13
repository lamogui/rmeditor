
#ifndef DEMOTIMELINE_HPP
#define DEMOTIMELINE_HPP

#include <QVector>
#include <QGraphicsScene>


/*
** DemoTimelineRenderer : know how to render a DemoTimeline
*/
/*
class DemoTimelineRenderer : public Renderer
{
public:
  DemoTimelineRenderer(Project& project, DemoTimeline&  timeline, const QSize& initialSize); // Timeline is the parent

  // Rendering
  void initializeGL(RenderFunctionsCache& gl) override;
  void renderChildrens(RenderFunctionsCache& gl) override;
  void glRender(RenderFunctionsCache& gl, Render& render) override; 

protected:
  Project& m_project;
  DemoTimeline& m_timeline;};
*/


class Project;
class DemoTimeline : public QGraphicsScene
{
  Q_OBJECT

public:
	DemoTimeline(Project& _parentProject);
  virtual ~DemoTimeline();

  /*
  qint64 sequenceStartFrameChanged(qint64 previous_frame,  Sequence* seq); //return the correct start frame of the seq
  qint64 maxSequenceLengthBeforeOverlap(Sequence* seq) const;


  qint64 addSequence(Sequence* seq); //Return the correct start frame of the seq
  Sequence* isInsideSequence(qint64 frame) const;
  unsigned int sequenceID(qint64 frame) const;

  inline Render* getRender() override { return render; }
  */
  virtual void updateTime();

signals:
	// property
	void framerateChanged(double); // meta compiler doesn't support signal declaration inside macros...
	void requestPosition(double position);
  /*
public slots:
  void updateCamera(qint64 frame, Camera& cam);
  inline void updateCamera(Camera& cam) { updateCamera(currentFrame(),cam);}
  inline void updateCamera(qint64 frame) { updateCamera(frame,m_camera); }
  inline void updateCamera() { updateCamera(currentFrame(),m_camera); }

  void insertCameraKeyframe(qint64 frame, const QVector3D& pos, const QQuaternion& rot, float fov);
  void insertCameraKeyframe(const QVector3D& pos, const QQuaternion& rot, float fov);
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

  void load();
  bool correctStartFrame(Sequence* seq); //seq must not be in m_sequences !


  bool parseTrackNode(QDomElement& node);

  QDomElement m_node;
  Camera m_camera;
  double m_trackHeight;
  Project& m_project;
  DemoRenderer* m_renderer;
  QMap<qint64, Sequence*> m_sequences;
  QPointF m_mousePressPos;
	*/

protected:
	double m_framerate;
	Project& m_project;
};

#endif
