#ifndef RAYMARCHINGSCENE_HPP
#define RAYMARCHINGSCENE_HPP

#include "quadfragmentshader.hpp"

/*
** RaymarchingScene : textfile media that contains a fragment shader using an external camera 
*/
class RaymarchingScene : public QuadFragmentShaderCode
{
  Q_OBJECT

public:
  Q_INVOKABLE RaymarchingScene(QObject* parent = nullptr);

  void initializeGL(RenderFunctionsCache& gl) override;

  // MediaFile Renderer
  Renderer* createRenderer() const override;

private:
  typedef QuadFragmentShaderCode BaseClass;
};

/*
** ReferencedRaymarchingSceneRenderer : 
** the same as ReferencedQuadFragmentShaderRenderer but taking 
** in account external camera to facilitate the process
*/
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
