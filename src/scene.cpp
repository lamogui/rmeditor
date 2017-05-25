
#include "scene.hpp"

#include "framework.hpp"
#include "renderer.hpp"
#include "shaderminifier.hpp"

Scene::Scene(const QString &filename, QDomNode node, Framework *framework, LogWidget &log, QObject *parent):
  FragmentShaderCode(filename,node,log,parent),
  framework(framework),
  sceneRenderer(new SceneRenderer(*this,1280,720,this))
{
  sceneRenderer->setCamera(&camera);
  load();
}

Renderer* Scene::getRenderer() const
{
  return sceneRenderer;
}

Scene::~Scene()
{
  sceneRenderer->setCamera(NULL);
}


bool Scene::build(const QString &text)
{
  fragmentcode = text;
  QString code;
  if (framework)
  {
    code += framework->getText();
    emit startLineNumberChanged(code.count(QChar::LineFeed));
  }
  code += fragmentcode;
  return (shader.compil(Shader::getVertexShader(), code.toStdString().c_str()) == SHADER_SUCCESS);
}

