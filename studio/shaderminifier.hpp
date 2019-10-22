#ifndef SHADERMINIFIER_HPP
#define SHADERMINIFIER_HPP

#include <QObject>

class LogWidget;

class ShaderMinifier : public QObject
{
  Q_OBJECT

public:
  ShaderMinifier(LogWidget& log);

  //Do the connections to log
  virtual void connectLog(LogWidget& log);


  QString minifiedShaderCode(const QString& filename, const QString& original) const;
  QString cFormatedShaderCode(const QString& filename, const QString &variable_name, const QString& minified) const;

  static QString generatedHeaderString();
  static QString generatedSourceString();

signals:
  void error(QString err) const;
  void warning(QString warn) const;
  void info(QString txt) const;

};

#endif
