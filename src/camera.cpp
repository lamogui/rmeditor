#include "camera.hpp"


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
void Camera::fromKeyframe(const CameraKeyframe &keyframe)
{
  position = keyframe.getPosition();
  rotation = keyframe.getRotation();
}



#include "project.hpp"

CameraKeyframe::CameraKeyframe(Project &project, Sequence *seq, QDomElement &node):
  Keyframe(project,seq,node)
{
  load();
}

CameraKeyframe::CameraKeyframe(qint64 rel_frame, Project &project, Sequence *seq, QDomElement &node, const QVector3D &p, const QQuaternion &q):
  Keyframe(rel_frame,project,seq,node)
{
  setPosition(p);
  setRotation(q);
}

void CameraKeyframe::setPosition(const QVector3D &p)
{
  position = p;
  node.setAttribute("pos_x",p.x());
  node.setAttribute("pos_y",p.y());
  node.setAttribute("pos_z",p.z());
  project->notifyDocumentChanged();
}

void CameraKeyframe::setRotation(const QQuaternion &q)
{
  rotation = q;
  node.setAttribute("rot_x",q.x());
  node.setAttribute("rot_y",q.y());
  node.setAttribute("rot_z",q.z());
  node.setAttribute("rot_w",q.scalar());
  project->notifyDocumentChanged();
}

void CameraKeyframe::fromCamera(Camera &cam)
{
  setPosition(cam.getPosition());
  setRotation(cam.getRotation());
}

void CameraKeyframe::load()
{
  Keyframe::load();
  position.setX(node.attribute("pos_x","0.0").toFloat());
  position.setY(node.attribute("pos_y","0.0").toFloat());
  position.setZ(node.attribute("pos_z","0.0").toFloat());

  rotation.setX(node.attribute("rot_x","0.0").toFloat());
  rotation.setY(node.attribute("rot_y","0.0").toFloat());
  rotation.setZ(node.attribute("rot_z","0.0").toFloat());
  rotation.setScalar(node.attribute("rot_w","1.0").toFloat());
}
*/
