#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>


namespace Ui {
class MainWindow;
}

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
    ~MainWindow() override;

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

private:
    void connectProject();

    Project* project;

    Ui::MainWindow *ui;
    LogDockWidget *info;
    EditorWidget *editor;
    TimelineDockWidget *timelineWidget;
};

#endif // MAINWINDOW_H
