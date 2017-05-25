#include "texteditable.hpp"
#include <QTextStream>


TextEditable::TextEditable(const QString &filename, QDomNode node, LogWidget &log, QObject *parent):
  NodeFile(filename,node,log,parent)
{
}

bool TextEditable::load()
{
  QString text;
  bool readSuccess = false;
  if (!this->exists())
  {
    emit warning(QString("File ") + fileName() + QString(" doesn't exists create it"));
    if (!open(QIODevice::WriteOnly))
    {
      emit error(QString("Can't create file ") + fileName());
    }
    else
    {
      readSuccess = true;
    }
  }
  else
  {
    if (!open(QIODevice::ReadOnly))
    {
      emit error(QString("Can't open file ") + fileName());
    }
    else
    {
      QTextStream stream(this);
      text = stream.readAll();
      readSuccess = (stream.status() == QTextStream::Ok);
    }
  }
  close();
  build(text);
  return readSuccess;
}

bool TextEditable::save(const QString& text)
{
  if (!open(QIODevice::WriteOnly))
  {
    emit error(QString("Can't open file ") + fileName() + QString(" for wrinting"));
    return false;
  }

  QTextStream stream(this);
  stream << text;
  stream.flush();

  close();
  return true;
}


bool TextEditable::save()
{
  return save(getText());
}




