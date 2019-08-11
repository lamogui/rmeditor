#include "camera.hpp"


/*
** Camera
*/
Camera::Camera():
  m_rotation(1,0,0,0),
  m_position(0,0,0),
  m_fov(60.0f)
{

}

Camera::~Camera()
{

}

void Camera::fromKeyframe(const CameraKeyframe& keyframe)
{
  rotation = keyframe.getRotation();
  position = keyframe.getPosition();
  fov = keyframe.getFov();
}

void Camera::reset()
{
  m_rotation = QQuaternion(1,0,0,0);
  m_position = QVector3D(0,0,0);
  m_fov = 60.0f;
}

void Camera::translate(const QVector3D &dir)
{
  m_position += dir;
}

void Camera::translateRelative(const QVector3D &dir)
{
  m_position += m_rotation.rotatedVector(dir);
}

void Camera::rotate(qreal yaw, qreal pitch, qreal roll)
{
  m_rotation *= QQuaternion::fromAxisAndAngle(0,1,0,yaw) * QQuaternion::fromAxisAndAngle(1,0,0,pitch) * QQuaternion::fromAxisAndAngle(0,0,1,roll);
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

CameraKeyframe::CameraKeyframe(Project &project, Sequence *seq, QDomElement &node):
  Keyframe(project,seq,node)
{
  load();
}

CameraKeyframe::CameraKeyframe(qint64 rel_frame, Project &project, Sequence *seq, QDomElement &node, const QVector3D &p, const QQuaternion &q, float fov):
  Keyframe(rel_frame,project,seq,node)
{
  setPosition(p);
  setRotation(q);
  setFov(fov);
}

void CameraKeyframe::setPosition(const QVector3D &p)
{
  m_position = p;
  m_node.setAttribute("pos_x",p.x());
  m_node.setAttribute("pos_y",p.y());
  m_node.setAttribute("pos_z",p.z());
  m_project->notifyDocumentChanged();
}

void CameraKeyframe::setRotation(const QQuaternion &q)
{
  m_rotation = q;
  m_node.setAttribute("rot_x",q.x());
  m_node.setAttribute("rot_y",q.y());
  m_node.setAttribute("rot_z",q.z());
  m_node.setAttribute("rot_w",q.scalar());
  m_project->notifyDocumentChanged();
}
void CameraKeyframe::setFov(float fov)
{
    m_fov = fov;
    m_node.setAttribute("fov",fov);

}

void CameraKeyframe::fromCamera(Camera &cam)
{
  setPosition(cam.position());
  setRotation(cam.rotation());
  setFov(cam.fov());
}

void CameraKeyframe::load()
{
  Keyframe::load();
  m_position.setX(m_node.attribute("pos_x","0.0").toFloat());
  m_position.setY(m_node.attribute("pos_y","0.0").toFloat());
  m_position.setZ(m_node.attribute("pos_z","0.0").toFloat());

  m_rotation.setX(m_node.attribute("rot_x","0.0").toFloat());
  m_rotation.setY(m_node.attribute("rot_y","0.0").toFloat());
  m_rotation.setZ(m_node.attribute("rot_z","0.0").toFloat());
  m_rotation.setScalar(m_node.attribute("rot_w","1.0").toFloat());

  m_fov = m_node.attribute("fov","60.0").toFloat();
}



