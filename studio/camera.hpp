#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <QVector3D>
#include <QQuaternion>

class CameraState // Data struct for a camera
{
public:
	CameraState();

	// Properties
	QQuaternion m_rotation;
	QVector3D m_position;
	float m_fov;

	// Utils
	void rotate(qreal _yaw, qreal _pitch, qreal _roll ); //Degrees
	void translateRelative(const QVector3D& _dir);
	void translate(const QVector3D& _dir);
	void reset();
	inline QVector3D rd() const { return m_rotation.rotatedVector(QVector3D(0,0,-1)); } //Ogre (0,0,-1) Unity (0,0,1)
	inline QVector3D ro() const { return m_position; }
};

#include "keyframe.hpp"

class CameraKeyframe : public Keyframe
{
  Q_OBJECT

public:
	CameraKeyframe(QGraphicsObject* _parent);
	CameraState m_cameraState;

	// Deprecated !
	void load(const QDomElement& _node);
};


#endif
