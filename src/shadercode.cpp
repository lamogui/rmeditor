
#include "shadercode.hpp"

#include "logwidget.hpp"
#include "shaderminifier.hpp"

FragmentShaderCode::FragmentShaderCode(const QString &filename,QDomNode node,LogWidget& log, QObject *parent):
  TextEditable(filename,node,log,parent)
{
  connectLog(log);
}

const QString &FragmentShaderCode::getText() const
{
  return fragmentcode;
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

bool FragmentShaderCode::handleShaderCompileResult(const QString& shaderCode, ShaderProgram& program, QOpenGLShader::ShaderType type)
{
  if (!program.addShaderFromSourceCode(type, shaderCode))
  {
    log.writeError(fileName() + " (at compile): " + program.log());
    return false;
  }
  else if (!program.log().isEmpty())
    log.writeWarning(fileName() + " (at compile): " + program.log());
  return true;
}

bool FragmentShaderCode::handleShaderLinkResult(ShaderProgram& program)
{
  if (!program.link())
  {
    log.writeError(fileName() + " (at link): " + program.log());
    return false;
  }
  else if (!program.log().isEmpty())
    log.writeWarning(fileName() + " (at link): " + program.log());
  return false;
}
