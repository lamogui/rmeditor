#include "logdockwidget.hpp"
#include "ui_logdockwidget.h"

LogDockWidget::LogDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::LogDockWidget)
{
    ui->setupUi(this);
    connect(ui->logWidget, SIGNAL(textChanged()), this, SLOT(logChanged()));
}

LogDockWidget::~LogDockWidget()
{
    delete ui;
}

LogWidget *LogDockWidget::getLogWidget()
{
  return ui->logWidget;
}

void LogDockWidget::logChanged()
{
  ui->logWidget->ensureCursorVisible();
}

void LogDockWidget::on_clearButton_clicked(bool)
{
  getLogWidget()->clear();
}
