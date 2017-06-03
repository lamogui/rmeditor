#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP

#include "texteditable.hpp"

class ShaderMinifier;
class GLSLShaderCode : public TextEditable /* Represent any shader code media text */
{
  Q_OBJECT
  Q_PROPERTY(QWeakPointer<GLSLShaderCode> framework MEMBER framework READ getFramework WRITE setFramework)

public:
  GLSLShaderCode();
  GLSLShaderCode(const GLSLShaderCode& other);

  // accessors
  const QWeakPointer<GLSLShaderCode>& getFramework() const { return framework; }
  void setFramework(const QWeakPointer<GLSLShaderCode>& framework);

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
  QWeakPointer<GLSLShaderCode> framework;
};

Q_DECLARE_METATYPE(GLSLShaderCode);

#endif // !SHADERCODE_HPP
