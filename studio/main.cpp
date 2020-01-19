#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "mainwindow.hpp"
#include <iostream>
#include <QtXml>
#include <QFile>
#include <QMessageBox>
#include "logmanager.hpp"
#include "styles.hpp"

int main(int argc, char *argv[])
{
	g_logManager = std::move( std::unique_ptr< Log::Manager >( new Log::Manager() ) );
	QApplication a(argc, argv);
	QFile f(":/resources/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	g_generalWidgetStyleSheet = ts.readAll();
	a.setStyleSheet( g_generalWidgetStyleSheet );

	MainWindow w;
	w.showMaximized();
	return a.exec();
}
