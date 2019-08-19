#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "ui_mainwindow.h"

class MediaFilesEditorWidget;
class LogDockWidget;
class TimelineDockWidget;
class Timeline;
class Project;
/*
class Project;
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() override;

public slots:
    void newProject();
    void open();
    void saveAllShaders();
    void insertCameraKeyframe();

    void exportAsVideo();
    void exportAsLinuxDemo();

    void freezeAll();
    void unfreezeAll();

private:
    void resetProject();
    void initializeProject();


    Project* m_project;

		Ui::MainWindow m_ui;
		LogDockWidget m_info;
    EditorWidget *m_editor;
    TimelineDockWidget *m_timeline;
};

#endif // MAINWINDOW_H
