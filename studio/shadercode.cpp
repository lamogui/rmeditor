
#include "shadercode.hpp"

#include "logwidget.hpp"
#include "shaderminifier.hpp"

FragmentShaderCode::FragmentShaderCode(const QString &_filename, QDomNode _node, QObject *_parent):
  TextEditable(_filename,_node,_parent)
{
}

const QString &FragmentShaderCode::text() const
{
  return m_fragmentcode;
}

bool FragmentShaderCode::build(const QString &text)
{
  m_fragmentcode = text;
  return (m_shader.compil(Shader::getVertexShader(), m_fragmentcode.toStdString().c_str()) == SHADER_SUCCESS);
}


QString FragmentShaderCode::minifiedShaderCode() const
{
	return ShaderMinifier::minifiedShaderCode( *this );
}

QString FragmentShaderCode::cFormatedShaderCode() const
{
  QString variable_name = QString("fs_") + this->objectName();
	return ShaderMinifier::cFormatedShaderCode( variable_name, minifiedShaderCode() );
}

