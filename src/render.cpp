
#include "render.hpp"
#include "renderer.hpp"
#include <QDebug>

/*
** Render
*/
Render::Render(const QSize& initialSize, Type type):
  fbo(new QOpenGLFramebufferObject(initialSize, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGBA8)),
  type(type)
{
  createAttachements(initialSize);
}

Render::~Render()
{
  delete fbo;
}

void Render::render(Renderer& renderer)
{
  renderer.renderChildrens(*this);
  fbo->bind();
  this->glViewport(0, 0, (GLsizei)fbo->width(), (GLsizei)fbo->height());
  renderer.glRender(*this, *this);
  fbo->release();
}

void Render::resize(const QSize& newSize)
{
  if (newSize != fbo->size())
    resizeFBO(newSize);
}

void Render::resizeFBO(const QSize& newSize)
{
  delete fbo;
  fbo = new QOpenGLFramebufferObject(newSize, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGBA8);
  createAttachements(newSize);
}

/*
** RenderTexture2D
*/

RenderTexture2D::RenderTexture2D(const QSize& initialSize) :
  Render(initialSize, Texture2D)
{

}

void RenderTexture2D::createAttachements(const QSize& fboSize)
{
  // fbo->addColorAttachment(fboSize, GL_RGBA8);           // color (always created by default)
  fbo->addColorAttachment(fboSize, GL_RGB8);               // normal
  fbo->addColorAttachment(fboSize, GL_DEPTH_COMPONENT32F); // depth
}

/*
SceneRender::SceneRender(Scene &scene, const QSize& initialSize, QObject* parent):
  Render(initialSize, parent),
  scene(&scene)
{

}

void SceneRender::glRender()
{
  scene->getShader().enable();

  if (camera)
  {
    QSharedPointer<Camera> cam = camera.lock();
    scene->getShader().sendf("cam_position", cam->getPosition().x(), cam->getPosition().y(), cam->getPosition().z());
    scene->getShader().sendf("cam_rotation", cam->getRotation().x(), cam->getRotation().y(), cam->getRotation().z(), cam->getRotation().scalar());
  }
  else
  {
    scene->getShader().sendf("cam_rotation",0.f,0.f,0.f,1.f);
  }

  scene->getShader().sendf("xy_scale_factor",(float)fbo->width()/(float)fbo->height());
  scene->getShader().sendf("sequence_time",(float)(sequenceTime.elapsed())/1000.f);
  scene->getShader().sendf("track_time",(float)(sequenceTime.elapsed())/1000.f);
  
  quad.draw();

  scene->getShader().disable();
}

void SceneRender::attachedWidget(RenderWidget *widget)
{
  if (widget)
  {
    sequenceTime.restart();
  }
  else
  {

  }
}
*/