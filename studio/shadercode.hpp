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

  virtual const QString& text() const;
  inline Shader& getShader() { return m_shader;}
  virtual void connectLog(LogWidget& log);

  virtual bool buildable() const { return true; }


  virtual QString minifiedShaderCode(const ShaderMinifier& minifier) const;
  virtual QString cFormatedShaderCode(const ShaderMinifier& minifier) const;



public slots:
  virtual bool build(const QString& text);


protected:
    Shader m_shader;
    QString m_fragmentcode;

};

#endif // SCENE_H
