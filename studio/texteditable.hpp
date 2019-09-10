

#ifndef TEXTEDITABLE_HPP
#define TEXTEDITABLE_HPP

#include "mediafile.hpp"

class Renderer;
class TextEditable : public MediaFile /* Represent any text compilable text media */
{
	Q_OBJECT

public:
	TextEditable(QObject* _parent, const QFileInfo& _path);

	//The text to print in the editor and that will be saved in the file
	QString m_text;

	//Is the text object buildable
	virtual bool buildable() const {return false;}

public slots:
	//Return true is the file is loaded or created (not if it build correctly)
	bool load() final;
	virtual bool save();
	virtual bool build(const QString& /*_text*/) { return false; }


signals:
	// text edition
	void startLineNumberChanged(int _line);

protected:
	bool save(const QString& _text);
};

#endif
