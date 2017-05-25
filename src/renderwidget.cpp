
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
#include "fast2dquad.hpp"
#include "renderer.hpp"
#include "fbo.hpp"


RenderWidget::RenderWidget(QWidget *parent) :
  QOpenGLWidget(parent),
  renderer(nullptr),
  captureMouse(false),
  onlyShowTexture(false)
{
  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
  connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update()));
  startUpdateLoop();
  resetCamera();
  setFocusPolicy(Qt::ClickFocus);

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
  if (renderer)
  {
    renderer->attachedWidget(nullptr);
  }

  Fast2DQuadFree();
}


void RenderWidget::setLogWidget(LogWidget* log)
  {logWidget = log;}

//-----------------------------------------------------------------------------
// initializeGL
//-----------------------------------------------------------------------------
void RenderWidget::initializeGL()
{
  Q_ASSERT(logWidget);
  qDebug() << "Created context version: " << format().majorVersion() << "." << format().minorVersion();
  openglDebugLogger.initialize();
  connect(&openglDebugLogger, &QOpenGLDebugLogger::messageLogged, logWidget, &LogWidget::handleOpengGLLoggedMessage);
  openglDebugLogger.startLogging();


    Fast2DQuadInit();
    //glMatrixMode(GL_PROJECTION);
    //gluPerspective(90., 16./9., 0.01, 10000.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------
// paintGL
//-----------------------------------------------------------------------------
void RenderWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (renderer)
    {
      if (!onlyShowTexture)
      {
        if (renderer->getCamera() && !keysPressed.empty())
        {
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
          renderer->getCamera()->translateRelative(delta*0.02f);
        }
        renderer->glRender(this->width(),this->height());
      }

      QOpenGLFunctions gl(QOpenGLContext::currentContext());

      glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();


      gl.glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, renderer->getColorTexture());
      Fast2DQuadDraw();



      glMatrixMode(GL_PROJECTION);
        glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    //glFlush();
    //glFinish();
}

//-----------------------------------------------------------------------------
// resizeGL
//-----------------------------------------------------------------------------
void RenderWidget::resizeGL(int width, int height)
{
    glViewport(0,0,width,height);
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
    if (renderer && renderer->getCamera())
    {
      renderer->getCamera()->rotate(yaw*0.25,pitch*0.25,0);
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
  if (renderer && renderer->getCamera())
  {
    renderer->getCamera()->reset();
  }
}


void RenderWidget::takeScreenshot()
{
  update();
  QImage imageToSave = this->grabFramebuffer();
  QString filename = QFileDialog::getSaveFileName(this, "Save a screenshot", QString(), "Images (*.png *.jpg)");
  if (!filename.isEmpty())
  {
    imageToSave.save(filename);
  }
}

void RenderWidget::setRender(Render *renderer)
{
 /* if (!renderer)
  {
    return;
  }
*/
  if (this->renderer)
  {
    disconnect(this->renderer,SIGNAL(destroyed(QObject*)),this,SLOT(onRenderDestroy()));
    this->renderer->attachedWidget(nullptr);
  }
  this->renderer = renderer;
  if (renderer)
  {
    connect(renderer,SIGNAL(destroyed(QObject*)),this,SLOT(onRenderDestroy()),Qt::DirectConnection);
    renderer->attachedWidget(this);
  }
}

void RenderWidget::onRenderDestroy()
{
  renderer = nullptr;
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

Camera* RenderWidget::camera() const
{
  if (renderer)
  {
    return renderer->getCamera();
  }
  return nullptr;
}

void RenderWidget::startUpdateLoop()
{
   updateTimer.start(10);
}

void RenderWidget::stopUpdateLoop()
{
  updateTimer.stop();
}
