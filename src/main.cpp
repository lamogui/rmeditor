#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "mainwindow.hpp"
#include <iostream>
#include <QtXml>
#include <QFile>
#include <QMessageBox>
#include "classmanager.hpp"

#ifdef MSVC_STATIC
  #include <QtPlugin>
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif // MSVC_STATIC

int main(int argc, char *argv[])
{
  ClassManager::get()->initalizeClasses();

  QApplication a(argc, argv);
  //QFile f(":qdarkstyle/style.qss");
  //f.open(QFile::ReadOnly | QFile::Text);
  //QTextStream ts(&f);
  //a.setStyleSheet(ts.readAll());

  MainWindow w;
  w.showMaximized();
    
  return a.exec();
}
