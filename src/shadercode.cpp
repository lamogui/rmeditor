
#include "shadercode.hpp"

#include "logwidget.hpp"
#include "shaderminifier.hpp"

FragmentShaderCode::FragmentShaderCode(const QString &filename,QDomNode node,LogWidget& log, QObject *parent):
  TextEditable(filename,node,log,parent),
  shader(QOpenGLShader::Fragment)
{
  connectLog(log);
}

const QString &FragmentShaderCode::getText() const
{
  return fragmentcode;
}

bool FragmentShaderCode::build(const QString &text)
{
  fragmentcode = text;
  return handleShaderCompileResult(fragmentcode, QOpenGLShader::Fragment);
}

bool FragmentShaderCode::handleShaderCompileResult(const QString& shaderCode, QOpenGLShader::ShaderType type)
{
  if (shader.compileSourceCode(fragmentcode))
  {
    if (!shader.log().isEmpty())
      log.writeWarning(fileName() + ": " + shader.log());
    return true;
  }
  log.writeError(fileName() + ": " + shader.log());
  return false;
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

