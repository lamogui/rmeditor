
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
  renderFunctions(nullptr),
  render(nullptr),
  quad(nullptr),
  quadShader(nullptr),
  openglDebugLogger(nullptr),
  captureMouse(false),
  onlyShowTexture(false),
  textureDisplayed(color)
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

}


void RenderWidget::setLogWidget(LogWidget* log)
  {logWidget = log;}

const char* RenderWidget::getDisplayTextureFragmentShaderCode()
{
  static const char* vertexShader =
    "#version 440\n"
    "uniform sampler2D colorTexture;"
    "uniform sampler2D normalTexture;"
    "uniform sampler2D depthTexture;"
    "uniform int textureDisplayed;"
    "smooth in vec2 coords;"
    "smooth in vec2 uv;"
    "layout(location = 0) out vec4 color;"
    "void main()"
    "{"
      "if (textureDisplayed == 0) "
        "color = texture(colorTexture, coords);"
      "else if (textureDisplayed == 1) "
        "color = vec4(texture(normalTexture, coords).rgb, 1.0);"
      "else "
        "color = vec4(vec3(texture(depthTexture, coords).r), 1.0);"
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
  Q_ASSERT(!openglDebugLogger);
  openglDebugLogger = new QOpenGLDebugLogger(this);
  openglDebugLogger->initialize();
  connect(openglDebugLogger, &QOpenGLDebugLogger::messageLogged, logWidget, &LogWidget::handleOpengGLLoggedMessage);
  openglDebugLogger->startLogging();

  // Functions 
  Q_ASSERT(!renderFunctions);
  renderFunctions = new RenderFunctionsCache();
  renderFunctions->initializeOpenGLFunctions();
  renderFunctions->glDisable(GL_DEPTH_TEST);

  // Render
  Q_ASSERT(!render);
  render = new RenderTexture2D();
  render->initializeGL(*renderFunctions, size());

  // Fast2DQuad
  Q_ASSERT(!quad);
  quad = new Fast2DQuad();
  quad->initializeGL(*renderFunctions);

  // Shader
  Q_ASSERT(!quadShader);
  quadShader = new QOpenGLShaderProgram(this);
  if (quadShader->create())
  {
    if (!quadShader->addShaderFromSourceCode(QOpenGLShader::Vertex, QuadFragmentShaderCode::getVertexShaderCode()))
    {
      logWidget->writeError("Fatal error! could not compile vertex shader: " + quadShader->log());
      qDebug() << "Fatal error! could not compile vertex shader: " + quadShader->log();
    }
    else if (quadShader->log().size() > 0)
    {
      logWidget->writeWarning("Warning ! while compiling vertex shader: " + quadShader->log());
      qDebug() << "Warning ! while compiling vertex shader: " + quadShader->log();
    }

    if (!quadShader->addShaderFromSourceCode(QOpenGLShader::Fragment, getDisplayTextureFragmentShaderCode()))
    {
      logWidget->writeError("Fatal error! could not compile fragment shader: " + quadShader->log());
      qDebug() << "Fatal error! could not compile fragment shader: " + quadShader->log();
    }
    else if (quadShader->log().size() > 0)
    {
      logWidget->writeWarning("Warning ! while compiling fragment shader: " + quadShader->log());
      qDebug() << "Warning ! while compiling fragment shader: " + quadShader->log();
    }
    {
      // link
      quadShader->bindAttributeLocation("position", VertexAttributesIndex::position);
      quadShader->bindAttributeLocation("texCoords", VertexAttributesIndex::texCoord);

      if (!quadShader->link())
      {
        logWidget->writeError("Fatal error! could not link shader: " + quadShader->log());
        qDebug() << "Fatal error! could not link shader: " + quadShader->log();
      }
      else if (quadShader->log().size() > 0)
      {
        logWidget->writeWarning("Warning ! while linking shader: " + quadShader->log());
        qDebug() << "Warning ! while linking shader: " + quadShader->log();
      }

      quadShader->bind();
      quadShader->setUniformValue("textureDisplayed", (int)textureDisplayed);
    }
  }
  else
    logWidget->writeError("Fatal error! could not create shader program");

  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &RenderWidget::cleanup);
}

//-----------------------------------------------------------------------------
// paintGL
//-----------------------------------------------------------------------------
void RenderWidget::paintGL()
{
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
      render->render(*renderFunctions, *renderer);
    }
  }

  static int k = 0;
  renderFunctions->glViewport(0, 0, width(), height());
  renderFunctions->glClearColor(0, 0, 0, 0);
  renderFunctions->glClear(GL_COLOR_BUFFER_BIT);
  quadShader->bind();
  QVector<uint> textures = render->getFBO().textures();
  if (textures.size() > 0)
  {
    quadShader->setUniformValue("colorTexture", 0);
    renderFunctions->glActiveTexture(GL_TEXTURE0);
    renderFunctions->glBindTexture(GL_TEXTURE_2D, textures[0]);
  }
  if (textures.size() > 1)
  {
    quadShader->setUniformValue("normalTexture", 1);
    renderFunctions->glActiveTexture(GL_TEXTURE1);
    renderFunctions->glBindTexture(GL_TEXTURE_2D, textures[1]);
  }
  if (textures.size() > 2)
  {
    quadShader->setUniformValue("depthTexture", 2);
    renderFunctions->glActiveTexture(GL_TEXTURE2);
    renderFunctions->glBindTexture(GL_TEXTURE_2D, textures[2]);
  }
  quad->draw(*renderFunctions);
  quadShader->release();
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

void RenderWidget::switchDisplayedTexture()
{
  if (textureDisplayed < 2)
    textureDisplayed = (TextureDisplayed)((int)textureDisplayed + 1);
  else
    textureDisplayed = TextureDisplayed::color;
  if (quadShader)
  {
    quadShader->bind();
    quadShader->setUniformValue("textureDisplayed", (int)textureDisplayed);
  }
}

void RenderWidget::cleanup()
{
  if (render)
  {
    delete render;
    render = nullptr;
  }
  if (quad)
  {
    delete quad;
    quad = nullptr;
  }
  if (quadShader)
  {
    delete quadShader;
    quadShader = nullptr;
  }

  Q_ASSERT(renderFunctions);
  if (renderFunctions)
  {
    delete renderFunctions;
    renderFunctions = nullptr;
  }

  Q_ASSERT(openglDebugLogger);
  if (openglDebugLogger)
  {
    delete openglDebugLogger;
    openglDebugLogger = nullptr;
  }
}

