#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QTime>
#include <QGLWidget>
#include <QListWidget>
#include <QTimer>
#include <QSet>

class Camera;
class Renderer;



class RenderWidget : public QGLWidget
{
    Q_OBJECT

public:
       RenderWidget(QWidget *parent = 0);
       ~RenderWidget();

public slots:
       void resetCamera();
       Camera* camera() const;
       void takeScreenshot();

       void setRenderer(Renderer* renderer);

       inline void setOnlyShowTexture(bool e) { m_onlyShowTexture = e;}
       void startUpdateLoop();
       void stopUpdateLoop();



protected slots:
       void mousePressEvent(QMouseEvent* event);
       void mouseReleaseEvent(QMouseEvent* event);
       void mouseMoveEvent(QMouseEvent* event);
       void wheelEvent(QWheelEvent* event);
       void keyPressEvent(QKeyEvent* event);
       void keyReleaseEvent(QKeyEvent* event);

       virtual void onRendererDestroy();



protected:
       void initializeGL();
       void paintGL();
       void resizeGL(int width, int height);


       QTimer m_updateTimer;
       Renderer* m_renderer;
       QPointF m_previousMousePos;
       QSet<Qt::Key> m_keysPressed;
       bool m_captureMouse;

       bool m_onlyShowTexture;

};

#endif // RENDERWIDGET_H
