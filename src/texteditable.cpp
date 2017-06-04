#include "texteditable.hpp"
#include <QTextStream>

TextEditable::TextEditable() :
  MediaFile()
{
  connect(this, SIGNAL(pathChanged(QFileInfo)), this, SLOT(load()));
}

TextEditable::TextEditable(const TextEditable& other) :
  MediaFile(other)
{
  Q_ASSERT(false); // Humhum should not be called !
  connect(this, SIGNAL(pathChanged(QFileInfo)), this, SLOT(load()));
}

bool TextEditable::load()
{
  QString text;
  QFile file(getPath().absoluteFilePath());
  bool readSuccess = false;
  if (!file.exists())
  {
    emit warning(QString("File ") + getPath().fileName() + QString(" doesn't exists create it"));
    if (!file.open(QIODevice::WriteOnly))
    {
      emit error(QString("Can't create file ") + getPath().absoluteFilePath());
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
      emit error(QString("Can't open file ") + getPath().absoluteFilePath());
    }
    else
    {
      QTextStream stream(&file);
      text = stream.readAll();
      readSuccess = (stream.status() == QTextStream::Ok);
    }
  }
  file.close();
  build(text);
  return readSuccess;
}

bool TextEditable::save(const QString& text)
{
  QFile file(getPath().absoluteFilePath());
  if (!file.open(QIODevice::WriteOnly))
  {
    emit error(QString("Can't open file ") + getPath().absoluteFilePath() + QString(" for wrinting"));
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
  return save(getText());
}




