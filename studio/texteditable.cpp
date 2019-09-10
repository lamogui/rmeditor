#include "texteditable.hpp"
#include <QTextStream>
#include "logmanager.hpp"

TextEditable::TextEditable(QObject* _parent, const QFileInfo& _path) :
	MediaFile(_parent,_path)
{
}

bool TextEditable::load()
{
	QString text;
	QFile file(m_path.absoluteFilePath());
	bool readSuccess = false;
	if (!file.exists())
	{
		pwarning(Log::File, this, tr("File ") + m_path.absoluteFilePath() + tr(" doesn't exists create it"));
		if (!file.open(QIODevice::WriteOnly))
		{
			perror(Log::File, this, tr("Can't create file ") + m_path.absoluteFilePath());
		}
		else
		{
			readSuccess = true;
		}
	}
	else
	{
		if (!file.open(QIODevice::ReadOnly))
		{
			perror(Log::File, this, tr("Can't open file ") + m_path.absoluteFilePath());
		}
		else
		{
			QTextStream stream(&file);
			text = stream.readAll();
			readSuccess = (stream.status() == QTextStream::Ok);
		}
	}
	file.close();
	return readSuccess && build(text);
}

bool TextEditable::save(const QString& _text)
{
	QFile file(m_path.absoluteFilePath());
	if (!file.open(QIODevice::WriteOnly))
	{
		perror(Log::File, this, tr("Can't open file ") + m_path.absoluteFilePath() + tr(" for wrinting"));
		return false;
	}

	QTextStream stream(&file);
	stream << _text;
	stream.flush();
	file.close();

	return true;
}


bool TextEditable::save()
{
	return save(m_text);
}

