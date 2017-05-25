#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP


#include <QString>
#include "shader.hpp"
#include "texteditable.hpp"
#include "fbo.hpp"


class ShaderMinifier;

class FragmentShaderCode : public TextEditable
{
  Q_OBJECT

public:
  FragmentShaderCode(const QString& filename, QDomNode node,LogWidget& log,QObject* parent=nullptr);

  const QString& getText() const override;
  inline Shader& getShader() { return shader;}
  void connectLog(LogWidget& log) override;

  bool buildable() const override { return true; }


  virtual QString minifiedShaderCode(const ShaderMinifier& minifier) const;
  virtual QString cFormatedShaderCode(const ShaderMinifier& minifier) const;

public slots:
   bool build(const QString& text) override;


protected:
    Shader shader;
    QString fragmentcode;

};

#endif // !SHADERCODE_HPP
