#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QTimer>
#include <QSet>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>

#include "renderfunctionscache.hpp"

class Camera;
class RenderTexture2D;
class Fast2DQuad;
class Render;
class Renderer;
class LogWidget; // FIXME : remove this !
class RenderWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
       RenderWidget(QWidget *parent = 0);
       ~RenderWidget();

       void setLogWidget(LogWidget* log); // FIXME : remove this !

       RenderFunctionsCache& getRenderFunctions() { Q_ASSERT(renderFunctions); return *renderFunctions; }

       static const char* getDisplayTextureFragmentShaderCode();

       enum TextureDisplayed
       {
         color = 0,
         normal = 1,
         depth = 2
       };

       TextureDisplayed getTextureDisplayed() const { return textureDisplayed; }

public slots:
       void resetCamera();
       void takeScreenshot();

       inline void setOnlyShowTexture(bool e) { onlyShowTexture = e;}
       void startUpdateLoop();
       void stopUpdateLoop();
       void setCurrentRenderer(const QWeakPointer<Renderer>& renderer);
       void switchDisplayedTexture();

protected slots:
       void cleanup();

protected:
       // QWidget
       void mousePressEvent(QMouseEvent* event) override;
       void mouseReleaseEvent(QMouseEvent* event) override;
       void mouseMoveEvent(QMouseEvent* event) override;
       void wheelEvent(QWheelEvent* event) override;
       void keyPressEvent(QKeyEvent* event) override;
       void keyReleaseEvent(QKeyEvent* event) override;

       // QOpenGLWidget
       void initializeGL() override;
       void paintGL() override;

       // Time
       QTimer updateTimer;

       // Render objects 
       RenderFunctionsCache* renderFunctions;
       RenderTexture2D* render;          // fbo
       Fast2DQuad* quad;                 // geometry
       QOpenGLShaderProgram* quadShader;  // shader
       QWeakPointer<Renderer> currentRenderer;
       QOpenGLDebugLogger* openglDebugLogger;

       // Control (mouse / keyboard)
       QPointF previousMousePos;
       QSet<Qt::Key> keysPressed;

       // Hum let's clean this one day 
       LogWidget* logWidget;
       bool captureMouse;
       bool onlyShowTexture;

       // Options 
       TextureDisplayed textureDisplayed;

};

#endif // RENDERWIDGET_H
