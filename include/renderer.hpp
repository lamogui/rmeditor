
#ifndef RMEDITOR_RENDERER_HPP
#define RMEDITOR_RENDERER_HPP

#include "renderfunctionscache.hpp" 

class Render;
class Renderer
{
  public:
    Renderer() {}
    virtual ~Renderer() {}

    virtual void glRender(RenderFunctionsCache& gl, Render& target) = 0;
};

#include "fast2dquad.hpp"
#include <QOpenGLShaderProgram>

class SimpleQuadWithFragmentShaderRenderer : public Renderer
{
public:
  SimpleQuadWithFragmentShaderRenderer() {}
  ~SimpleQuadWithFragmentShaderRenderer() override {}

  virtual void glRender(RenderFunctionsCache& gl) = 0;

  const QWeakPointer<QOpenGLShaderProgram>& getShaderProgram() const {}
  inline void setShaderProgram(QSharedPointer<QOpenGLShaderProgram>& shaderProgram)
  {
    this->shaderProgram = shaderProgram;
    shaderProgramChangedCallback(*shaderProgram);
  }

  protected:
    // to override to set const uniforms
    virtual void shaderProgramChangedCallback(QOpenGLShaderProgram& program) {}

  private:
    QWeakPointer<QOpenGLShaderProgram> shaderProgram; 
    Fast2DQuad quad;
      
};



#endif // !RMEDITOR_RENDERER_HPP