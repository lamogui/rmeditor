#ifndef LOGDOCKWIDGET_HPP
#define LOGDOCKWIDGET_HPP

#include <QDockWidget>
#include "ui_logdockwidget.h"
#include "logwidget.hpp"

class LogDockWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit LogDockWidget(QWidget *parent = nullptr);

	// TMP remove this
	LogWidget* getLogWidget();

private slots:
	void on_clearButton_clicked(bool);

private:
	Ui::LogDockWidget m_ui;
};

#endif
