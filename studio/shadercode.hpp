#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP

#include "texteditable.hpp"
#include <QPointer>

class ShaderMinifier;

/*
** GLSLShaderCode : Represent any shader code media text 
*/
class GLSLShaderCode : public TextEditable 
{
  Q_OBJECT

public:
  GLSLShaderCode(QObject* parent = nullptr);

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
  DECLARE_PROPERTY_REFERENCE(QPointer<GLSLShaderCode>, framework, Framework)
};

#endif // SCENE_H
