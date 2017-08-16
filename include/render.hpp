#ifndef RMEDITOR_RENDER_HPP
#define RMEDITOR_RENDER_HPP

#include "renderfunctionscache.hpp"
#include <QOpenGLFramebufferObject>

class Renderer;
/*
** Render : a render represent the final layer / picture which the renderer draw into
*/
class Render
{
public:

  enum Type
  {
    Texture2D,
    Cubemap // One day 
  };

  Render(Type type);
  virtual ~Render();

  void initializeGL(RenderFunctionsCache& renderFunctions, const QSize& initialFBOSize);

  // Accessors
  const QOpenGLFramebufferObject& getFBO() const { return *fbo; } // Ugly !
  Type getType() const { return type; }
  void resize(const QSize& newSize);

  // The function ! 
  void render(RenderFunctionsCache& renderFunctions, Renderer& renderer);

protected:
  // utils
  void resizeFBO(const QSize& newSize);

  // to override
  virtual void createAttachements(const QSize& fboSize) = 0; // note that the default RGBA color attachement is always created !

  QSharedPointer<QOpenGLFramebufferObject> fbo;

private:
  Type type;
};

class RenderTexture2D : public Render
{
public:
  RenderTexture2D();

protected:
  void createAttachements(const QSize& fboSize) final;

};

#endif // !RMEDITOR_RENDER_HPP
