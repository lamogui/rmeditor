
#include "scene.hpp"

#include "framework.hpp"
#include "renderer.hpp"
#include "shaderminifier.hpp"
#include "logwidget.hpp"

Scene::Scene(const QString &filename, QDomNode node, Framework *framework, LogWidget &log, QObject *parent) :
  FragmentShaderCode(filename, node, log, parent),
  framework(framework),
  camera(new Camera),
  sceneRender(new SceneRender(*this,QSize(1280,720),this))
{
  sceneRender->getCamera() = camera;
  load();
}

const char* Scene::getVertexShaderCode()
{

  static const char* vertexShader =
    "#version 450\n"
    "in vec2 position;"
    "in vec2 texCoords;"
    "smooth out vec2 coords;"
    "smooth out vec2 uv;"
    "void main()"
    "{"
    "coords         = texCoords;"
    "uv             = position;"
    "gl_Position    = vec3(position, 0.0);"
    "}";
  return vertexShader;
}

Render* Scene::getRender() const
{
  return sceneRender;
}

bool Scene::build(const QString &text)
{
  shaderProgram.reset(new QOpenGLShaderProgram());
  shaderProgram->create();

  fragmentcode = text;
  QString code;

  if (framework)
  {
    code += framework->getText();
    emit startLineNumberChanged(code.count(QChar::LineFeed));
  }
  code += fragmentcode;
  if (!handleShaderCompileResult(code, QOpenGLShader::Fragment))
    return false;
 
  if (!shaderProgram->addShader(&shader))
  {
    log.writeError(fileName() + " (at compile): " + shaderProgram->log());
    return false;
  }
  else if (!shaderProgram->log().isEmpty())
    log.writeWarning(fileName() + " (at compile): " + shader.log());
  
  if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, getVertexShaderCode()))
  {
    log.writeError(fileName() + " (at compile): " + shaderProgram->log());
    return false;
  }
  else if (!shaderProgram->log().isEmpty())
    log.writeWarning(fileName() + " (at compile): " + shader.log());


  // link
  shaderProgram->bindAttributeLocation("position", VertexAttributesIndex::position);
  shaderProgram->bindAttributeLocation("texCoords", VertexAttributesIndex::texCoord);

  if (!shaderProgram->link())
  {
    log.writeError(fileName() + " (at link): " + shaderProgram->log());
    return false;
  }
  else if (!shaderProgram->log().isEmpty())
    log.writeWarning(fileName() + " (at link): " + shader.log());

  return shaderProgram->isLinked();
}



