#include <QTime>
#include <QString>
#include <QMetaMethod>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include "logwidget.hpp"
#include "jassert.hpp"


LogWidget::LogWidget(QWidget *parent):
  QTextEdit(parent),
  m_printTime(true)
{
  
}

QString LogWidget::getFormattedDate()
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
  if(m_printTime)
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
  if(m_printTime)
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
  if(m_printTime)
  {
    txt = getFormattedDate() + txt;
  }
  this->insertPlainText(txt+"\n");
}


/*
void LogWidget::findAndConnectLogSignalsRecursively(const QObject& object)
{
  const QMetaObject* meta = object.metaObject();
  if (meta)
  {
    if (meta->indexOfSignal("error(QString)") != -1)
    {
      disconnect(&object, SIGNAL(error(QString)), this, SLOT(writeError(QString))); 
      connect(&object, SIGNAL(error(QString)), this, SLOT(writeError(QString)));
    }
    if (meta->indexOfSignal("warning(QString)") != -1)
    {
      disconnect(&object, SIGNAL(warning(QString)), this, SLOT(writeWarning(QString)));
      connect(&object, SIGNAL(warning(QString)), this, SLOT(writeWarning(QString)));
    }
    if (meta->indexOfSignal("info(QString)") != -1)
    {
      disconnect(&object, SIGNAL(info(QString)), this, SLOT(writeInfo(QString)));
      connect(&object, SIGNAL(info(QString)), this, SLOT(writeInfo(QString)));
    }


    // TODO : Make a ConnectObjectRecurssively or DoSomethingRecurssively function helper 
    if (object.metaObject()->inherits(&QGraphicsObject::staticMetaObject))
    {
      const QGraphicsObject* graphicsObject = qobject_cast<const QGraphicsObject*>(&object);
      foreach(const QGraphicsItem* child, graphicsObject->childItems())
      {
        const QGraphicsObject* childGraphicsObject = dynamic_cast<const QGraphicsObject*>(child);
        if (childGraphicsObject)
          findAndConnectLogSignalsRecursively(*childGraphicsObject);
      }
    }
    else if (object.metaObject()->inherits(&QGraphicsScene::staticMetaObject))
    {
      const QGraphicsScene* graphicsScene = qobject_cast<const QGraphicsScene*>(&object);
      foreach(const QGraphicsItem* child, graphicsScene->items())
      {
        const QGraphicsObject* childGraphicsObject = dynamic_cast<const QGraphicsObject*>(child);
        if (childGraphicsObject)
          findAndConnectLogSignalsRecursively(*childGraphicsObject);
      }
    }
    foreach(const QObject* child, object.children())
    {
      findAndConnectLogSignalsRecursively(*child);
    }
  }
  else
    jassertfalse;
}

*/