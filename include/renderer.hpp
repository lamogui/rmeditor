
#ifndef RMEDITOR_RENDERER_HPP
#define RMEDITOR_RENDERER_HPP

#include "renderfunctionscache.hpp" 

class Render;
class Renderer
{
  public:
    Renderer() {}
    virtual ~Renderer() {}

    virtual void glRender(RenderFunctionsCache& gl) = 0;
};

#include "fast2dquad.hpp"
#include <QOpenGLShaderProgram>

class SimpleQuadWithFragmentShaderRenderer : public Renderer
{
public:
  SimpleQuadWithFragmentShaderRenderer() {}
  ~SimpleQuadWithFragmentShaderRenderer() override {}

  void glRender(RenderFunctionsCache& gl) override
  {
    QSharedPointer<QOpenGLShaderProgram> shader = shaderProgram.lock();
    Q_ASSERT(shader);
    shader->bind();
    configureUniforms(*shader);
    quad.draw();
    shader->release();
  }

  const QWeakPointer<QOpenGLShaderProgram>& getShaderProgram() const {}
  inline void setShaderProgram(QSharedPointer<QOpenGLShaderProgram>& shaderProgram)
  {
    this->shaderProgram = shaderProgram;
    configureConstUniforms(*shaderProgram);
  }

  protected:
    // to override to set uniforms
    virtual void configureConstUniforms(QOpenGLShaderProgram& program) {}
    virtual void configureUniforms(QOpenGLShaderProgram& program) {}

  private:
    QWeakPointer<QOpenGLShaderProgram> shaderProgram; 
    Fast2DQuad quad;
      
};



#endif // !RMEDITOR_RENDERER_HPP