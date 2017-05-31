#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP


#include <QString>

#include "renderfunctionscache.hpp"
#include "texteditable.hpp"

class ShaderMinifier;
class FragmentShaderCode : public TextEditable
{
  Q_OBJECT

public:
  FragmentShaderCode(const QString& filename, QDomNode node,LogWidget& log,QObject* parent=nullptr);

  const QString& getText() const override;

  virtual QString minifiedShaderCode(const ShaderMinifier& minifier) const;
  virtual QString cFormatedShaderCode(const ShaderMinifier& minifier) const;

protected:
  // Utils
  bool handleShaderCompileResult(const QString& shaderCode, ShaderProgram& program, QOpenGLShader::ShaderType type);
  bool handleShaderLinkResult(ShaderProgram& program);

  QString fragmentcode;

};

#endif // !SHADERCODE_HPP
