
#ifndef RMEDITOR_RENDERER_HPP
#define RMEDITOR_RENDERER_HPP

#include "renderfunctionscache.hpp" // because it's a typedef
#include <QHash>
#include <QSharedPointer>


class Render;
class Renderer
{
  public:
    Renderer() {}
    virtual ~Renderer() {}

    virtual void renderChildrens(RenderFunctionsCache& gl) {}            // Pre-render all the internals FBOs/GeometryShaders before "real" render
    virtual void glRender(RenderFunctionsCache& gl, Render& render) = 0; // You must not bind any FBO in this function 
    virtual bool hasDynamicCamera() const { return false; }              // Can we control the camera of the renderer

};

#include "fast2dquad.hpp"

class ReferencedQuadFragmentShaderRenderer : public Renderer
{
public:
  ReferencedQuadFragmentShaderRenderer() {}
  ~ReferencedQuadFragmentShaderRenderer() override {}

  void glRender(RenderFunctionsCache& gl, Render& render) override
  {
    QSharedPointer<ShaderProgram> shader = shaderProgram.lock();
    Q_ASSERT(shader);
    shader->bind();
    configureUniforms(*shader);
    quad.draw();
    shader->release();
  }

  const QWeakPointer<ShaderProgram>& getShaderProgram() const {}
  inline void setShaderProgram(QSharedPointer<ShaderProgram>& shaderProgram)
  {
    this->shaderProgram = shaderProgram;
    configureConstUniforms(*shaderProgram);
  }

  protected:
    // to override to set uniforms
    virtual void configureConstUniforms(ShaderProgram& program) {}
    virtual void configureUniforms(ShaderProgram& program) {}

  private:
    QWeakPointer<ShaderProgram> shaderProgram;
    Fast2DQuad quad;
      
};



#endif // !RMEDITOR_RENDERER_HPP