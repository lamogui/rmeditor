
#include "shadercode.hpp"

#include "logmanager.hpp"
#include "shaderminifier.hpp"

/*
** GLSLShaderCode
*/

GLSLShaderCode::GLSLShaderCode(QObject* _parent, const QFileInfo& _path) :
	TextEditable(_parent, _path)
{

}

QString GLSLShaderCode::getShaderCodeRecursive() const
{
	if ( m_framework )
  {
		return m_framework->getShaderCodeRecursive() + getText();
  }
	return getText();
}

QString GLSLShaderCode::minifiedShaderCode(const ShaderMinifier& minifier) const
{
	return minifier.minifiedShaderCode(m_path.fileName(), getText());
}

QString GLSLShaderCode::cFormatedShaderCode(const ShaderMinifier& minifier) const
{
  QString variable_name = QString("fs_") + this->objectName();
	return minifier.cFormatedShaderCode(m_path.fileName(),variable_name,minifiedShaderCode(minifier));
}

bool GLSLShaderCode::handleShaderCompileResult(const QString& shaderCode, ShaderProgram& program, QOpenGLShader::ShaderType type)
{
  if (!program.addShaderFromSourceCode(type, shaderCode))
  {
		perror(Log::Shader, this, program.log());
    return false;
  }
	else if (!program.log().isEmpty()) {
	 pwarning(Log::Shader, this, program.log());
	}
  return true;
}

bool GLSLShaderCode::handleShaderLinkResult(ShaderProgram& program)
{
  if (!program.link())
  {
		perror(Log::Shader, this, program.log());
    return false;
  }
	else if (!program.log().isEmpty())
	{
		pwarning(Log::Shader, this, program.log());
	}
  return true;
}

