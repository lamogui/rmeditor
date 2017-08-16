#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <QVector3D>
#include <QQuaternion>

class Camera
{
public:
  Camera();
  virtual ~Camera();

  inline const QVector3D& getPosition() const { return position;}
  inline const QQuaternion& getRotation() const { return rotation; }

  inline void setPosition(const QVector3D& pos) { position = pos;}
  inline void setRotation(const QQuaternion& rot) { rotation = rot; }

  inline QVector3D rd() const { return getRotation().rotatedVector(QVector3D(0,0,-1)); } //Ogre (0,0,-1) Unity (0,0,1)
  inline QVector3D ro() const { return getPosition(); }

  void rotate(qreal yaw, qreal pitch, qreal roll ); //Degrees
  void translateRelative(const QVector3D& dir);
  void translate(const QVector3D& dir);

  void reset();

protected:
  QQuaternion rotation;
  QVector3D position;

};


#include "keyframe.hpp"

class CameraKeyframe : public Keyframe
{
public:
  CameraKeyframe(QGraphicsObject* parent);

  // utils
  void fromCamera(Camera& cam);

private:
  // properties
  DECLARE_PROPERTY_REFERENCE(QQuaternion, rotation, Rotation)
  DECLARE_PROPERTY_REFERENCE(QVector3D, position, Position)
};

Q_DECLARE_METATYPE(CameraKeyframe*)


#endif // !CAMERA_HPP
