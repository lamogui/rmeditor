

#include "timelinewidget.hpp"

#include <QDebug>

#include <QBrush>
#include <QMouseEvent>
#include <QPen>
#include <QScrollBar>
#include <QTime>
#include <QTimer>
#include <cmath> // fmod

#include "music.hpp"
#include "timeline.hpp"

TimelineWidget::TimelineWidget(QWidget *parent, qreal borderheight):
  QGraphicsView(parent),
  borderHeight(borderheight),
  timeline(nullptr),
  updateTimer(new QTimer(this)),
  grabTime(false),
  scale(1,1)
{
  updateTimer->setSingleShot(false);
}

void TimelineWidget::setTimeline(Timeline *timeline)
{
  stopUpdateLoop();
  this->timeline = timeline;
  startUpdateLoop();

  setScene(this->timeline);
  if(this->timeline)
  {
    setScale(QPointF((qreal)this->rect().width()/ this->timeline->sceneRect().width(),1.0));
    QRectF scene_rect = this->timeline->sceneRect();
    scene_rect.setTop(-borderHeight - 2.0);
    this->timeline->setSceneRect(scene_rect);
  }
  else
  {
    setScale(QPointF(1.0,1.0));
  }

}

void TimelineWidget::updateWithTime()
{
  double t = timeline->getMusic()->getTime();
  if (t != lastTimeDraw)
  {
    timeline->updateTime();

    if (t > timeline->getMusic()->getLength())
    {
      emit timePositionChanged(0);
    }

    QScrollBar* horizontal = this->horizontalScrollBar();
    qreal scroll_length = horizontal->maximum() - horizontal->minimum() + horizontal->pageStep();
    qreal scroll_pos = t*scroll_length/timeline->getMusic()->getLength();

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

    lastTimeDraw = t;
  }
}

void TimelineWidget::onTimelineDestroy()
{

  disconnect(updateTimer,SIGNAL(timeout()),this,SLOT(updateWithTime()));
  disconnect(this,SIGNAL(timePositionChanged(double)),0,0);
  updateTimer->stop();
  timeline=nullptr;
  setScene(timeline);
}


void TimelineWidget::drawForeground(QPainter* painter, const QRectF& rect)
{

  QScrollBar *vertical = this->verticalScrollBar(), *horizontal = this->horizontalScrollBar();
  qreal x = sceneRect().width() * (horizontal->value() - horizontal->minimum())/
            (qreal)(horizontal->maximum() - horizontal->minimum() + horizontal->pageStep());
  qreal y = sceneRect().height() * (vertical->value() - vertical->minimum())/
            (qreal)(vertical->maximum() - vertical->minimum() + vertical->pageStep());

  QPointF delta(x,y-getBorderHeight()-2.0);
  qreal visibleWidth = sceneRect().width()  * (qreal)horizontal->pageStep()/(qreal)(horizontal->maximum() - horizontal->minimum() + horizontal->pageStep());


  //qDebug() << delta << visibleWidth << sceneRect();

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
    if (timeline)
    {
      framerate = timeline->getFramerate();
      lastTimeForegroundDraw = timeline->getMusic()->getTime();
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
      textFont.setStretch((int)(100 * 1.0 / scale.x()));
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

    qreal lineWidth = 2.0 / scale.x();
    linePen.setWidthF(lineWidth);
    painter->setPen(linePen);
    double t = lastTimeForegroundDraw * framerate;
    if (t >= rect.left() && t <= rect.right())
    {
      QLineF line(QPointF(t,rect.top()),QPointF(t,rect.bottom()));
      painter->drawLine(line);
    }

    lastBorderRect = border;
  }

  QGraphicsView::drawForeground(painter,rect);
}

void TimelineWidget::mousePressEvent(QMouseEvent* mouseEvent)
{
  if (timeline)
  {
    emit rendererChanged(timeline->getRender());
  }

  QPointF scenePos = mapToScene(mouseEvent->pos());
  if (lastBorderRect.contains(scenePos))
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
   if (grabTime)
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
   if (grabTime)
   {
     grabTime = false;
   }
   else
   {
     QGraphicsView::mouseReleaseEvent(mouseEvent);
   }
   grabTime = false;
 }



 void TimelineWidget::mouseTimePositionChanged(const QPointF& scenePos)
 {
   if (timeline)
   {
     emit timePositionChanged(scenePos.x()/timeline->getFramerate());
     invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
     update();
     grabTime = true;
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
     //scale.setY(scale.y()*delta);
   }
   else
   {
     scale.setX(scale.x()*delta);
   }
   setScale(scale);
 }


void TimelineWidget::setScale(const QPointF& scale)
{
  this->scale = scale;
  if (scale.x() > 1.0)
  {
    this->scale.setX(1.0);
  }
  else if (timeline && this->rect().width() >  timeline->sceneRect().width() * scale.x())
  {
    this->scale.setX((qreal)this->rect().width()/timeline->sceneRect().width());
  }
  QTransform t;
  t.translate(transform().dx(),transform().dy());
  t.scale(scale.x(),scale.y());
  setTransform(t,false);
  invalidateScene(sceneRect(),QGraphicsScene::ForegroundLayer);
  //update();
}

void TimelineWidget::focusInEvent(QFocusEvent* event)
{
  if (timeline)
  {
    emit rendererChanged(timeline->getRender());
  }
}

void TimelineWidget::startUpdateLoop()
{
  if (timeline)
  {
    connect(timeline,SIGNAL(destroyed()),this,SLOT(onTimelineDestroy()),Qt::DirectConnection);
    connect(updateTimer,SIGNAL(timeout()),this,SLOT(updateWithTime()));
    connect(this,SIGNAL(timePositionChanged(double)),timeline->getMusic(),SLOT(setPosition(double)));
    updateTimer->start();
    updateTimer->setInterval(10.0);
  }
}

void TimelineWidget::stopUpdateLoop()
{
  if (timeline)
  {
    disconnect(updateTimer,SIGNAL(timeout()),this,SLOT(updateWithTime()));
    disconnect(this,SIGNAL(timePositionChanged(double)),timeline->getMusic(),SLOT(setPosition(double)));
    disconnect(timeline,SIGNAL(destroyed()),this,SLOT(onTimelineDestroy()));
    updateTimer->stop();
  }
}
