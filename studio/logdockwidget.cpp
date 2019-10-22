#include "logdockwidget.hpp"

LogDockWidget::LogDockWidget(QWidget *parent) :
  QDockWidget(parent)
{
	m_ui.setupUi(this);
}

void LogDockWidget::on_clearButton_clicked(bool)
{
	m_ui.m_logWidget->clear();
}

// TMP Remove this
LogWidget* LogDockWidget::getLogWidget()
{
	return m_ui.m_logWidget;
}
