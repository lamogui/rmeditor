#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QTimer>
#include <QSet>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include "jassert.hpp"

#include "renderfunctionscache.hpp"

class Camera;
class RenderTexture2D;
class Fast2DQuad;
class Render;
class Renderer;
class RenderWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
       RenderWidget(QWidget *parent = 0);
       ~RenderWidget();

       // Utils
       static const char* getDisplayTextureFragmentShaderCode();

       // Enum
       enum TextureDisplayed
       {
         color = 0,
         normal = 1,
         depth = 2
       };

       // Accessors
       QWeakPointer<Renderer> getCurrentRenderer() const { return currentRenderer; }
       TextureDisplayed getTextureDisplayed() const { return textureDisplayed; }
       RenderFunctionsCache& getRenderFunctions() { jassert(renderFunctions); return *renderFunctions; }

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
       bool captureMouse;
       bool onlyShowTexture;

       // Options 
       TextureDisplayed textureDisplayed;

};

#endif // RENDERWIDGET_H
