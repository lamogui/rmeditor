#ifndef TIMELINEDOCKWIDGET_HPP
#define TIMELINEDOCKWIDGET_HPP

#include <QDockWidget>

class Camera;
class Music;
class Renderer;
class Timeline;
class TimelineWidget;


namespace Ui {
class TimelineDockWidget;
}

class TimelineDockWidget : public QDockWidget
{
    Q_OBJECT

public:
  explicit TimelineDockWidget(QWidget *parent = 0);
  virtual ~TimelineDockWidget();


  void setTargetTimeline(Timeline* timeline);
  Timeline* getTargetTimeline() const;
  TimelineWidget* getTimelineWidget() const;

public slots:
  void insertCameraKeyframe(Camera* cam);

signals:
  void currentRendererChanged(QWeakPointer<Renderer> renderer);

protected:
  void focusInEvent(QFocusEvent* event);

protected slots:
  void emitCurrentRendererChanged(const QWeakPointer<Renderer>& renderer);

  void on_playPauseButton_clicked(bool);
  void on_rewindButton_clicked(bool);
  void on_stopButton_clicked(bool);

private:
    Ui::TimelineDockWidget *ui;

    Project* m_project;
};

#endif
