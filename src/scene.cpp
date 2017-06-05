
#include "scene.hpp"

/*
** RaymarchingScene
*/
RaymarchingScene::RaymarchingScene() :
  QuadFragmentShaderCode()
{

}

RaymarchingScene::RaymarchingScene(const RaymarchingScene& other) :
  RaymarchingScene()
{
  Q_ASSERT(false); // do not use this constructor (forbiden by QObject)
}

void RaymarchingScene::initializeGL(RenderFunctionsCache& gl)
{
  BaseClass::initializeGL(gl);
  defaultRenderer->setCurrentCamera(defaultCamera);
}

Renderer* RaymarchingScene::createRenderer() const
{
  ReferencedRaymarchingSceneRenderer* renderer = new ReferencedRaymarchingSceneRenderer();
  renderer->setShaderCode(this);
  return renderer;
}

/*
** ReferencedRaymarchingSceneRenderer
*/

void ReferencedRaymarchingSceneRenderer::configureUniforms(ShaderProgram& program)
{
  if (currentCamera)
  {
    // TODO 
  }
  BaseClass::configureUniforms(program);
}




