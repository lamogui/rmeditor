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

  inline GLuint getColorTexture() const { return m_fbo.getColor(); }
  inline void setCamera(Camera* c) { m_camera = c; }
  inline Camera* camera() const  { return m_camera; }

  //Called by RenderWidget, widget is nullptr when detached
  virtual void attachedWidget(RenderWidget* widget) { (void) widget; }


  inline size_t width() const { return m_fbo.width(); }
  inline size_t height() const { return m_fbo.height(); }


  QImage getImage();

protected:
  virtual void customResize(size_t width, size_t height) { (void) width; (void) height; }
  virtual void glRender() = 0;


  FBO m_fbo;
  Camera* m_camera;
};

class Scene;

class SceneRenderer : public Renderer
{
public:
  SceneRenderer(Scene& scene, size_t w, size_t h, QObject *parent=nullptr);
  virtual void attachedWidget(RenderWidget* widget);

protected:
  virtual void glRender();

  Scene* m_scene;
  QTime m_sequenceTime;
};

#endif
