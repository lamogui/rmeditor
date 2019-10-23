
#include "shaderminifier.hpp"
#include "logmanager.hpp"
#include "shadercode.hpp"
#include <QObject> // for tr

QString ShaderMinifier::minifiedShaderCode( const FragmentShaderCode& _shaderfile )
{
	QString minified = QString::fromStdString( _shaderfile.text().toStdString() );
	minified.replace(QChar('\r'),QString());
	minified.replace(QChar('\t'),QString(" "));


	/* remove all comments */
	int startComment;
	while ((startComment = minified.indexOf("//"))!=-1)
	{
		int endComment = minified.indexOf("\n",startComment);
		if (endComment == -1)
		{
			pwarning( Log::Shader, &_shaderfile, QObject::tr("missing new line after '//' comment !") );
			break;
		}
		else
		{
			minified.remove(startComment, endComment - startComment );
		}
	}
	while ((startComment = minified.indexOf("/*"))!=-1)
	{
		int endComment = minified.indexOf("*/",startComment);
		if (endComment == -1)
		{
			perror( Log::Shader, &_shaderfile, QObject::tr("unterminated multiline comment cannot minifie properly !"));
			break;
		}
		else
		{
			minified.remove(startComment, endComment + 2 - startComment );
		}
	}

	/* remove double spaces/newlines */
	while (minified.contains("  "))
	{
		minified.replace(QString("  "),QString(" "));
	}
	while (minified.contains(" \n"))
	{
		minified.replace(QString(" \n"),QString("\n"));
	}
	while (minified.contains("\n "))
	{
		minified.replace(QString("\n "),QString("\n"));
	}

	pinfo( Log::Shader, &_shaderfile, QObject::tr("minified shadercode size: ") + QString::number(minified.length() + 1) + QObject::tr(" bytes"));

	return minified;
}

QString ShaderMinifier::cFormatedShaderCode(const QString& _variableName, const QString& _minifiedCode)
{
	int indentLvl = 0;
	QString sCode;
	QString cCode;
	QStringList lines = _minifiedCode.split('\n',QString::SkipEmptyParts);

	unsigned int size = 0;

	QString previous_line;
	foreach (QString line, lines)
	{
		QString begin;
		QString end;
		line.replace("\"","\\\"");
		if (line.contains("#") || line.contains("\\"))
		{
			if (!previous_line.contains("#") && line.contains("#"))
			{
				begin = "\\n";
			}
			end = "\\n";
		}

		if (line.contains("}"))
		{
			indentLvl-=2;
		}
		sCode += "\n" + QString().fill(' ',indentLvl) + "\"" + begin + line + end + "\"";
		size += (line+end).length();
		if (line.contains("{"))
		{
			indentLvl+=2;
		}
		previous_line = line;
	}
	sCode += ";\n";

	//cCode = QString("const char ") + variable_name + "[" +QString::number(size + 1) + "]" + " = ";
	cCode = QString("const char* const ") + _variableName + " = ";
	cCode += sCode;
	//cCode += QString("const unsigned int ") + variable_name + "_len = " + QString::number(size + 1) + ";\n";
	pinfo( Log::Shader, nullptr , QObject::tr("C formated shadercode size: ") + QString::number(size + 1) + QObject::tr(" bytes") );
	return cCode;
}
