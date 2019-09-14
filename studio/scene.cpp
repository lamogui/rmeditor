
#include "scene.hpp"
#include "project.hpp"

/*
** RaymarchingScene
*/
RaymarchingScene::RaymarchingScene(Project& _parent) :
	QuadFragmentShaderCode(&parent)
{

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




