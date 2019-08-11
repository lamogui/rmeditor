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
	GLSLShaderCode(QObject* parent = nullptr);

	// Links
	QPointer<GLSLShaderCode> m_framework;

	// TextEditable
	const QString& getText() const override;

	QString getShaderCodeRecursive() const;

	// overridable
	virtual QString minifiedShaderCode(const ShaderMinifier& _minifier) const;
	virtual QString cFormatedShaderCode(const ShaderMinifier& _minifier) const;

protected:
	// Utils
	bool handleShaderCompileResult(const QString& _shaderCode, ShaderProgram& _program, QOpenGLShader::ShaderType _type);
	bool handleShaderLinkResult(ShaderProgram& _program);

	QString m_shaderCode;

};

#endif // SCENE_H
