
#include "scene.hpp"

/*
** RaymarchingScene
*/
RaymarchingScene::RaymarchingScene() :
  QuadFragmentShaderCode()
{
  defaultRenderer->setCurrentCamera(defaultCamera); // Ugly !!
}

RaymarchingScene::RaymarchingScene(const RaymarchingScene& other) :
  QuadFragmentShaderCode(other)
{
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




