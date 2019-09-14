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
	m_rotation *= QQuaternion::fromAxisAndAngle(0,1,0,static_cast<float>(_yaw))
								* QQuaternion::fromAxisAndAngle(1,0,0,static_cast<float>(_pitch))
								* QQuaternion::fromAxisAndAngle(0,0,1,static_cast<float>(_roll));
}
