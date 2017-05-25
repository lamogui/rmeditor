#ifndef LOGDOCKWIDGET_HPP
#define LOGDOCKWIDGET_HPP

#include <QDockWidget>
#include <QTextEdit>
#include "logwidget.hpp"

namespace Ui {
class LogDockWidget;
}

class LogDockWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit LogDockWidget(QWidget *parent = 0);
    ~LogDockWidget();
    
    LogWidget* getLogWidget();


private slots:
    void on_clearButton_clicked(bool);

    void logChanged();
private:
    Ui::LogDockWidget *ui;
};

#endif // !LOGDOCKWIDGET_HPP
