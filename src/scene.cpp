
#include "scene.hpp"

#include "framework.hpp"
#include "renderer.hpp"
#include "shaderminifier.hpp"

Scene::Scene(const QString &filename, QDomNode node, Framework *framework, LogWidget &log, QObject *parent):
  FragmentShaderCode(filename,node,log,parent),
  m_framework(framework),
  m_sceneRenderer(new SceneRenderer(*this,1280,720,this))
{
  m_sceneRenderer->setCamera(&m_camera);
  load();
}

Renderer* Scene::getRenderer() const
{
  return m_sceneRenderer;
}

Scene::~Scene()
{
  m_sceneRenderer->setCamera(NULL);
}


bool Scene::build(const QString &text)
{
  m_fragmentcode = text;
  QString code;
  if (m_framework)
  {
    code += m_framework->text();
    emit startLineNumberChanged(code.count(QChar::LineFeed));
  }
  code += m_fragmentcode;
  return (m_shader.compil(Shader::getVertexShader(), code.toStdString().c_str()) == SHADER_SUCCESS);
}

