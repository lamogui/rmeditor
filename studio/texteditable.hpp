

#ifndef TEXTEDITABLE_HPP
#define TEXTEDITABLE_HPP

#include "nodefile.hpp"

class Renderer;
class TextEditable : public NodeFile
{
	Q_OBJECT

public:
	TextEditable(const QString& _filename, QDomNode _node, QObject* _parent);

	//The text to print in the editor and that will be saved in the file
	virtual const QString& text() const = 0;

	//Is the object buildable
	virtual bool buildable() const {return false;}
	inline virtual Renderer* getRenderer() const {return nullptr;}

public slots:
	//Return true is the file is loaded or created (not if it build correctly)
	bool load();
	virtual bool save();
	virtual bool build(const QString& _text) = 0;

signals:
	void startLineNumberChanged(int _line);
	void objectTextChanged(QString _text);

protected:
	bool save(const QString& _text);

};

#endif
