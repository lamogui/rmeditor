
#include "renderwidget.hpp"
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
  QGLWidget(parent),
  m_renderer(nullptr),
  m_captureMouse(false),
  m_onlyShowTexture(false)
{
    QGLFormat myFormat = format();
    //myFormat.setVersion(4,0);
    myFormat.setOverlay(true);
    setFormat(myFormat);
    /*bool hasOverlay =  format().hasOpenGLOverlays();
    int minv = format().minorVersion();
    int majv = format().majorVersion();*/


    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
    startUpdateLoop();



    resetCamera();

    setFocusPolicy(Qt::ClickFocus);
}

RenderWidget::~RenderWidget()
{
  if (m_renderer)
  {
    m_renderer->attachedWidget(nullptr);
  }

    Fast2DQuadFree();
}


//-----------------------------------------------------------------------------
// initializeGL
//-----------------------------------------------------------------------------
void RenderWidget::initializeGL()
{
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

    if (m_renderer)
    {
      if (!m_onlyShowTexture)
      {
        if (m_renderer->camera() && !m_keysPressed.empty())
        {
          QVector3D delta;
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

      QOpenGLFunctions gl(QOpenGLContext::currentContext());

      glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();


      gl.glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_renderer->getColorTexture());
      Fast2DQuadDraw();



      glMatrixMode(GL_PROJECTION);
        glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    glFlush();
    glFinish();
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
    if (m_renderer && m_renderer->camera())
    {
      m_renderer->camera()->rotate(yaw*0.25,pitch*0.25,0);
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
  if (m_renderer && m_renderer->camera())
  {
    m_renderer->camera()->reset();
  }
}


void RenderWidget::takeScreenshot()
{
    emit updateGL();
    QImage imageToSave = grabFrameBuffer(false);
    QString filename = QFileDialog::getSaveFileName(this, "Save a screenshot", QString(), "Images (*.png *.jpg)");
    if (!filename.isEmpty())
    {
      imageToSave.save(filename);
    }
}

void RenderWidget::setRenderer(Renderer *renderer)
{
 /* if (!renderer)
  {
    return;
  }
*/
  if (m_renderer)
  {
    disconnect(m_renderer,SIGNAL(destroyed(QObject*)),this,SLOT(onRendererDestroy()));
    m_renderer->attachedWidget(nullptr);
  }
  m_renderer = renderer;
  if (m_renderer)
  {
    connect(m_renderer,SIGNAL(destroyed(QObject*)),this,SLOT(onRendererDestroy()),Qt::DirectConnection);
    m_renderer->attachedWidget(this);
  }
}

void RenderWidget::onRendererDestroy()
{
  m_renderer = nullptr;
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

Camera* RenderWidget::camera() const
{
  if (m_renderer)
  {
    return m_renderer->camera();
  }
  return nullptr;
}

void RenderWidget::startUpdateLoop()
{
   m_updateTimer.start(10);
}

void RenderWidget::stopUpdateLoop()
{
  m_updateTimer.stop();
}
