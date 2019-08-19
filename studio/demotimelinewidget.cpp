

#include "demotimelinewidget.hpp"

#include <QDebug>

#include <QBrush>
#include <QMouseEvent>
#include <QPen>
#include <QScrollBar>
#include <QTime>

#include "music.hpp"
#include "project.hpp"

DemoTimelineWidget::DemoTimelineWidget(QWidget* _parent, qreal _borderheight):
	QGraphicsView(_parent),
	m_borderHeight(_borderheight),
	m_project(nullptr),
	m_updateTimer(),
	m_grabTime(false),
	m_scale(1,1)
{
	m_updateTimer.setSingleShot(false);
}

void DemoTimelineWidget::setProject(Project * _project)
{
	stopUpdateLoop();
	m_project = _project;
	startUpdateLoop();

	if(m_project)
	{
		DemoTimeline& demoTimeline = m_project->m_demoTimeline;
		setScene(&demoTimeline);
		setScale(QPointF((qreal)this->rect().width()/demoTimeline.sceneRect().width(),1.0));
		QRectF scene_rect = demoTimeline.sceneRect();
		scene_rect.setTop(-m_borderHeight - 2.0);
		demoTimeline.setSceneRect(scene_rect);
	}
	else
	{
		setScene(nullptr);
		setScale(QPointF(1.0,1.0));
	}

}

void DemoTimelineWidget::updateWithTime()
{
	jassert( m_project && m_project->m_music );
	Music& music = *m_project->m_music;
	double t = music.getTime();
	if (t != m_lastTimeDraw)
	{
		DemoTimeline& demoTimeline = m_project->m_demoTimeline;
		demoTimeline.updateTime();

		if (t > music.getLength())
		{
			emit timePositionChanged(0);
		}

		QScrollBar* horizontal = this->horizontalScrollBar();
		qreal scroll_length = horizontal->maximum() - horizontal->minimum() + horizontal->pageStep();
		qreal scroll_pos = t*scroll_length/music.getLength();

		if (scroll_pos > horizontal->value() + horizontal->pageStep())
		{
			horizontal->setValue(static_cast<int>(scroll_pos) - horizontal->pageStep()/4);
		}
		else if (scroll_pos < horizontal->value())
		{
			horizontal->setValue(static_cast<int>(std::max(static_cast<qreal>(horizontal->minimum()),scroll_pos - horizontal->pageStep()/4)));
		}

		invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
		update();

		m_lastTimeDraw = t;
  }
}

void DemoTimelineWidget::onProjectDestroy()
{
	disconnect(&m_updateTimer, &QTimer::timeout,this, &DemoTimelineWidget::updateWithTime);
	disconnect(this,&DemoTimelineWidget::timePositionChanged,nullptr,nullptr);
	m_updateTimer.stop();
	m_project = nullptr;
	setScene(nullptr);
}


void DemoTimelineWidget::drawForeground(QPainter* _painter, const QRectF& _rect)
{

	QScrollBar *vertical = this->verticalScrollBar(), *horizontal = this->horizontalScrollBar();
	qDebug() << "horizontal" << "min" << horizontal->minimum() << "value" << horizontal->value() << "max" << horizontal->maximum() << "pageStep" << horizontal->pageStep();
	qDebug() << "vertical" << "min" << vertical->minimum() << "value" << vertical->value() << "max" << vertical->maximum() << "pageStep" << vertical->pageStep();
	qDebug() << "sceneRect" << sceneRect();
	qreal x = sceneRect().width() * (horizontal->value() - horizontal->minimum())/
						static_cast<qreal>(horizontal->maximum() - horizontal->minimum() + horizontal->pageStep());
	qreal y = sceneRect().height() * (vertical->value() - vertical->minimum())/
						static_cast<qreal>(vertical->maximum() - vertical->minimum() + vertical->pageStep());

	QPointF delta(x,y-getBorderHeight()-2.0);
	qreal visibleWidth = sceneRect().width()  * static_cast<qreal>(horizontal->pageStep())/static_cast<qreal>(horizontal->maximum() - horizontal->minimum() + horizontal->pageStep());

	QRectF border = QRectF(delta,QSizeF(visibleWidth,getSceneBorderHeight()));
	if (_rect.intersects(border))
	{
		QBrush fillBrush(QColor(166,166,166));
		_painter->setBrush(fillBrush);
		_painter->drawRect(border);

		QBrush textBrush(QColor(0,0,0));

		QPen linePen(QColor(0,0,0));
		linePen.setWidth(0);
		_painter->setPen(linePen);

		double framerate = 60.0;
		if (m_project)
		{
			framerate = m_project->getFramerate();
			m_lastTimeForegroundDraw = m_project->m_music->getTime();
		}

		//TODO condition for framerate
		{
			double step = framerate;
			while (border.width()/step > 15.0)
			{
				step *= 2.0;
			}
			QFont textFont;
			textFont.setPointSizeF(getSceneBorderHeight()*0.5);
			textFont.setStretch(static_cast<int>(100 * 1.0 / m_scale.x()));
			_painter->setFont(textFont);


			qreal sp = (border.height())*0.6 + border.top();
			double start = _rect.left() - fmod(_rect.left(),step);
			for (double f = start; f < _rect.right(); f+=step)
			{
				_painter->drawLine(QPointF(f,sp),QPointF(f,border.bottom()));
			}
			sp = (border.height())*0.8 + border.top();
			for (double f = start+step/2.0; f < _rect.right(); f+=step)
			{
				_painter->drawLine(QPointF(f,sp),QPointF(f,border.bottom()));
			}

			QTime time;
			_painter->setBrush(textBrush);
			for (double f = start; f < border.right(); f+=step)
			{
				double f_f = f/framerate; //in sec
				time.setHMS(0,f_f/60.0,fmod(f_f,60.0));
				_painter->drawText(QRectF(f - step*0.9,border.top(),step*1.8,getSceneBorderHeight()*0.5),Qt::AlignCenter,time.toString("mm:ss"));
			}
		}

		qreal lineWidth = 2.0 / m_scale.x();
		linePen.setWidthF(lineWidth);
		_painter->setPen(linePen);
		double m_t = m_lastTimeForegroundDraw * framerate;
		if (m_t >= _rect.left() && m_t <= _rect.right())
		{
			QLineF line(QPointF(m_t,_rect.top()),QPointF(m_t,_rect.bottom()));
			_painter->drawLine(line);
		}

		m_lastBorderRect = border;
  }

	QGraphicsView::drawForeground(_painter,_rect);
}

void DemoTimelineWidget::mousePressEvent(QMouseEvent* _mouseEvent)
{
	if (m_project)
	{
		emit currentRendererChanged(&m_project->m_demoTimeline.m_renderer);
	}

	QPointF scenePos = mapToScene(_mouseEvent->pos());
	if (m_lastBorderRect.contains(scenePos))
	{
		mouseTimePositionChanged(scenePos);
	}
	else
	{
		QGraphicsView::mousePressEvent(_mouseEvent);
	}
}


 void DemoTimelineWidget::mouseMoveEvent(QMouseEvent* mouseEvent)
 {
	 QPointF scenePos = mapToScene(mouseEvent->pos());
	 if (m_grabTime)
	 {
			mouseTimePositionChanged(scenePos);
	 }
	 else
	 {
		 QGraphicsView::mouseMoveEvent(mouseEvent);
	 }
 }

 void DemoTimelineWidget::mouseReleaseEvent(QMouseEvent* mouseEvent)
 {
	 //QPointF scenePos = mapToScene(mouseEvent->pos());
	 if (m_grabTime)
	 {
		 m_grabTime = false;
	 }
	 else
	 {
		 QGraphicsView::mouseReleaseEvent(mouseEvent);
	 }
	 m_grabTime = false;
 }



 void DemoTimelineWidget::mouseTimePositionChanged(const QPointF& scenePos)
 {
	 if (m_project)
	 {
		 emit timePositionChanged(scenePos.x()/m_project->getFramerate());
		 invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
		 update();
		 m_grabTime = true;
	 }
 }

 void DemoTimelineWidget::wheelEvent(QWheelEvent *event)
 {
	 qreal delta = event->angleDelta().y();
	 if (delta > 0.0)
	 {
		 delta = 1.1;
	 }
	 else
	 {
		 delta = 0.9;
	 }

	 if (event->modifiers() & Qt::ShiftModifier)
	 {
		 //m_scale.setY(m_scale.y()*delta);
	 }
	 else
	 {
		 m_scale.setX(m_scale.x()*delta);
	 }
	 setScale(m_scale);
 }


void DemoTimelineWidget::setScale(const QPointF& scale)
{
	m_scale = scale;
	if (m_scale.x() > 1.0)
	{
		m_scale.setX(1.0);
	}
	else if (m_project && this->rect().width() > m_project->m_demoTimeline.sceneRect().width() * m_scale.x())
	{
		m_scale.setX(static_cast<qreal>(this->rect().width())/m_project->m_demoTimeline.sceneRect().width());
	}
	QTransform t;
	t.translate(transform().dx(),transform().dy());
	t.scale(m_scale.x(),m_scale.y());
	setTransform(t,false);
	invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
	//update();
}

void DemoTimelineWidget::focusInEvent(QFocusEvent* _event)
{
	if (m_project)
	{
		emit currentRendererChanged(&m_project->m_demoTimeline.m_renderer);
	}
}

void DemoTimelineWidget::startUpdateLoop()
{
	if (m_project)
	{
		connect(m_project, &QObject::destroyed,this,&DemoTimelineWidget::onProjectDestroy,Qt::DirectConnection);
		connect(&m_updateTimer, &QTimer::timeout,this, &DemoTimelineWidget::updateWithTime);
		connect(this,&DemoTimelineWidget::timePositionChanged, m_project->m_music, &Music::setPosition);
		m_updateTimer.start();
		m_updateTimer.setInterval(10.0);
	}
}

void DemoTimelineWidget::stopUpdateLoop()
{
	if (m_project)
	{
		disconnect(&m_updateTimer, &QTimer::timeout,this, &DemoTimelineWidget::updateWithTime);
		disconnect(this,&DemoTimelineWidget::timePositionChanged, m_project->m_music, &Music::setPosition);
		disconnect(m_project, &QObject::destroyed, this, &DemoTimelineWidget::onProjectDestroy);
		m_updateTimer.stop();
	}
}
