#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "ui_mainwindow.h"

class EditorWidget;
class LogDockWidget;
class Project;
class TimelineDockWidget;
class Timeline;
class MainWindow : public QMainWindow
{
	Q_OBJECT
    
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void newProject();
	void open();
	void saveAllShaders();
	void setTimeline(Timeline* t);
	void insertCameraKeyframe();

	void exportAsVideo();
	void exportAsLinuxDemo();

	void freezeAll();
	void unfreezeAll();

protected:
	void paintEvent(QPaintEvent * _pe) override;

private:
	void connectProject();

	Project* m_project;

	Ui::MainWindow m_ui;
	LogDockWidget * m_info;
	EditorWidget * m_editor;
	TimelineDockWidget * m_timeline;
	QPixmap m_backgroundImage;
};

#endif // MAINWINDOW_H
