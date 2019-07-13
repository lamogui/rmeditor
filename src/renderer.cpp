#include "renderer.hpp"
#include "fast2dquad.hpp"
#include "shader.hpp"

#include <QDebug>
#include <QtMath>

Renderer::Renderer(size_t w, size_t h, QObject *parent):
  QObject(parent),
  QOpenGLFunctions(QOpenGLContext::currentContext()),
  m_fbo(),
  m_camera(nullptr)
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

#include "music.hpp"
#include "project.hpp"
#include "scene.hpp"

SceneRenderer::SceneRenderer(Project& project, Scene &scene, size_t w, size_t h, QObject* parent):
  Renderer(w,h,parent),
  m_scene(scene),
  m_project(project)
{

}

void SceneRenderer::glRenderScene(Renderer& renderer, Project& project, Scene& scene, const Camera* camera, float track_time, float sequence_time)
{
    scene.getShader().enable();

    if (project.music()) {
        project.music()->updateTextures();
    }

    int i = 0;
    for (auto item = project.textures().begin(); item != project.textures().end(); ++item)
    {
       renderer.glActiveTexture(GL_TEXTURE0 + i);
       item.value()->bind();
       scene.getShader().sendi(item.key().toStdString().c_str(), i);
       ++i;
    }

    if (camera)
    {
      scene.getShader().sendf("cam_position",camera->position().x(),camera->position().y(), camera->position().z());
      scene.getShader().sendf("cam_rotation",camera->rotation().x(),camera->rotation().y(), camera->rotation().z(), camera->rotation().scalar());
      scene.getShader().sendf("cam_fov", qDegreesToRadians(camera->fov()));
    }
    else
    {
      scene.getShader().sendf("cam_rotation",0.f,0.f,0.f,1.f);
      scene.getShader().sendf("cam_fov", qDegreesToRadians(60.f));
    }

    scene.getShader().sendf("xy_scale_factor",(float)renderer.m_fbo.getSizeX()/(float)renderer.m_fbo.getSizeY());
    scene.getShader().sendf("sequence_time", sequence_time);
    scene.getShader().sendf("track_time", track_time);
    Fast2DQuadDraw();

    /*i = 0;
    for (auto item = project.textures().begin(); item != project.textures().end(); ++item)
    {
       renderer.glActiveTexture(GL_TEXTURE0+i);
       renderer.glBindTexture(GL_TEXTURE_2D, 0);
       ++i;
    }*/

    scene.getShader().disable();
}

void SceneRenderer::glRender()
{
  m_fbo.enable();

  if (m_project.music()) {
    SceneRenderer::glRenderScene(*this, m_project, m_scene, m_camera, m_project.music()->getTime(), m_sequenceTime.elapsed() / 1000.f);
  }
  else {
    SceneRenderer::glRenderScene(*this, m_project, m_scene, m_camera, static_cast<float>(m_sequenceTime.elapsed()) /1000.f, static_cast<float>(m_sequenceTime.elapsed()) / 1000.f);
  }
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


