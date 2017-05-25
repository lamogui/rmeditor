#include "renderer.hpp"
#include "fast2dquad.hpp"
#include "shader.hpp"

#include <QDebug>

Renderer::Renderer(size_t w, size_t h, QObject *parent):
  QObject(parent),
  QOpenGLFunctions(QOpenGLContext::currentContext()),
  fbo(),
  camera(nullptr)
{
  fbo.setSize(w,h);
}

Renderer::~Renderer()
{

}


void Renderer::resize(size_t width, size_t height)
{
  fbo.setSize(width,height);
  customResize(width,height);
}

void Renderer::glRender(size_t width, size_t height)
{
  if (width != fbo.getSizeX() || height != fbo.getSizeY())
  {
    resize(width,height);
  }
  glRender();
}

QImage Renderer::getImage()
{
  fbo.enable();
  QImage img = fbo.getImage();
  fbo.disable();
  return img;
}

#include "scene.hpp"

SceneRenderer::SceneRenderer(Scene &scene, size_t w, size_t h, QObject* parent):
  Renderer(w,h,parent),
  scene(&scene)
{

}

void SceneRenderer::glRender()
{
  fbo.enable();

  scene->getShader().enable();

  if (camera)
  {
    scene->getShader().sendf("cam_position",camera->getPosition().x(),camera->getPosition().y(), camera->getPosition().z());
    scene->getShader().sendf("cam_rotation",camera->getRotation().x(),camera->getRotation().y(), camera->getRotation().z(), camera->getRotation().scalar());
  }
  else
  {
    scene->getShader().sendf("cam_rotation",0.f,0.f,0.f,1.f);
  }

  scene->getShader().sendf("xy_scale_factor",(float)fbo.getSizeX()/(float)fbo.getSizeY());
  scene->getShader().sendf("sequence_time",(float)(sequenceTime.elapsed())/1000.f);
  scene->getShader().sendf("track_time",(float)(sequenceTime.elapsed())/1000.f);
  Fast2DQuadDraw();
  scene->getShader().disable();

  fbo.disable();
}

void SceneRenderer::attachedWidget(RenderWidget *widget)
{
  if (widget)
  {
    sequenceTime.restart();
  }
  else
  {

  }
}


