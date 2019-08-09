

#include "timelinewidget.hpp"

#include <QDebug>

#include <QBrush>
#include <QMouseEvent>
#include <QPen>
#include <QScrollBar>
#include <QTime>
#include <QTimer>

#include "music.hpp"
#include "timeline.hpp"

TimelineWidget::TimelineWidget(QWidget *parent, qreal borderheight):
  QGraphicsView(parent),
  m_borderHeight(borderheight),
  m_timeline(nullptr),
  m_updateTimer(new QTimer(this)),
  m_grabTime(false),
  m_scale(1,1)
{
  m_updateTimer->setSingleShot(false);
}

void TimelineWidget::setTimeline(Timeline *timeline)
{
  stopUpdateLoop();
  m_timeline = timeline;
  startUpdateLoop();

  setScene(m_timeline);
  if(m_timeline)
  {
    setScale(QPointF((qreal)this->rect().width()/m_timeline->sceneRect().width(),1.0));
    QRectF scene_rect = m_timeline->sceneRect();
    scene_rect.setTop(-m_borderHeight - 2.0);
    m_timeline->setSceneRect(scene_rect);
  }
  else
  {
    setScale(QPointF(1.0,1.0));
  }

}

void TimelineWidget::updateWithTime()
{
  double t = timeline->getMusic().getPosition();
  if (t != m_lastTimeDraw)
  {
    m_timeline->updateTime();

    if (t > timeline->getMusic().getLength())
    {
      emit timePositionChanged(0);
    }

    QScrollBar* horizontal = this->horizontalScrollBar();
    qreal scroll_length = horizontal->maximum() - horizontal->minimum() + horizontal->pageStep();
    qreal scroll_pos = t*scroll_length/timeline->getMusic().getLength();

    if (scroll_pos > horizontal->value() + horizontal->pageStep())
    {
      horizontal->setValue((int)scroll_pos - horizontal->pageStep()/4);
    }
    else if (scroll_pos < horizontal->value())
    {
      horizontal->setValue(std::max((qreal)horizontal->minimum(),scroll_pos - horizontal->pageStep()/4));
    }

    invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
    update();

    m_lastTimeDraw = t;
  }
}

void TimelineWidget::onTimelineDestroy()
{

  disconnect(m_updateTimer,SIGNAL(timeout()),this,SLOT(updateWithTime()));
  disconnect(this,SIGNAL(timePositionChanged(double)),0,0);
  m_updateTimer->stop();
  m_timeline=nullptr;
  setScene(m_timeline);
}


void TimelineWidget::drawForeground(QPainter* painter, const QRectF& rect)
{

  QScrollBar *vertical = this->verticalScrollBar(), *horizontal = this->horizontalScrollBar();
  qreal x = sceneRect().width() * (horizontal->value() - horizontal->minimum())/
            (qreal)(horizontal->maximum() - horizontal->minimum() + horizontal->pageStep());
  qreal y = sceneRect().height() * (vertical->value() - vertical->minimum())/
            (qreal)(vertical->maximum() - vertical->minimum() + vertical->pageStep());

  QPointF delta(x,y-borderHeight()-2.0);
  qreal visibleWidth = sceneRect().width()  * (qreal)horizontal->pageStep()/(qreal)(horizontal->maximum() - horizontal->minimum() + horizontal->pageStep());

  QRectF border = QRectF(delta,QSizeF(visibleWidth,sceneBorderHeight()));
  if (rect.intersects(border))
  {
    QBrush fillBrush(QColor(166,166,166));
    painter->setBrush(fillBrush);
    painter->drawRect(border);

    QBrush textBrush(QColor(0,0,0));

    QPen linePen(QColor(0,0,0));
    linePen.setWidth(0);
    painter->setPen(linePen);

    double framerate = 60.0;
    if (m_timeline)
    {
      framerate = m_timeline->getFramerate();
      m_lastTimeForegroundDraw = m_timeline->getMusic()->getPosition();
    }

    //TODO condition for framerate
    {
      double step = framerate;
      while (border.width()/step > 15.0)
      {
        step *= 2.0;
      }
      QFont textFont;
      textFont.setPointSizeF(sceneBorderHeight()*0.5);
      textFont.setStretch((int)(100 * 1.0 / m_scale.x()));
      painter->setFont(textFont);


      qreal sp = (border.height())*0.6 + border.top();
      double start = rect.left() - fmod(rect.left(),step);
      for (double f = start; f < rect.right(); f+=step)
      {
        painter->drawLine(QPointF(f,sp),QPointF(f,border.bottom()));
      }
      sp = (border.height())*0.8 + border.top();
      for (double f = start+step/2.0; f < rect.right(); f+=step)
      {
        painter->drawLine(QPointF(f,sp),QPointF(f,border.bottom()));
      }

      QTime time;
      painter->setBrush(textBrush);
      for (double f = start; f < border.right(); f+=step)
      {
        double f_f = f/framerate; //in sec
        time.setHMS(0,f_f/60.0,fmod(f_f,60.0));
        painter->drawText(QRectF(f - step*0.9,border.top(),step*1.8,sceneBorderHeight()*0.5),Qt::AlignCenter,time.toString("mm:ss"));
      }
    }

    qreal lineWidth = 2.0 / m_scale.x();
    linePen.setWidthF(lineWidth);
    painter->setPen(linePen);
    double m_t = m_lastTimeForegroundDraw * framerate;
    if (m_t >= rect.left() && m_t <= rect.right())
    {
      QLineF line(QPointF(m_t,rect.top()),QPointF(m_t,rect.bottom()));
      painter->drawLine(line);
    }

    m_lastBorderRect = border;
  }

  QGraphicsView::drawForeground(painter,rect);
}

void TimelineWidget::mousePressEvent(QMouseEvent* mouseEvent)
{
  if (m_timeline)
  {
    emit currentRendererChanged(timeline->getRenderer());
  }

  QPointF scenePos = mapToScene(mouseEvent->pos());
  if (m_lastBorderRect.contains(scenePos))
  {
    mouseTimePositionChanged(scenePos);
  }
  else
  {
    QGraphicsView::mousePressEvent(mouseEvent);
  }
}


 void TimelineWidget::mouseMoveEvent(QMouseEvent* mouseEvent)
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

 void TimelineWidget::mouseReleaseEvent(QMouseEvent* mouseEvent)
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



 void TimelineWidget::mouseTimePositionChanged(const QPointF& scenePos)
 {
   if (m_timeline)
   {
     emit timePositionChanged(scenePos.x()/m_timeline->framerate());
     invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
     update();
     m_grabTime = true;
   }
 }

 void TimelineWidget::wheelEvent(QWheelEvent *event)
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


void TimelineWidget::setScale(const QPointF& scale)
{
  m_scale = scale;
  if (m_scale.x() > 1.0)
  {
    m_scale.setX(1.0);
  }
  else if (m_timeline && this->rect().width() >  m_timeline->sceneRect().width() * m_scale.x())
  {
    m_scale.setX((qreal)this->rect().width()/m_timeline->sceneRect().width());
  }
  QTransform t;
  t.translate(transform().dx(),transform().dy());
  t.scale(m_scale.x(),m_scale.y());
  setTransform(t,false);
  invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
  //update();
}

void TimelineWidget::focusInEvent(QFocusEvent* event)
{
  if (m_timeline)
  {
    emit currentRendererChanged(timeline->getRenderer());
  }
}

void TimelineWidget::startUpdateLoop()
{
  if (m_timeline)
  {
    connect(m_timeline,SIGNAL(destroyed()),this,SLOT(onTimelineDestroy()),Qt::DirectConnection);
    connect(m_updateTimer,SIGNAL(timeout()),this,SLOT(updateWithTime()));
    connect(this,SIGNAL(timePositionChanged(double)),m_timeline->music(),SLOT(setPosition(double)));
    m_updateTimer->start();
    m_updateTimer->setInterval(10.0);
  }
}

void TimelineWidget::stopUpdateLoop()
{
  if (m_timeline)
  {
    disconnect(m_updateTimer,SIGNAL(timeout()),this,SLOT(updateWithTime()));
    disconnect(this,SIGNAL(timePositionChanged(double)),m_timeline->music(),SLOT(setPosition(double)));
    disconnect(m_timeline,SIGNAL(destroyed()),this,SLOT(onTimelineDestroy()));
    m_updateTimer->stop();
  }
}
