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
  inline Timeline* getTimeline() { return timeline; }

  inline double getBorderHeight() const { return borderHeight; }
  inline double sceneBorderHeight() const { return getBorderHeight()/scale.y(); }

  void setScale(const QPointF &scale);
  inline const QPointF& getScale() const { return scale; }

  void startUpdateLoop();
  void stopUpdateLoop();

signals:
  void timePositionChanged(double);
  void currentRendererChanged(QWeakPointer<Renderer> renderer);


protected:
  void drawForeground(QPainter* painter, const QRectF& rect) override;
  void mousePressEvent(QMouseEvent* mouseEvent) override;
  void mouseMoveEvent(QMouseEvent* mouseEvent) override;
  void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
  void wheelEvent(QWheelEvent* event) override;
  void focusInEvent(QFocusEvent* event) override;


protected slots:
  void onTimelineDestroy();

private slots:
  void updateWithTime();

private:
  void mouseTimePositionChanged(const QPointF &scenePos);

 double lastTimeForegroundDraw; //This should be useless as a member now
 double lastTimeDraw;
 qreal borderHeight;
 QRectF lastBorderRect;
 Timeline* timeline;
 QTimer* updateTimer;
 bool grabTime;
 QPointF scale;

};

#endif
