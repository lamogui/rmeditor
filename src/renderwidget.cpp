
#include "renderwidget.hpp" 
#include "logwidget.hpp" // FIXME : remove this !

#include <QFileDialog> // for take screen shots 
#include <QMouseEvent>  
#include <QVector3D>   // for camera control

#include "camera.hpp"
#include "render.hpp"
#include "renderer.hpp"
#include "fast2dquad.hpp"
#include "quadfragmentshader.hpp" // for vertex quad shader ! 

RenderWidget::RenderWidget(QWidget *parent) :
  QOpenGLWidget(parent),
  render(nullptr),
  quad(nullptr),
  captureMouse(false),
  onlyShowTexture(false)
{
  // Widget config
  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
  setFocusPolicy(Qt::ClickFocus);

  // Update Timer config
  connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update()));
  startUpdateLoop();

  // Open GL Context config
  QSurfaceFormat f;
  f.setRenderableType(QSurfaceFormat::OpenGL);
  f.setMajorVersion(4);
  f.setMinorVersion(5);
  f.setProfile(QSurfaceFormat::CoreProfile);
  f.setOptions(QSurfaceFormat::DebugContext | f.options());
  setFormat(f);
}

RenderWidget::~RenderWidget()
{
  if (render)
  {
    delete render;
  }
  if (quad)
  {
    delete quad;
  }
}


void RenderWidget::setLogWidget(LogWidget* log)
  {logWidget = log;}

const char* RenderWidget::getDisplayTextureFragmentShaderCode()
{
  static const char* vertexShader =
    "#version 450\n"
    "uniform sampler2D colorTexture;"
    "smooth in vec2 coords;"
    "smooth in vec2 uv;"
    "layout(location = 0) out vec4 color;"
    "void main()"
    "{"
    "color = texture(colorTexture, coords);"
    "}";
  return vertexShader;
}

//-----------------------------------------------------------------------------
// initializeGL
//-----------------------------------------------------------------------------
void RenderWidget::initializeGL()
{
  // Debugger 
  Q_ASSERT(logWidget);
  qDebug() << "Created context version: " << format().majorVersion() << "." << format().minorVersion();
  openglDebugLogger.initialize();
  connect(&openglDebugLogger, &QOpenGLDebugLogger::messageLogged, logWidget, &LogWidget::handleOpengGLLoggedMessage);
  openglDebugLogger.startLogging();

  // Render
  Q_ASSERT(!render);
  render = new RenderTexture2D(size());
  render->initializeOpenGLFunctions(); // UGLY !!!
  render->glDisable(GL_DEPTH_TEST);

  // Fast2DQuad
  Q_ASSERT(!quad);
  quad = new Fast2DQuad();
  quad->initializeOpenGLFunctions(); // UGLY !!!

  // Shader
  if (quadShader.create())
  {
    if (!quadShader.addShaderFromSourceCode(QOpenGLShader::Vertex, QuadFragmentShaderCode::getVertexShaderCode()))
      logWidget->writeError("Fatal error! could not compile vertex shader: " + quadShader.log());
    else if (!quadShader.addShaderFromSourceCode(QOpenGLShader::Fragment, getDisplayTextureFragmentShaderCode()))
      logWidget->writeError("Fatal error! could not compile fragment shader: " + quadShader.log());
    else
    {
      // link
      quadShader.bindAttributeLocation("position", VertexAttributesIndex::position);
      quadShader.bindAttributeLocation("texCoords", VertexAttributesIndex::texCoord);

      if (!quadShader.link())
        logWidget->writeError("Fatal error! could not link shader: " + quadShader.log());
    }
  }
  else
    logWidget->writeError("Fatal erreor! could not create shader program");
}

//-----------------------------------------------------------------------------
// paintGL
//-----------------------------------------------------------------------------
void RenderWidget::paintGL()
{
  render->glClear(GL_COLOR_BUFFER_BIT);
  QSharedPointer<Renderer> renderer = currentRenderer.lock();
  if (renderer)
  {
    if (!onlyShowTexture)
    {
      if (renderer->hasDynamicCamera() && !keysPressed.empty())
      {
        Q_ASSERT(renderer->getCurrentCamera());
        QVector3D delta;
        if (keysPressed.contains(Qt::Key_Up))
        {
          delta += QVector3D(0, 0, 1);
        }
        if (keysPressed.contains(Qt::Key_Down))
        {
          delta += QVector3D(0, 0, -1);
        }
        if (keysPressed.contains(Qt::Key_Left))
        {
          delta += QVector3D(-1, 0, 0);
        }
        if (keysPressed.contains(Qt::Key_Right))
        {
          delta += QVector3D(1, 0, 0);
        }
        if (keysPressed.contains(Qt::Key_PageUp))
        {
          delta += QVector3D(0, 1, 0);
        }
        if (keysPressed.contains(Qt::Key_PageDown))
        {
          delta += QVector3D(0, -1, 0);
        }
        renderer->getCurrentCamera()->translateRelative(delta*0.02f);
      }
      render->render(*renderer);
    }
  }

  makeCurrent();
  render->glViewport(0, 0, width(), height());
  quadShader.bind();
  quadShader.setUniformValue("textureColor", 0);
  render->glActiveTexture(GL_TEXTURE0);
  render->glBindTexture(GL_TEXTURE_2D, render->getFBO().texture());
  quad->draw();
  quadShader.release();
  doneCurrent();
}

void RenderWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::RightButton)
  {
    captureMouse = true;
  }
  QWidget::mousePressEvent(event);
  previousMousePos = event->screenPos();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::RightButton)
  {
    captureMouse = false;
  }
  QWidget::mouseReleaseEvent(event);
  previousMousePos = event->screenPos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (captureMouse)
  {
    float yaw = event->screenPos().x() - previousMousePos.x();
    float pitch = event->screenPos().y() - previousMousePos.y();
    QSharedPointer<Renderer> renderer = currentRenderer.lock();
    if (renderer && renderer->hasDynamicCamera())
    {
      renderer->getCurrentCamera()->rotate(yaw*0.25,pitch*0.25,0);
    }
  }
  QWidget::mouseMoveEvent(event);
  previousMousePos = event->screenPos();
}

void RenderWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
}

void RenderWidget::resetCamera()
{
  QSharedPointer<Renderer> renderer = currentRenderer.lock();
  if (renderer && renderer->hasDynamicCamera())
  {
    renderer->getCurrentCamera()->reset();
  }
}

void RenderWidget::takeScreenshot()
{
  update();
  QImage imageToSave = grabFramebuffer();
  QString filename = QFileDialog::getSaveFileName(this, "Save a screenshot", QString(), "Images (*.png *.jpg)");
  if (!filename.isEmpty())
  {
    imageToSave.save(filename);
  }
}

void RenderWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Up ||
      event->key() == Qt::Key_Down ||
      event->key() == Qt::Key_Left ||
      event->key() == Qt::Key_Right ||
      event->key() == Qt::Key_PageUp ||
      event->key() == Qt::Key_PageDown)
  {
    keysPressed.insert((Qt::Key)event->key());
  }
  else
  {
    event->ignore();
  }
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event)
{
  keysPressed.remove((Qt::Key)event->key());
}

void RenderWidget::startUpdateLoop()
{
   updateTimer.start(10);
}

void RenderWidget::stopUpdateLoop()
{
  updateTimer.stop();
}

void RenderWidget::setCurrentRenderer(const QWeakPointer<Renderer>& renderer)
{
  currentRenderer = renderer;
}

