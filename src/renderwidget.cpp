
#include "renderwidget.hpp"
#include "logwidget.hpp"
#include <iostream>

#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QDir>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QVector3D>

#include <GL/gl.h>
#include <GL/glu.h>

#include "camera.hpp"
#include "render.hpp"
#include "renderer.hpp"
#include "fast2dquad.hpp"

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

  // Fast2DQuad
  Q_ASSERT(!quad);
  quad = new Fast2DQuad();
  quad->initializeOpenGLFunctions(); // UGLY !!!
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
            delta += QVector3D(0,0,1);
          }
          if (keysPressed.contains(Qt::Key_Down))
          {
            delta += QVector3D(0,0,-1);
          }
          if (keysPressed.contains(Qt::Key_Left))
          {
            delta += QVector3D(-1,0,0);
          }
          if (keysPressed.contains(Qt::Key_Right))
          {
            delta += QVector3D(1,0,0);
          }
          if (keysPressed.contains(Qt::Key_PageUp))
          {
            delta += QVector3D(0,1,0);
          }
          if (keysPressed.contains(Qt::Key_PageDown))
          {
            delta += QVector3D(0,-1,0);
          }
          renderer->getCurrentCamera()->translateRelative(delta*0.02f);
        }
        render->render(*renderer);
      }

      makeCurrent();
      render->glViewport(0, 0, width(), height());
      render->glActiveTexture(GL_TEXTURE0);
      render->glBindTexture(GL_TEXTURE_2D, render->getFBO().texture());
      quad->draw();
      doneCurrent();
  }
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

