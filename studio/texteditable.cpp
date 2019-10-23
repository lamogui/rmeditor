#include "texteditable.hpp"
#include "logmanager.hpp"
#include <QTextStream>


TextEditable::TextEditable(const QString & _filename, QDomNode _node, QObject * _parent):
	NodeFile(_filename,_node,_parent)
{
}

bool TextEditable::load()
{
	QString text;
	bool readSuccess = false;
	if (!this->exists())
	{
		pwarning( Log::File, this, tr("file doesn't exists create it"));
		if (!open(QIODevice::WriteOnly))
		{
			perror(Log::File, this, tr("Can't create file ") + fileName());
		}
		else
		{
			readSuccess = true;
		}
	}
	else
	{
		if (!open(QIODevice::ReadOnly))
		{
			perror(Log::File, this, tr("Can't open file ") + fileName());
		}
		else
		{
			QTextStream stream(this);
			text = stream.readAll();
			readSuccess = (stream.status() == QTextStream::Ok);
		}
	}
	close();
	build(text);
	return readSuccess;
}

bool TextEditable::save(const QString& _text)
{
	if (!open(QIODevice::WriteOnly))
	{
		perror(Log::File, this, tr("Can't open file ") + fileName() + QString(" for wrinting"));
		return false;
	}

	QTextStream stream(this);
	stream << _text;
	stream.flush();

	close();
	return true;
}


bool TextEditable::save()
{
	return save(text());
}




