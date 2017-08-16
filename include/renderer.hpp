
#ifndef RMEDITOR_RENDERER_HPP
#define RMEDITOR_RENDERER_HPP

#include "renderfunctionscache.hpp" // because it's a typedef
#include <QHash>
#include <QSharedPointer>


class Render;
class Camera;

/*
** Renderer : class that know how to render a media using opengl
*/
class Renderer 
{
public:
  Renderer();
  virtual ~Renderer() {}

  // Rendering
  virtual void initializeGL(RenderFunctionsCache& gl) = 0;
  virtual void renderChildrens(RenderFunctionsCache& gl) {}            // Pre-render all the internals FBOs/GeometryShaders before "real" render
  virtual void glRender(RenderFunctionsCache& gl, Render& render) = 0; // You must not bind any FBO in this function 

  // Camera
  inline virtual bool hasDynamicCamera() const { return false; }       // Can we control the camera of the renderer
  virtual void setCurrentCamera(const QWeakPointer<Camera>& camera) {} // Set the camera that should be used to render 
  virtual Camera* getCurrentCamera() { return nullptr; }               // Get the current camera to modify it's posistion if needed ! 

  // Media
  inline virtual bool hasFixedLength() const { return false; }         // Does the Renderer is linked to a fixed media length
  inline virtual float getLength() const { return 0.f; }               // the media length if any 

  // Time 
  void setTimeVariables(float music, float sequence, float media);


  // Music
  void setBassVariables(float bass, float bassTime, float bassSequenceTime);
  void setNoiseVariables(float noise, float noiseTime, float noiseSequenceTime);


protected:
  // Time variables
  float musicTime;    // current music time (= global timeline time)
  float sequenceTime; // time since the last cut in the timeline 
  float mediaTime;    // time over media (if any) 

  // Music variables 
  float bassLvl;
  float bassLvlTime;     
  float bassLvlSequenceTime; 
  float noiseLvl;
  float noiseLvlTime;
  float noiseLvlSequenceTime;

};



#endif // !RMEDITOR_RENDERER_HPP