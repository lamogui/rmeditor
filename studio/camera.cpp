#include "camera.hpp"

/*
 * CameraState
 */

CameraState::CameraState():
	m_rotation(1,0,0,0),
	m_position(0,0,0),
	m_fov(60.0f)
{
}

void CameraState::reset()
{
	m_rotation = QQuaternion(1,0,0,0);
	m_position = QVector3D(0,0,0);
	m_fov = 60.0f;
}

void CameraState::translate(const QVector3D &_dir)
{
	m_position += _dir;
}

void CameraState::translateRelative(const QVector3D &_dir)
{
	m_position += m_rotation.rotatedVector(_dir);
}

void CameraState::rotate(qreal _yaw, qreal _pitch, qreal _roll)
{
	m_rotation *= QQuaternion::fromAxisAndAngle(0,1,0, _yaw) * QQuaternion::fromAxisAndAngle(1,0,0,_pitch) * QQuaternion::fromAxisAndAngle(0,0,1,_roll);
}

#include <QDomElement>

/*
	** CameraKeyframe :
*/
CameraKeyframe::CameraKeyframe(QGraphicsObject* _parent):
	Keyframe(_parent),
	m_cameraState()
{
}

void CameraKeyframe::load(const QDomElement& _node)
{
	Keyframe::load( _node );
	m_cameraState.m_position.setX(_node.attribute("pos_x","0.0").toFloat());
	m_cameraState.m_position.setY(_node.attribute("pos_y","0.0").toFloat());
	m_cameraState.m_position.setZ(_node.attribute("pos_z","0.0").toFloat());

	m_cameraState.m_rotation.setX(_node.attribute("rot_x","0.0").toFloat());
	m_cameraState.m_rotation.setY(_node.attribute("rot_y","0.0").toFloat());
	m_cameraState.m_rotation.setZ(_node.attribute("rot_z","0.0").toFloat());
	m_cameraState.m_rotation.setScalar(_node.attribute("rot_w","1.0").toFloat());

	m_cameraState.m_fov = _node.attribute("fov","60.0").toFloat();
}



