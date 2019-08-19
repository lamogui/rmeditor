#ifndef TIMELINEWIDGET_HPP
#define TIMELINEWIDGET_HPP

#include <QGraphicsView>
#include <QTimer>

class Renderer;
class Project;

class DemoTimelineWidget : public QGraphicsView
{
  Q_OBJECT

public:
	DemoTimelineWidget(QWidget* _parent, qreal _borderheight = 25.0);

	void setProject(Project* _project);

	inline double getBorderHeight() const { return m_borderHeight; }
	inline double getSceneBorderHeight() const { return getBorderHeight()/m_scale.y(); }

	void setScale(const QPointF &_scale);
	inline const QPointF& scale() const { return m_scale; }

	void startUpdateLoop();
	void stopUpdateLoop();

signals:
	void timePositionChanged(double);
	void currentRendererChanged(QPointer<Renderer> _renderer);


protected:
	virtual void drawForeground(QPainter* _painter, const QRectF& _rect);
	virtual void mousePressEvent(QMouseEvent* _mouseEvent);
	virtual void mouseMoveEvent(QMouseEvent* _mouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent* _mouseEvent);
	virtual void wheelEvent(QWheelEvent* _event);
	virtual void focusInEvent(QFocusEvent* _event);


protected slots:
	void onProjectDestroy();

private slots:
	void updateWithTime();

private:
	void mouseTimePositionChanged(const QPointF &_scenePos);

	double m_lastTimeForegroundDraw; //This should be useless as a member now
	double m_lastTimeDraw;
	qreal m_borderHeight;
	QRectF m_lastBorderRect;
	Project* m_project;
	QTimer m_updateTimer;
	bool m_grabTime;
	QPointF m_scale;

};

#endif
