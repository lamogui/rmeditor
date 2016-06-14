#include "renderer.hpp"
#include "fast2dquad.hpp"
#include "shader.hpp"

#include <QDebug>

Renderer::Renderer(size_t w, size_t h, QObject *parent):
  QObject(parent),
  QOpenGLFunctions(QOpenGLContext::currentContext()),
  m_fbo(),
  m_camera(NULL)
{
  m_fbo.setSize(w,h);
}

Renderer::~Renderer()
{

}


void Renderer::resize(size_t width, size_t height)
{
  m_fbo.setSize(width,height);
  customResize(width,height);
}

void Renderer::glRender(size_t width, size_t height)
{
  if (width != m_fbo.getSizeX() || height != m_fbo.getSizeY())
  {
    resize(width,height);
  }
  glRender();
}

QImage Renderer::getImage()
{
  m_fbo.enable();
  QImage img = m_fbo.getImage();
  m_fbo.disable();
  return img;
}

#include "scene.hpp"

SceneRenderer::SceneRenderer(Scene &scene, size_t w, size_t h, QObject* parent):
  Renderer(w,h,parent),
  m_scene(&scene)
{

}

void SceneRenderer::glRender()
{
  m_fbo.enable();

  m_scene->getShader().enable();

  if (m_camera)
  {
    m_scene->getShader().sendf("cam_position",m_camera->position().x(),m_camera->position().y(), m_camera->position().z());
    m_scene->getShader().sendf("cam_rotation",m_camera->rotation().x(),m_camera->rotation().y(), m_camera->rotation().z(), m_camera->rotation().scalar());
  }
  else
  {
    m_scene->getShader().sendf("cam_rotation",0.f,0.f,0.f,1.f);
  }

  m_scene->getShader().sendf("xy_scale_factor",(float)m_fbo.getSizeX()/(float)m_fbo.getSizeY());
  m_scene->getShader().sendf("sequence_time",(float)(m_sequenceTime.elapsed())/1000.f);
  m_scene->getShader().sendf("track_time",(float)(m_sequenceTime.elapsed())/1000.f);
  Fast2DQuadDraw();
  m_scene->getShader().disable();

  m_fbo.disable();
}

void SceneRenderer::attachedWidget(RenderWidget *widget)
{
  if (widget)
  {
    m_sequenceTime.restart();
  }
  else
  {

  }
}


