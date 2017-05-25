
#include "shadercode.hpp"

#include "logwidget.hpp"
#include "shaderminifier.hpp"

FragmentShaderCode::FragmentShaderCode(const QString &filename,QDomNode node,LogWidget& log, QObject *parent):
  TextEditable(filename,node,log,parent)
{
  connectLog(log);
}

void FragmentShaderCode::connectLog(LogWidget &log)
{
  TextEditable::connectLog(log);
  connect(&shader,SIGNAL(error(QString)),&log,SLOT(writeError(QString)));
  connect(&shader,SIGNAL(warning(QString)),&log,SLOT(writeWarning(QString)));
}


const QString &FragmentShaderCode::getText() const
{
  return fragmentcode;
}

bool FragmentShaderCode::build(const QString &text)
{
  fragmentcode = text;
  return (shader.compil(Shader::getVertexShader(), fragmentcode.toStdString().c_str()) == SHADER_SUCCESS);
}


QString FragmentShaderCode::minifiedShaderCode(const ShaderMinifier& minifier) const
{
  return minifier.minifiedShaderCode(this->fileName(),fragmentcode);
}

QString FragmentShaderCode::cFormatedShaderCode(const ShaderMinifier& minifier) const
{
  QString variable_name = QString("fs_") + this->objectName();
  return minifier.cFormatedShaderCode(this->fileName(),variable_name,minifiedShaderCode(minifier));
}

