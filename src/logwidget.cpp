#include <QTime>
#include <QString>
#include "logwidget.hpp"


LogWidget::LogWidget(QWidget *parent):
  QTextEdit(parent),
  printTime(true)
{
}



QString LogWidget::getDate()
{
    return QString("[")+QString::number(QTime::currentTime().hour())+":"+QString::number(QTime::currentTime().minute())+":"+QString::number(QTime::currentTime().second())+QString("] ");
}

//Log
void LogWidget::writeInfo(QString txt)
{
  QTextCursor cur = this->textCursor();
  cur.movePosition(QTextCursor::End);
  this->setTextCursor(cur);
  this->setTextColor(QColor(0,150,0) );
  if(printTime)
  {
    txt = getDate() + txt;
  }
  this->insertPlainText(txt+"\n");
}
void LogWidget::writeWarning(QString txt)
{
  QTextCursor cur = this->textCursor();
  cur.movePosition(QTextCursor::End);
  this->setTextCursor(cur);
  this->setTextColor( QColor(255,255,100) );
  if(printTime)
  {
    txt = getDate() + txt;
  }
  this->insertPlainText(txt+"\n");
}
void LogWidget::writeError(QString txt)
{
  QTextCursor cur = this->textCursor();
  cur.movePosition(QTextCursor::End);
  this->setTextCursor(cur);
  this->setTextColor( QColor(255,0,0) );
  if(printTime)
  {
    txt = getDate() + txt;
  }
  this->insertPlainText(txt+"\n");
}
