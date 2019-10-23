#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include <QVector> //Mysterious bug
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

class Music;
class Renderer;
class Timeline : public QGraphicsScene
{
	Q_OBJECT

public:
	Timeline(Music& _music, double _height, double _framerate);

	inline double framerate() const { return m_framerate; }
	inline Music* music() const { return m_music; }
	qint64 length() const; //in frames

	qint64 currentFrame() const;

	virtual Renderer* getRenderer() { return nullptr; }
	virtual void updateTime() = 0;

public slots:
	void requestFramePosition(qint64 frame);

protected:
	double m_height;
	double m_framerate;
	Music* m_music;
};



#endif
