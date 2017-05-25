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

       void onRendererDestroy();



protected:
       void initializeGL();
       void paintGL();
       void resizeGL(int width, int height);


       QTimer updateTimer;
       Renderer* renderer;
       QPointF previousMousePos;
       QSet<Qt::Key> keysPressed;
       bool captureMouse;

       bool onlyShowTexture;

};

#endif // RENDERWIDGET_H
