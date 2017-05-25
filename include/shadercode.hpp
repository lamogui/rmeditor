#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP


#include <QString>
#include <QOpenGLShader>

#include "texteditable.hpp"

class ShaderMinifier;
class FragmentShaderCode : public TextEditable
{
  Q_OBJECT

public:
  FragmentShaderCode(const QString& filename, QDomNode node,LogWidget& log,QObject* parent=nullptr);

  const QString& getText() const override;
  inline QOpenGLShader& getShader() { return shader;}

  bool buildable() const override { return true; }

  virtual QString minifiedShaderCode(const ShaderMinifier& minifier) const;
  virtual QString cFormatedShaderCode(const ShaderMinifier& minifier) const;

public slots:
   bool build(const QString& text) override;

protected:
  // Utils
  bool handleShaderCompileResult(const QString& shaderCode, QOpenGLShader::ShaderType type);

  QOpenGLShader shader;
  QString fragmentcode;

};

#endif // !SHADERCODE_HPP
