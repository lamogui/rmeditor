#ifndef QUADFRAGMENTSHADER_HPP
#define QUADFRAGMENTSHADER_HPP

#include <QPointer>
#include "fast2dquad.hpp"
#include "renderer.hpp"
#include "shadercode.hpp"

class QuadFragmentShaderCode : public GLSLShaderCode
{
  Q_OBJECT

public:
  QuadFragmentShaderCode(QObject* parent = nullptr);

  // MediaFile Renderer
	bool canBeRendered() const override { return true; }
	Renderer* createRenderer() const override;

  // Accessors
  ShaderProgram* getShaderProgram() const { return shaderProgram; }

  // TextEditable
  bool buildable() const override { return true; }

  // Utils
  static const char* getVertexShaderCode();

public slots:
  // TextEditable
  bool build(const QString& text) override;

protected:
  ShaderProgram* shaderProgram;
};


/*
** ReferencedQuadFragmentShaderRenderer: Intermediate class for all renderer based on a signle fraglent shader (using a quad)
*/
class ReferencedQuadFragmentShaderRenderer : public Renderer
{
public:
  ReferencedQuadFragmentShaderRenderer() : Renderer() {}
  ~ReferencedQuadFragmentShaderRenderer() override {}

  // accesors
  const QPointer<const QuadFragmentShaderCode>& getShaderCode() const { return shaderCode; }
  void setShaderCode(const QuadFragmentShaderCode* s) { shaderCode = s; }

  // Rendering
  void initializeGL(RenderFunctionsCache& gl) override;
  void glRender(RenderFunctionsCache& gl, Render& render) override;

protected:
  // to override to set uniforms
  virtual void configureUniforms(ShaderProgram& program);

private:
  Fast2DQuad quad;
  QPointer<const QuadFragmentShaderCode> shaderCode;
};



#endif // !QUADFRAGMENTSHADER_HPP
