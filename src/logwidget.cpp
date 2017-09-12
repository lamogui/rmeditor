#include <QTime>
#include <QString>
#include <QMetaMethod>
#include "logwidget.hpp"
#include <cassert>


LogWidget::LogWidget(QWidget *parent):
  QTextEdit(parent),
  printTime(true)
{
  
}

QString LogWidget::getFormattedDate()
{
  return QString("[")+QString::number(QTime::currentTime().hour())+":"+QString::number(QTime::currentTime().minute())+":"+QString::number(QTime::currentTime().second())+QString("] ");
}

LogWidget* LogWidget::getLogWidget(const QObject& context)
{
  QObject* parent = context.parent();
  while (parent)
  {
    LogWidget* log = qobject_cast<LogWidget*>(parent);
    if (log)
      return log;
    parent = parent->parent();
  }
  assert(false && "could not find LogWidget in parents");
  return nullptr;
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
    txt = getFormattedDate() + txt;
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
    txt = getFormattedDate() + txt;
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
    txt = getFormattedDate() + txt;
  }
  this->insertPlainText(txt+"\n");
}

void LogWidget::handleOpengGLLoggedMessage(const QOpenGLDebugMessage &debugMessage)
{
  if (debugMessage.type() & (QOpenGLDebugMessage::ErrorType))
    writeError(debugMessage.message());
  else if (debugMessage.type() &
    (QOpenGLDebugMessage::DeprecatedBehaviorType | QOpenGLDebugMessage::UndefinedBehaviorType |
      QOpenGLDebugMessage::PortabilityType | QOpenGLDebugMessage::PerformanceType | QOpenGLDebugMessage::InvalidType))
    writeWarning(debugMessage.message());
  else 
    writeInfo(debugMessage.message());
}

void LogWidget::findAndConnectLogSignalsRecursively(const QObject& object)
{
  const QMetaObject* meta = object.metaObject();
  if (meta)
  {
    if (meta->indexOfSignal("error(QString)") != -1)
      connect(&object, SIGNAL(error(QString)), this, SLOT(writeError(QString)));
    if (meta->indexOfSignal("warning(QString)") != -1)
      connect(&object, SIGNAL(warning(QString)), this, SLOT(writeWarning(QString)));
    if (meta->indexOfSignal("info(QString)") != -1)
      connect(&object, SIGNAL(info(QString)), this, SLOT(writeInfo(QString)));

    foreach(const QObject* child, object.children())
    {
      findAndConnectLogSignalsRecursively(*child);
    }
  }
}