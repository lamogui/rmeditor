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

  inline const QVector3D& position() const { return m_position;}
  inline const QQuaternion& rotation() const { return m_rotation; }

  inline void setPosition(const QVector3D& pos) { m_position = pos;}
  inline void setRotation(const QQuaternion& rot) { m_rotation = rot; }

  inline QVector3D rd() const { return rotation().rotatedVector(QVector3D(0,0,-1)); } //Ogre (0,0,-1) Unity (0,0,1)
  inline QVector3D ro() const { return position(); }

  void rotate(qreal yaw, qreal pitch, qreal roll ); //Degrees
  void translateRelative(const QVector3D& dir);
  void translate(const QVector3D& dir);

  void fromKeyframe(const CameraKeyframe& keyframe);

  void reset();

protected:
  QQuaternion m_rotation;
  QVector3D m_position;

};

#include "keyframe.hpp"

class CameraKeyframe : public Keyframe
{
public:
  CameraKeyframe(Project& project, Sequence *seq, QDomElement& node);
  CameraKeyframe(qint64 rel_frame, Project& project, Sequence *seq, QDomElement& node, const QVector3D& pos, const QQuaternion& q);


  void setPosition(const QVector3D& p);
  void setRotation(const QQuaternion& q);

  inline const QVector3D& position() const { return m_position;}
  inline const QQuaternion& rotation() const { return m_rotation; }

  void fromCamera(Camera& cam);

  virtual void load();

protected:
  QQuaternion m_rotation;
  QVector3D m_position;

};

#endif
