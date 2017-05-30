
#ifndef RMEDITOR_RENDERER_HPP
#define RMEDITOR_RENDERER_HPP

#include "renderfunctionscache.hpp" // because it's a typedef
#include <QHash>
#include <QSharedPointer>

class Renderer;
class RendererComponent
{
public:
  enum Type
  {
    Camera
  };

  RendererComponent(Type type, Renderer* owner = nullptr) : owner(owner), type(type) {}

  Type getType() const { return type; }
  Renderer* getOwner() const { return owner; }
  void setOwner(Renderer* owner) { this->owner = owner; }

protected:
  Renderer* owner;

private:
  Type type;
  
};

class Render;
class Renderer
{
  public:
    Renderer() {}
    virtual ~Renderer() {}

    virtual void renderChildrens(RenderFunctionsCache& gl) {}            // Pre-render all the internals FBOs/GeometryShaders before "real" render
    virtual void glRender(RenderFunctionsCache& gl, Render& render) = 0; // You must not bind any FBO in this function 

  protected:
    bool addComponent(RendererComponent* newComponent);      // Return false if a component of the same type already exists
    bool removeComponent(RendererComponent::Type toRemove);  // Warning this is not deleting the component, return false if the component type wasn't in the table
    bool deleteComponent(RendererComponent::Type toDelete);  // return false if the component type wasn't in the table
    void deleteAllComponents(); 

  private:
    QHash<RendererComponent::Type, RendererComponent*> components;

};

#include "fast2dquad.hpp"
#include <QOpenGLShaderProgram>

class SimpleQuadWithFragmentShaderRenderer : public Renderer
{
public:
  SimpleQuadWithFragmentShaderRenderer() {}
  ~SimpleQuadWithFragmentShaderRenderer() override {}

  void glRender(RenderFunctionsCache& gl, Render& render) override
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