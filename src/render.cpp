
#include "render.hpp"
#include "renderer.hpp"
#include <QOpenGLFunctions>

#include "logmanager.hpp"
#include "jassert.hpp"

/*
** Render
*/
Render::Render(Type type):
  type(type)
{
}

Render::~Render()
{
}

void Render::initializeGL(RenderFunctionsCache& renderFunctions, const QSize& initialFBOSize)
{
  //QOpenGLFunctions functions;
  //functions.initializeOpenGLFunctions();
  //jassert(functions.hasOpenGLFeature(QOpenGLFunctions::MultipleRenderTargets));
  fbo.reset(new QOpenGLFramebufferObject(initialFBOSize, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGBA8));
  createAttachements(initialFBOSize);
}

void Render::render(RenderFunctionsCache& renderFunctions, Renderer& renderer)
{
  renderer.renderChildrens(renderFunctions);
  fbo->bind();
  configureDrawedBuffer(renderFunctions);
  //renderFunctions.glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
  //renderFunctions.glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
  renderFunctions.glClearColor(0, 0, 0, 1);
  renderFunctions.glClear(GL_COLOR_BUFFER_BIT);
  renderFunctions.glViewport(0, 0, (GLsizei)fbo->width(), (GLsizei)fbo->height());
  renderer.glRender(renderFunctions, *this);
  fbo->release();
}

void Render::resize(const QSize& newSize)
{
  if (newSize != fbo->size())
    resizeFBO(newSize);
}

void Render::resizeFBO(const QSize& newSize)
{
  fbo.reset(new QOpenGLFramebufferObject(newSize, QOpenGLFramebufferObject::Depth, GL_TEXTURE_2D, GL_RGBA8));
  createAttachements(newSize);
}

/*
** RenderTexture2D
*/

RenderTexture2D::RenderTexture2D() :
  Render(Texture2D)
{

}

void RenderTexture2D::createAttachements(const QSize& fboSize)
{
  // fbo->addColorAttachment(fboSize, GL_RGBA8);   // color (always created by default by qt)
  fbo->addColorAttachment(fboSize, GL_RGB8);       // normal
  fbo->addColorAttachment(fboSize, GL_RED);        // depth
}

void RenderTexture2D::configureDrawedBuffer(RenderFunctionsCache& renderFunctions)
{
  GLenum drawnBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  renderFunctions.glDrawBuffers(3, drawnBuffers);
}

QImage RenderTexture2D::getImage()
{
  jassert(fbo);
  return fbo->toImage(true, 0);
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