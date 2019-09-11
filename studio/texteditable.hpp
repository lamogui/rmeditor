

#ifndef TEXTEDITABLE_HPP
#define TEXTEDITABLE_HPP

#include "mediafile.hpp"

class Renderer;
class TextEditable : public MediaFile /* Represent any text compilable text media */
{
	Q_OBJECT

public:
	TextEditable(QObject* _parent, const QFileInfo& _path);

	// Accessors
	inline const QString& getText() const {return m_text; }

	//Is the text object buildable
	virtual bool buildable() const {return false;}

public slots:
	//Return true is the file is loaded or created (not if it build correctly)
	bool loadFromDisk() final;
	virtual bool saveToDisk();
	virtual bool build() { return false; } // Build the asociated object / resource

signals:
	// text edition
	void startLineNumberChanged(int _line);
	void textChanged(QString _text);

protected:
	bool saveToDisk(const QString& _text);

private:
	//The text to print in the editor and that will be saved in the file
	QString m_text;

};

#endif
