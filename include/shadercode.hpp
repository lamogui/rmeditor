#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP

#include "texteditable.hpp"
#include <QPointer>

class ShaderMinifier;
class GLSLShaderCode : public TextEditable /* Represent any shader code media text */
{
  Q_OBJECT

public:
  GLSLShaderCode();
  GLSLShaderCode(const GLSLShaderCode& other); // for Q_DECLARE_METATYPE do not use !

  // TextEditable
  const QString& getText() const override;
  
  QString getShaderCodeRecursive() const;

  // overridable
  virtual QString minifiedShaderCode(const ShaderMinifier& minifier) const;
  virtual QString cFormatedShaderCode(const ShaderMinifier& minifier) const;

protected:
  // Utils
  bool handleShaderCompileResult(const QString& shaderCode, ShaderProgram& program, QOpenGLShader::ShaderType type);
  bool handleShaderLinkResult(ShaderProgram& program);

  QString shaderCode;

private:
  //QPointer<GLSLShaderCode> framework;
  DECLARE_PROPERTY_REFERENCE(QPointer<GLSLShaderCode>, framework, Framework)
};

// FIXME : create a class instatiator that to the job of QMetaType without need a copy constructor
Q_DECLARE_METATYPE(GLSLShaderCode);

#endif // !SHADERCODE_HPP
