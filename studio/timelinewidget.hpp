#ifndef TIMELINEWIDGET_HPP
#define TIMELINEWIDGET_HPP

#include <QGraphicsView>

class Renderer;

class Timeline;

class TimelineWidget : public QGraphicsView
{
  Q_OBJECT

public:
  TimelineWidget(QWidget* parent, qreal borderheight = 25.0);

  void setTimeline(Timeline* timeline);
  inline Timeline* timeline() { return m_timeline; }

  inline double borderHeight() const { return m_borderHeight; }
  inline double sceneBorderHeight() const { return borderHeight()/m_scale.y(); }

  void setScale(const QPointF &scale);
  inline const QPointF& scale() const { return m_scale; }

  void startUpdateLoop();
  void stopUpdateLoop();

signals:
  void timePositionChanged(double);
  void rendererChanged(Renderer* renderer);


protected:
  virtual void drawForeground(QPainter* painter, const QRectF& rect);
  virtual void mousePressEvent(QMouseEvent* mouseEvent);
  virtual void mouseMoveEvent(QMouseEvent* mouseEvent);
  virtual void mouseReleaseEvent(QMouseEvent* mouseEvent);
  virtual void wheelEvent(QWheelEvent* event);
  virtual void focusInEvent(QFocusEvent* event);


protected slots:
  void onTimelineDestroy();

private slots:
  void updateWithTime();

private:
  void mouseTimePositionChanged(const QPointF &scenePos);

 double m_lastTimeForegroundDraw; //This should be useless as a member now
 double m_lastTimeDraw;
 qreal m_borderHeight;
 QRectF m_lastBorderRect;
 Timeline* m_timeline;
 QTimer* m_updateTimer;
 bool m_grabTime;
 QPointF m_scale;

};

#endif
