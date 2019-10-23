#ifndef SHADERCODE_HPP
#define SHADERCODE_HPP


#include <QString>
#include "shader.hpp"
#include "texteditable.hpp"
#include "fbo.hpp"


class ShaderMinifier;
class FragmentShaderCode : public TextEditable
{
  Q_OBJECT

public:
	FragmentShaderCode(const QString& _filename, QDomNode _node, QObject* _parent=nullptr);

	virtual const QString& text() const;
	inline Shader& getShader() { return m_shader;}

	virtual bool buildable() const { return true; }

	virtual QString minifiedShaderCode() const;
	virtual QString cFormatedShaderCode() const;

public slots:
	virtual bool build(const QString& text);

protected:
	Shader m_shader;
	QString m_fragmentcode;

};

#endif // SCENE_H
