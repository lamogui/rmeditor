#include "texteditable.hpp"
#include <QTextStream>

TextEditable::TextEditable(QObject* parent) :
  MediaFile(parent)
{
}

bool TextEditable::load()
{
  QString text;
  QFile file(getPath().absoluteFilePath());
  bool readSuccess = false;
  if (!file.exists())
  {
    Log::Warning(QString("File ") + getPath().fileName() + QString(" doesn't exists create it"));
    if (!file.open(QIODevice::WriteOnly))
    {
      Log::Error(QString("Can't create file ") + getPath().absoluteFilePath());
    }
    else
    {
      readSuccess = true;
    }
  }
  else
  {
    if (!file.open(QIODevice::ReadOnly))
    {
      Log::Error(QString("Can't open file ") + getPath().absoluteFilePath());
    }
    else
    {
      QTextStream stream(&file);
      text = stream.readAll();
      readSuccess = (stream.status() == QTextStream::Ok);
    }
  }
  file.close();
  return readSuccess && build(text);
}

bool TextEditable::save(const QString& text)
{
  QFile file(getPath().absoluteFilePath());
  if (!file.open(QIODevice::WriteOnly))
  {
    Log::Error(QString("Can't open file ") + getPath().absoluteFilePath() + QString(" for wrinting"));
    return false;
  }

  QTextStream stream(&file);
  stream << text;
  stream.flush();
  file.close();

  return true;
}


bool TextEditable::save()
{
  return save(text());
}

