#ifndef RAYMARCHINGSCENE_HPP
#define RAYMARCHINGSCENE_HPP

#include "quadfragmentshader.hpp"



class RaymarchingScene : public QuadFragmentShaderCode
{
  Q_OBJECT

public:
  RaymarchingScene();
  RaymarchingScene(const RaymarchingScene& other);

  void initializeGL(RenderFunctionsCache& gl) override;

  // MediaFile Renderer
  Renderer* createRenderer() const override;

private:
  typedef QuadFragmentShaderCode BaseClass;
};

// FIXME : create a class instatiator that to the job of QMetaType without need a copy constructor
Q_DECLARE_METATYPE(RaymarchingScene); // only to instantiate from class name 

class ReferencedRaymarchingSceneRenderer : public ReferencedQuadFragmentShaderRenderer
{
public:
  ReferencedRaymarchingSceneRenderer() {}

  // Camera
  inline bool hasDynamicCamera() const override { return true; }     
  void setCurrentCamera(const QWeakPointer<Camera>& camera) { currentCamera = camera; }
  Camera* getCurrentCamera() override { return currentCamera.lock().data(); }

protected:
  // ReferencedQuadFragmentShaderRenderer
  void configureUniforms(ShaderProgram& program) override;

  QWeakPointer<Camera> currentCamera;

private:
  typedef ReferencedQuadFragmentShaderRenderer BaseClass;
};


#endif // !RAYMARCHINGSCENE_HPP
