#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "mainwindow.hpp"
#include <iostream>
#include <QtXml>
#include <QFile>
#include <QMessageBox>
#include "logmanager.hpp"

int main(int argc, char *argv[])
{
	g_logManager = std::move( std::unique_ptr< Log::Manager >( new Log::Manager() ) );
	QApplication a(argc, argv);
	QFile f(":qdarkstyle/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	a.setStyleSheet(ts.readAll());

	MainWindow w;
	w.showMaximized();
	return a.exec();
}
