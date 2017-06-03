
#include "quadfragmentshader.hpp"

/*
** QuadFragmentShaderCode
*/

QuadFragmentShaderCode::QuadFragmentShaderCode() :
  GLSLShaderCode()
{

}

QuadFragmentShaderCode::QuadFragmentShaderCode(const QuadFragmentShaderCode& other) :
  GLSLShaderCode(other)
{

}

Renderer* QuadFragmentShaderCode::createRenderer() const
{
  ReferencedQuadFragmentShaderRenderer* renderer = new ReferencedQuadFragmentShaderRenderer();
  renderer->setShaderCode(this);
  return renderer;
}

const char* QuadFragmentShaderCode::getVertexShaderCode()
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
    "gl_Position    = vec4(position, 0.0, 0.0);"
    "}";
  return vertexShader;
}

bool QuadFragmentShaderCode::build(const QString &text)
{
  // Take care of the code
  shaderCode = text; // Save this for the code minimiser 
  QString code;
  if (getFramework())
  {
    code += getFramework().lock()->getShaderCodeRecursive();
    emit startLineNumberChanged(code.count(QChar::LineFeed));
  }
  code += shaderCode;

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

/*
** ReferencedQuadFragmentShaderRenderer
*/

void ReferencedQuadFragmentShaderRenderer::glRender(RenderFunctionsCache& gl, Render& render)
{
  QSharedPointer<ShaderProgram> shader = shaderCode ? shaderCode->getShaderProgram() : nullptr;
  if (shader)
  {
    shader->bind();
    configureUniforms(*shader);
    quad.draw();
    shader->release();
  }
}

void ReferencedQuadFragmentShaderRenderer::configureUniforms(ShaderProgram& program)
{
  program.setUniformValue("musicTime", musicTime);
  program.setUniformValue("sequenceTime", sequenceTime);
  program.setUniformValue("mediaTime", mediaTime);
  program.setUniformValue("bassLvl", bassLvl);
  program.setUniformValue("bassLvlTime", bassLvlTime);
  program.setUniformValue("bassLvlSequenceTime", bassLvlSequenceTime);
  program.setUniformValue("noiseLvl", noiseLvl);
  program.setUniformValue("noiseLvlTime", noiseLvlTime);
  program.setUniformValue("noiseLvlSequenceTime", noiseLvlSequenceTime);
}