#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QTime>
#include <QOpenGLWidget>
#include <QListWidget>
#include <QTimer>
#include <QSet>
#include <QOpenGLDebugLogger>


class Camera;
class RenderTexture2D;
class Fast2DQuad;
class Renderer;
class LogWidget;
class RenderWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
       RenderWidget(QWidget *parent = 0);
       ~RenderWidget();

       void setLogWidget(LogWidget* log);

public slots:
       void resetCamera();
       void takeScreenshot();

       inline void setOnlyShowTexture(bool e) { onlyShowTexture = e;}
       void startUpdateLoop();
       void stopUpdateLoop();
       void setCurrentRenderer(const QWeakPointer<Renderer>& renderer);

protected slots:
       // QGLWidget
       void mousePressEvent(QMouseEvent* event) override;
       void mouseReleaseEvent(QMouseEvent* event) override;
       void mouseMoveEvent(QMouseEvent* event) override;
       void wheelEvent(QWheelEvent* event) override;
       void keyPressEvent(QKeyEvent* event) override;
       void keyReleaseEvent(QKeyEvent* event) override;

protected:
       void initializeGL() override;
       void paintGL() override;

       QTimer updateTimer;
       RenderTexture2D* render;
       Fast2DQuad* quad;
       QWeakPointer<Renderer> currentRenderer;
       QPointF previousMousePos;
       QSet<Qt::Key> keysPressed;
       QOpenGLDebugLogger openglDebugLogger;
       LogWidget* logWidget;
       bool captureMouse;
       bool onlyShowTexture;

};

#endif // RENDERWIDGET_H
