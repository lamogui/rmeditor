#ifndef RMEDITOR_RENDER_HPP
#define RMEDITOR_RENDER_HPP

#include "renderfunctionscache.hpp"
#include <QOpenGLFramebufferObject>

class Renderer;
class Render : public RenderFunctionsCache // FIXME : do not inherit from this 
{
public:

  enum Type
  {
    Texture2D,
    Cubemap // One day 
  };

  Render(const QSize& initialSize, Type type);
  virtual ~Render();

  // Accessors
  const QOpenGLFramebufferObject& getFBO() const { return *fbo; }
  Type getType() const { return type; }
  void resize(const QSize& newSize);

  // The function ! 
  void render(Renderer& renderer);

protected:
  // utils
  void resizeFBO(const QSize& newSize);

  // to override
  virtual void createAttachements(const QSize& fboSize) = 0; // note that the default RGBA color attachement is always created !

  QOpenGLFramebufferObject* fbo;

private:
  Type type;
};

class RenderTexture2D : public Render
{
public:
  RenderTexture2D(const QSize& initialSize);

protected:
  void createAttachements(const QSize& fboSize) final;

};

#endif // !RMEDITOR_RENDER_HPP
