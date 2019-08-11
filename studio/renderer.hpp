
#ifndef RMEDITOR_RENDERER_HPP
#define RMEDITOR_RENDERER_HPP

#include "renderfunctionscache.hpp" // because it's a typedef
#include <QHash>
#include <QSharedPointer>


class Render;
class CameraState;
class Project;
class Scene;

/*
** Renderer : class that know how to render a media using opengl
*/
class Renderer 
{
    friend class SceneRenderer;

public:
  Renderer();
  virtual ~Renderer() {}

  // Rendering
	virtual void initializeGL(RenderFunctionsCache& _gl) = 0;
	virtual void renderChildrens(RenderFunctionsCache& _gl) { (void)_gl; }            // Pre-render all the internals FBOs/GeometryShaders before "real" render
	virtual void glRender(RenderFunctionsCache& _gl, Render& _render) = 0; // You must not bind any FBO in this function

	CameraState* m_linkedCameraState; // if we can control the camera this is a link to the data both input / output...

	// Time variables (TODO put thoses in a dedicated struct
	float m_musicTime;    // current music time (= global timeline time)
	float m_sequenceTime; // time since the last cut in the timeline
	float m_mediaTime;    // time over media (if any)

	// Music variables
	float m_bassLvl;
	float m_bassLvlTime;
	float m_bassLvlSequenceTime;
	float m_noiseLvl;
	float m_noiseLvlTime;
	float m_noiseLvlSequenceTime;

  // Media
	inline virtual bool hasFixedLength() const { return false; }         // Does the Renderer is linked to a fixed media length
  inline virtual float getLength() const { return 0.f; }               // the media length if any 

};

#endif // !RMEDITOR_RENDERER_HPP
