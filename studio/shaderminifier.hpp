#ifndef SHADERMINIFIER_HPP
#define SHADERMINIFIER_HPP

#include <QString>

class FragmentShaderCode;
class ShaderMinifier
{
public:
	static QString minifiedShaderCode(const FragmentShaderCode& _shaderfile);
	static QString cFormatedShaderCode(const QString &_variableName, const QString& _minifiedCode);

};

#endif
