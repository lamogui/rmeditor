#ifndef TIMELINEDOCKWIDGET_HPP
#define TIMELINEDOCKWIDGET_HPP

#include <QDockWidget>
#include "ui_demotimelinedockwidget.h"

class Project;
class Camera;
class Music;
class Renderer;
class DemoTimelineWidget;
class DemoTimelineDockWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit DemoTimelineDockWidget(QWidget *_parent = nullptr);

	void setProject(Project* _project);
	DemoTimelineDockWidget* getDemoTimelineWidget() const;

public slots:
	void insertCameraKeyframe(Camera* _cam);

signals:
	void currentRendererChanged(QWeakPointer<Renderer> _renderer);

protected:
	void focusInEvent(QFocusEvent* _event);

protected slots:
	void emitCurrentRendererChanged(const QWeakPointer<Renderer>& _renderer);

	void on_playPauseButton_clicked(bool);
	void on_rewindButton_clicked(bool);
	void on_stopButton_clicked(bool);

private:
	Ui::DemoTimelineDockWidget m_ui;
};

#endif
