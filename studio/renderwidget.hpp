#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QTimer>
#include <QSet>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPointer>
#include "jassert.hpp"

#include "renderfunctionscache.hpp"
#include "renderer.hpp"

class Camera;
class RenderTexture2D;
class Fast2DQuad;
class Render;
class RenderWidget : public QOpenGLWidget
{
	Q_OBJECT

public:
	RenderWidget(QWidget *_parent = nullptr);

	// Utils
	static const char* getDisplayTextureFragmentShaderCode();

	// Enum
	enum TextureDisplayed
	{
	 color = 0,
	 normal = 1,
	 depth = 2
	};

	// Attributes
	QPointer<Renderer> m_currentRenderer;
	TextureDisplayed m_textureDisplayed;

	// Accessors
	RenderFunctionsCache& getRenderFunctions() { jassert(m_renderFunctions); return *m_renderFunctions; }

public slots:
	void resetCamera();
	void takeScreenshot();

	void startUpdateLoop();
	void stopUpdateLoop();
	void switchDisplayedTexture();

protected slots:
	void cleanup();

protected:
	// QWidget
	void mousePressEvent(QMouseEvent* _event) override;
	void mouseReleaseEvent(QMouseEvent* _event) override;
	void mouseMoveEvent(QMouseEvent* _event) override;
	void wheelEvent(QWheelEvent* _event) override;
	void keyPressEvent(QKeyEvent* _event) override;
	void keyReleaseEvent(QKeyEvent* _event) override;

	// QOpenGLWidget
	void initializeGL() override;
	void paintGL() override;

	// Time
	QTimer m_updateTimer;

	// Render objects
	RenderFunctionsCache* m_renderFunctions;
	RenderTexture2D* m_render;          // fbo
	Fast2DQuad* m_quad;                 // geometry
	QOpenGLShaderProgram* m_quadShader;  // shader

	QOpenGLDebugLogger* m_openglDebugLogger;

	// Control (mouse / keyboard)
	QPointF m_previousMousePos;
	QSet<Qt::Key> m_keysPressed;

	// Hum let's clean this one day
	bool m_captureMouse;


};

#endif // RENDERWIDGET_H
