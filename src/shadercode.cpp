
#include "shadercode.hpp"

#include "logwidget.hpp"
#include "shaderminifier.hpp"

/*
** GLSLShaderCode
*/

GLSLShaderCode::GLSLShaderCode() :
  TextEditable()
{

}

GLSLShaderCode::GLSLShaderCode(const GLSLShaderCode& other) :
  TextEditable(other)
{

}

void GLSLShaderCode::setFramework(const QWeakPointer<GLSLShaderCode>& newFramework)
{
  if (newFramework != framework)
  {
    QWeakPointer<GLSLShaderCode> oldFramework = framework;
    framework = newFramework;
    emit propertyChanged(this, "framework", QVariant::fromValue(oldFramework), QVariant::fromValue(newFramework));
  }
}

const QString &GLSLShaderCode::getText() const
{
  return shaderCode;
}

QString GLSLShaderCode::getShaderCodeRecursive() const
{
  if (framework)
  {
    return framework.lock()->getShaderCodeRecursive() + getText();
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
    emit error(getPath().fileName() + " (at compile): " + program.log());
    return false;
  }
  else if (!program.log().isEmpty())
   emit warning(getPath().fileName() + " (at compile): " + program.log());
  return true;
}

bool GLSLShaderCode::handleShaderLinkResult(ShaderProgram& program)
{
  if (!program.link())
  {
    emit error(getPath().fileName() + " (at link): " + program.log());
    return false;
  }
  else if (!program.log().isEmpty())
    emit warning(getPath().fileName() + " (at link): " + program.log());
  return true;
}

