
#include "renderwidget.hpp" 

#include <QFileDialog> // for take screen shots 
#include <QMouseEvent>  
#include <QVector3D>   // for camera control

#include "logwidget.hpp"
#include "jassert.hpp"

#include "camera.hpp"
#include "render.hpp"
#include "renderer.hpp"
#include "fast2dquad.hpp"
#include "quadfragmentshader.hpp" // for vertex quad shader ! 

RenderWidget::RenderWidget(QWidget *_parent) :
	QOpenGLWidget(_parent),
	m_textureDisplayed(color),
	m_renderFunctions(nullptr),
	m_render(nullptr),
	m_quad(nullptr),
	m_quadShader(nullptr),
	m_openglDebugLogger(nullptr),
	m_captureMouse(false)
{
	// Widget config
	setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
	setFocusPolicy(Qt::ClickFocus);

	// Update Timer config
	connect(&m_updateTimer, &QTimer::timeout, this, &RenderWidget::update);
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
{;
  // Debugger
  Log::Info("Created context version: " + QString::number(format().majorVersion()) + "." + QString::number(format().minorVersion()));
  jassert(!openglDebugLogger);
  openglDebugLogger = new QOpenGLDebugLogger(this);
  openglDebugLogger->initialize();
  connect(openglDebugLogger, &QOpenGLDebugLogger::messageLogged, LogManager::get(), &LogManager::handleOpengGLLoggedMessage);
  openglDebugLogger->startLogging();

  // Functions 
  jassert(!renderFunctions);
  renderFunctions = new RenderFunctionsCache();
  renderFunctions->initializeOpenGLFunctions();
  renderFunctions->glDisable(GL_DEPTH_TEST);

  // Render
  jassert(!render);
  render = new RenderTexture2D();
  render->initializeGL(*renderFunctions, size());

  // Fast2DQuad
  jassert(!quad);
  quad = new Fast2DQuad();
  quad->initializeGL(*renderFunctions);

  // Shader
  jassert(!quadShader);
  quadShader = new QOpenGLShaderProgram(this);
  if (quadShader->create())
  {
    if (!quadShader->addShaderFromSourceCode(QOpenGLShader::Vertex, QuadFragmentShaderCode::getVertexShaderCode()))
    {
      Log::Error("could not compile QuadFragmentShaderCode vertex shader: " + quadShader->log());
    }
    else if (quadShader->log().size() > 0)
    {
      Log::Warning("while compiling QuadFragmentShaderCode vertex shader: " + quadShader->log());
    }

    if (!quadShader->addShaderFromSourceCode(QOpenGLShader::Fragment, getDisplayTextureFragmentShaderCode()))
    {
      Log::Error("could not compile DisplayTextureFragmentShaderCode fragment shader: " + quadShader->log());
    }
    else if (quadShader->log().size() > 0)
    {
      Log::Warning("while compiling DisplayTextureFragmentShaderCode fragment shader: " + quadShader->log());
    }
    {
      // link
      quadShader->bindAttributeLocation("position", VertexAttributesIndex::position);
      quadShader->bindAttributeLocation("texCoords", VertexAttributesIndex::texCoord);

      if (!quadShader->link())
      {
        Log::Error("could not link QuadFragmentShaderCode with DisplayTextureFragmentShaderCode: " + quadShader->log());
      }
      else if (quadShader->log().size() > 0)
      {
        Log::Warning("Warning ! while linking QuadFragmentShaderCode with DisplayTextureFragmentShaderCode: " + quadShader->log());
      }

      quadShader->bind();
      quadShader->setUniformValue("textureDisplayed", (int)textureDisplayed);
    }
  }
  else
    Log::Error("could not create QuadFragmentShaderCode/DisplayTextureFragmentShaderCode shader program");

  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &RenderWidget::cleanup);
}

//-----------------------------------------------------------------------------
// paintGL
//-----------------------------------------------------------------------------
void RenderWidget::paintGL()
{
  QSharedPointer<Renderer> renderer = currentRenderer.lock();
    if (m_renderer)    {
      if (renderer->hasDynamicCamera() && !keysPressed.empty())
      {
        jassert(renderer->getCurrentCamera());
        QVector3D delta;
        if (keysPressed.contains(Qt::Key_Up))
        {
          delta += QVector3D(0, 0, 1);
          float fov_delta = 0.0f;
          if (m_keysPressed.contains(Qt::Key_Up) || m_keysPressed.contains(Qt::Key_Z))
          {
            delta += QVector3D(0,0,1);
          }
          if (m_keysPressed.contains(Qt::Key_Down) || m_keysPressed.contains(Qt::Key_S))
          {
            delta += QVector3D(0,0,-1);
          }
          if (m_keysPressed.contains(Qt::Key_Left) || m_keysPressed.contains(Qt::Key_Q))
          {
            delta += QVector3D(-1,0,0);
          }
          if (m_keysPressed.contains(Qt::Key_Right)|| m_keysPressed.contains(Qt::Key_D))
          {
            delta += QVector3D(1,0,0);
          }
          if (m_keysPressed.contains(Qt::Key_PageUp) || m_keysPressed.contains(Qt::Key_Plus))
          {
            delta += QVector3D(0,1,0);
          }
          if (m_keysPressed.contains(Qt::Key_PageDown) || m_keysPressed.contains(Qt::Key_Minus) )
          {
            delta += QVector3D(0,-1,0);
          }
          if (m_keysPressed.contains(Qt::Key_C)  )
          {
            fov_delta += 5.0f;
          }
          if (m_keysPressed.contains(Qt::Key_V)  )
          {
            fov_delta -= 5.0f;
          }
          m_renderer->camera()->translateRelative(delta*0.02f);
          m_renderer->camera()->setFov(qMax(qMin(180.0f,  m_renderer->camera()->fov() + fov_delta),30.0f));
        }
        m_renderer->glRender(static_cast<size_t>(this->width()),static_cast<size_t>(this->height()));
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
    m_captureMouse = true;
  }
  QWidget::mousePressEvent(event);
  m_previousMousePos = event->screenPos();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::RightButton)
  {
    m_captureMouse = false;
  }
  QWidget::mouseReleaseEvent(event);
  m_previousMousePos = event->screenPos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (m_captureMouse)
  {
    float yaw = event->screenPos().x() - m_previousMousePos.x();
    float pitch = event->screenPos().y() - m_previousMousePos.y();
    QSharedPointer<Renderer> renderer = currentRenderer.lock();
    if (renderer && renderer->hasDynamicCamera())
    {
      renderer->getCurrentCamera()->rotate(yaw*0.25,pitch*0.25,0);
    }
  }
  QWidget::mouseMoveEvent(event);
  m_previousMousePos = event->screenPos();
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
      event->key() == Qt::Key_PageDown ||
      event->key() == Qt::Key_Plus ||
          event->key() == Qt::Key_Minus ||
          event->key() == Qt::Key_Z ||
          event->key() == Qt::Key_Q ||
          event->key() == Qt::Key_S ||
          event->key() == Qt::Key_D ||
          event->key() == Qt::Key_C ||
          event->key() == Qt::Key_V)
  {
    m_keysPressed.insert((Qt::Key)event->key());
  }
  else
  {
    event->ignore();
  }
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event)
{
  m_keysPressed.remove((Qt::Key)event->key());
}

void RenderWidget::startUpdateLoop()
{
   m_updateTimer.start(10);
}

void RenderWidget::stopUpdateLoop()
{
  m_updateTimer.stop();
}

void RenderWidget::switchDisplayedTexture()
{
	if (m_textureDisplayed < 2) {
		m_textureDisplayed = static_cast<TextureDisplayed>(static_cast<int>(m_textureDisplayed) + 1);
	}
	else {
		m_textureDisplayed = TextureDisplayed::color;
	}
	if (m_quadShader)
	{
		m_quadShader->bind();
		m_quadShader->setUniformValue("textureDisplayed", (int)m_textureDisplayed);
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

  jassert(renderFunctions);
  if (renderFunctions)
  {
    delete renderFunctions;
    renderFunctions = nullptr;
  }

  jassert(openglDebugLogger);
  if (openglDebugLogger)
  {
    delete openglDebugLogger;
    openglDebugLogger = nullptr;
  }
}

