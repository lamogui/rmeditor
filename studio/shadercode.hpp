#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP

#include "texteditable.hpp"
#include <QPointer>

class ShaderMinifier;

/*
** GLSLShaderCode : Represent any shader code media text 
*/
class GLSLShaderCode : public TextEditable 
{
  Q_OBJECT

public:
	GLSLShaderCode(QObject* _parent, const QFileInfo& _path);

	// Links
	QPointer<GLSLShaderCode> m_framework;

	QString getShaderCodeRecursive() const;

	// overridable
	virtual QString minifiedShaderCode(const ShaderMinifier& _minifier) const;
	virtual QString cFormatedShaderCode(const ShaderMinifier& _minifier) const;

protected:
	// Utils
	bool handleShaderCompileResult(const QString& _shaderCode, ShaderProgram& _program, QOpenGLShader::ShaderType _type);
	bool handleShaderLinkResult(ShaderProgram& _program);
};

#endif // SCENE_H
