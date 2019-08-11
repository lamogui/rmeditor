
#include "shaderminifier.hpp"
#include "logmanager.hpp"

ShaderMinifier::ShaderMinifier()
{
}

QString ShaderMinifier::minifiedShaderCode(const QString &filename, const QString& original) const
{
  QString minified = QString::fromStdString(original.toStdString());
  minified.replace(QChar('\r'),QString());
  minified.replace(QChar('\t'),QString(" "));


  /* remove all comments */
  int startComment;
  while ((startComment = minified.indexOf("//"))!=-1)
  {
    int endComment = minified.indexOf("\n",startComment);
    if (endComment == -1)
    {
      Log::Warning(QString("[" + filename + "] : ") + tr("Missing new line after '//' comment !"));
      break;
    }
    else
    {
      minified.remove(startComment, endComment - startComment );
    }
  }
  while ((startComment = minified.indexOf("/*"))!=-1)
  {
    int endComment = minified.indexOf("*/",startComment);
    if (endComment == -1)
    {
      Log::Error(QString("[" + filename + "] : ") + tr("Unterminated multiline comment cannot minifie properly !"));
      break;
    }
    else
    {
      minified.remove(startComment, endComment + 2 - startComment );
    }
  }

  /* remove double spaces/newlines */
  while (minified.contains("  "))
  {
    minified.replace(QString("  "),QString(" "));
  }
  while (minified.contains(" \n"))
  {
    minified.replace(QString(" \n"),QString("\n"));
  }
  while (minified.contains("\n "))
  {
    minified.replace(QString("\n "),QString("\n"));
  }

  Log::Info(QString("[" + filename + "] ") + tr("Minified shadercode size: ") + QString::number(minified.length() + 1) + tr(" bytes"));

  return minified;
}

QString ShaderMinifier::cFormatedShaderCode(const QString &filename, const QString& variable_name, const QString& minified) const
{
  int indentLvl = 0;
  QString sCode;
  QString cCode;
  QStringList lines = minified.split('\n',QString::SkipEmptyParts);

  unsigned int size = 0;

 QString previous_line;
  foreach (QString line, lines)
  {
    QString begin;
    QString end;
    line.replace("\"","\\\"");
    if (line.contains("#") || line.contains("\\"))
    {
      if (!previous_line.contains("#") && line.contains("#"))
      {
        begin = "\\n";
      }
      end = "\\n";
    }

    if (line.contains("}"))
    {
      indentLvl-=2;
    }
    sCode += "\n" + QString().fill(' ',indentLvl) + "\"" + begin + line + end + "\"";
    size += (line+end).length();
    if (line.contains("{"))
    {
      indentLvl+=2;
    }
    previous_line = line;
  }
  sCode += ";\n";

  //cCode = QString("const char ") + variable_name + "[" +QString::number(size + 1) + "]" + " = ";
  cCode = QString("const char* const ") + variable_name + " = ";
  cCode += sCode;
  //cCode += QString("const unsigned int ") + variable_name + "_len = " + QString::number(size + 1) + ";\n";
  Log::Info(QString("[" + filename + "] ") + tr("C formated shadercode size: ") + QString::number(size + 1) + tr(" bytes"));
  return cCode;
}

