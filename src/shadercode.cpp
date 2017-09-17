
#include "shadercode.hpp"

#include "logwidget.hpp"
#include "shaderminifier.hpp"

/*
** GLSLShaderCode
*/

GLSLShaderCode::GLSLShaderCode(QObject* parent) :
  TextEditable(parent)
{

}

GENERATE_PROPERTY_SETTER_REFERENCE(GLSLShaderCode, QPointer<GLSLShaderCode>, framework, Framework)

const QString &GLSLShaderCode::getText() const
{
  return shaderCode;
}

QString GLSLShaderCode::getShaderCodeRecursive() const
{
  if (framework)
  {
    return framework->getShaderCodeRecursive() + getText();
  }
  return getText();
}

QString GLSLShaderCode::minifiedShaderCode(const ShaderMinifier& minifier) const
{
  return minifier.minifiedShaderCode(getPath().fileName(), shaderCode);
}

QString GLSLShaderCode::cFormatedShaderCode(const ShaderMinifier& minifier) const
{
  QString variable_name = QString("fs_") + this->objectName();
  return minifier.cFormatedShaderCode(getPath().fileName(),variable_name,minifiedShaderCode(minifier));
}

bool GLSLShaderCode::handleShaderCompileResult(const QString& shaderCode, ShaderProgram& program, QOpenGLShader::ShaderType type)
{
  if (!program.addShaderFromSourceCode(type, shaderCode))
  {
    Log::Error(getPath().fileName() + " (at compile): " + program.log());
    return false;
  }
  else if (!program.log().isEmpty())
   Log::Warning(getPath().fileName() + " (at compile): " + program.log());
  return true;
}

bool GLSLShaderCode::handleShaderLinkResult(ShaderProgram& program)
{
  if (!program.link())
  {
    Log::Error(getPath().fileName() + " (at link): " + program.log());
    return false;
  }
  else if (!program.log().isEmpty())
    Log::Warning(getPath().fileName() + " (at link): " + program.log());
  return true;
}

