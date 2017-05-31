
#include "scene.hpp"

#include "framework.hpp"
#include "renderer.hpp"
#include "shaderminifier.hpp"
#include "logwidget.hpp"
#include "camera.hpp"

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

  // Take care of the code
  fragmentcode = text; // Save this for the code minimiser 
  QString code;
  if (framework)
  {
    code += framework->getText();
    emit startLineNumberChanged(code.count(QChar::LineFeed));
  }
  code += fragmentcode;

  // Create the shader program
  shaderProgram.reset(new ShaderProgram());
  shaderProgram->create();
  
  if (!handleShaderCompileResult(getVertexShaderCode(), *shaderProgram, QOpenGLShader::Vertex) ||
      !handleShaderCompileResult(code, *shaderProgram, QOpenGLShader::Fragment))
    return false;

  // link
  shaderProgram->bindAttributeLocation("position", VertexAttributesIndex::position);
  shaderProgram->bindAttributeLocation("texCoords", VertexAttributesIndex::texCoord);

  return handleShaderLinkResult(*shaderProgram) && shaderProgram->isLinked();
}



