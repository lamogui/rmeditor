#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QTime>
#include <QOpenGLWidget>
#include <QListWidget>
#include <QTimer>
#include <QSet>
#include <QOpenGLDebugLogger>

class Camera;
class Render;
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
       Camera* camera() const;
       void takeScreenshot();

       void setRender(Render* renderer);

       inline void setOnlyShowTexture(bool e) { onlyShowTexture = e;}
       void startUpdateLoop();
       void stopUpdateLoop();



protected slots:
       // QGLWidget
       void mousePressEvent(QMouseEvent* event) override;
       void mouseReleaseEvent(QMouseEvent* event) override;
       void mouseMoveEvent(QMouseEvent* event) override;
       void wheelEvent(QWheelEvent* event) override;
       void keyPressEvent(QKeyEvent* event) override;
       void keyReleaseEvent(QKeyEvent* event) override;

       void onRenderDestroy();



protected:
       void initializeGL();
       void paintGL();
       void resizeGL(int width, int height);


       QTimer updateTimer;
       Render* renderer;
       QPointF previousMousePos;
       QSet<Qt::Key> keysPressed;
       QOpenGLDebugLogger openglDebugLogger;
       LogWidget* logWidget;
       bool captureMouse;

       bool onlyShowTexture;

};

#endif // RENDERWIDGET_H
