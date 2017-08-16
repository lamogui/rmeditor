#include "camera.hpp"


/*
** Camera
*/
Camera::Camera():
  rotation(1,0,0,0),
  position(0,0,0)
{

}

Camera::~Camera()
{

}

void Camera::reset()
{
  rotation = QQuaternion(1,0,0,0);
  position = QVector3D(0,0,0);
}

void Camera::translate(const QVector3D &dir)
{
  position += dir;
}

void Camera::translateRelative(const QVector3D &dir)
{
  position += rotation.rotatedVector(dir);
}

void Camera::rotate(qreal yaw, qreal pitch, qreal roll)
{
  rotation *= QQuaternion::fromAxisAndAngle(0,1,0,yaw) * QQuaternion::fromAxisAndAngle(1,0,0,pitch) * QQuaternion::fromAxisAndAngle(0,0,1,roll);
}

/*
** CameraKeyframe : 
*/
CameraKeyframe::CameraKeyframe(QGraphicsObject* parent):
  Keyframe(parent)
{
}

GENERATE_PROPERTY_SETTER_REFERENCE(CameraKeyframe, QQuaternion, rotation, Rotation)
GENERATE_PROPERTY_SETTER_REFERENCE(CameraKeyframe, QVector3D, position, Position)

void CameraKeyframe::fromCamera(Camera &cam)
{
  setPosition(cam.getPosition());
  setRotation(cam.getRotation());
}
