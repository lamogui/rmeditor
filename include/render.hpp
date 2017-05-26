#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QTime>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLFramebufferObject>

class RenderWidget;
class Camera;

class Render : public QObject, public QOpenGLFunctions_4_5_Core // FIXME : do no inherit from this 
{
  Q_OBJECT

public:
  Render(const QSize& initialSize, QObject* parent = nullptr);
  ~Render() override;

  // The function ! 
  void render();

  // Accessors
  const QOpenGLFramebufferObject& getFBO() const { return *fbo; }
  QWeakPointer<Camera>& getCamera() { return camera; }
  const QWeakPointer<Camera>& getCamera() const { return camera; }

  // To override
  virtual void resize(const QSize& newSize);
  virtual void attachedWidget(RenderWidget* widget) { (void)widget; } //Called by RenderWidget, widget is nullptr when detached

protected:
  // To override
  virtual void glRender() = 0;
  virtual void createAttachements(const QSize& fboSize);  // note that the default RGBA color attachement is always created !
  inline virtual void renderChildrens() {} // call here the render function of your childrens 

  // utils
  void resizeFBO(const QSize& newSize);

  QOpenGLFramebufferObject* fbo;
  QWeakPointer<Camera> camera;
};

#include "fast2dquad.hpp"

class Scene;
class SceneRender : public Render
{
public:
  SceneRender(Scene& scene, const QSize& initialSize, QObject *parent=nullptr);
  
  // Render 
  void attachedWidget(RenderWidget* widget) override;

protected:
  // Render
  void glRender() override;

  Scene* scene;
  QTime sequenceTime;
  Fast2DQuad quad;
};

#endif // !RENDERER_HPP
