#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <QVector3D>
#include <QQuaternion>


class CameraKeyframe;

class Camera
{
public:
  Camera();
  //Camera(const Camera& cpy);
  virtual ~Camera();

  //const Camera& operator= (const Camera& cpy);

  inline const QVector3D& getPosition() const { return position;}
  inline const QQuaternion& getRotation() const { return rotation; }

  inline void setPosition(const QVector3D& pos) { position = pos;}
  inline void setRotation(const QQuaternion& rot) { rotation = rot; }

  inline QVector3D rd() const { return getRotation().rotatedVector(QVector3D(0,0,-1)); } //Ogre (0,0,-1) Unity (0,0,1)
  inline QVector3D ro() const { return getPosition(); }

  void rotate(qreal yaw, qreal pitch, qreal roll ); //Degrees
  void translateRelative(const QVector3D& dir);
  void translate(const QVector3D& dir);

  void fromKeyframe(const CameraKeyframe& keyframe);

  void reset();

protected:
  QQuaternion rotation;
  QVector3D position;

};

#include "keyframe.hpp"

class CameraKeyframe : public Keyframe
{
public:
  CameraKeyframe(Project& project, Sequence *seq, QDomElement& node);
  CameraKeyframe(qint64 rel_frame, Project& project, Sequence *seq, QDomElement& node, const QVector3D& pos, const QQuaternion& q);


  void setPosition(const QVector3D& p);
  void setRotation(const QQuaternion& q);

  inline const QVector3D& getPosition() const { return position;}
  inline const QQuaternion& getRotation() const { return rotation; }

  void fromCamera(Camera& cam);

  virtual void load();

protected:
  QQuaternion rotation;
  QVector3D position;

};

#endif // !CAMERA_HPP
