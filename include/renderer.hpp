#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions>
#include <QTime>

#include "fbo.hpp"

class RenderWidget;
class Camera;


class Renderer : public QObject, public QOpenGLFunctions
{
  Q_OBJECT

public:
  Renderer(size_t w, size_t h, QObject* parent=nullptr);
  virtual ~Renderer();


  virtual void resize(size_t width, size_t height);
  void glRender(size_t width, size_t height);

  inline GLuint getColorTexture() const { return fbo.getColor(); }
  inline void setCamera(Camera* c) { camera = c; }
  inline Camera* getCamera() const  { return camera; }

  //Called by RenderWidget, widget is nullptr when detached
  virtual void attachedWidget(RenderWidget* widget) { (void) widget; }


  inline size_t width() const { return fbo.getWidth(); }
  inline size_t height() const { return fbo.getHeight(); }


  QImage getImage();

protected:
  virtual void customResize(size_t width, size_t height) { (void) width; (void) height; }
  virtual void glRender() = 0;


  FBO fbo;
  Camera* camera;
};

class Scene;

class SceneRenderer : public Renderer
{
public:
  SceneRenderer(Scene& scene, size_t w, size_t h, QObject *parent=nullptr);
  
  // Renderer 
  void attachedWidget(RenderWidget* widget) override;

protected:
  // Renderer
  void glRender() override;

  Scene* scene;
  QTime sequenceTime;
};

#endif // !RENDERER_HPP
