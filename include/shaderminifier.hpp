#ifndef SHADERMINIFIER_HPP
#define SHADERMINIFIER_HPP

#include <QObject>

class ShaderMinifier : public QObject
{
  Q_OBJECT

public:
  ShaderMinifier();

  QString minifiedShaderCode(const QString& filename, const QString& original) const;
  QString cFormatedShaderCode(const QString& filename, const QString &variable_name, const QString& minified) const;

signals:
  // Log
  void error(QString err) const;
  void warning(QString warn) const;
  void info(QString txt) const;

};

#endif
