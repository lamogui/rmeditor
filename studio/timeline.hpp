#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include <QVector> //Mysterious bug
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

#include "xmlsavedobject.hpp"
#include "music.hpp" // QPointer incomplete type is not allowed !

class Project;
class Renderer;
/*
** Timeline : Any "frame precision" seekable timeline which is linked to the master = the music should inherit from this
*/
class Timeline : public QGraphicsScene // TODO Make we don't inherit QGraphicsScene
{
	Q_OBJECT

public:
	Timeline(Project& _parent); // The music is the parent of the timeline, if it's doesn't exists anymore the timeline should be destroyed !

	void setFramerate(double _newFramerate);

	// frame timings shortcuts
	qint64 getLength() const;
	qint64 currentFrame() const;

	virtual Renderer* getRenderer() { return nullptr; }
	virtual void updateTime() = 0;


signals:
	// property
	void framerateChanged(double); // meta compiler doesn't support signal declaration inside macros...
	void requestPosition(double position);

protected:
	double m_framerate;
	Project& m_project;
};



#endif
