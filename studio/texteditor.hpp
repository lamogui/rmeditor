#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

#include "texteditable.hpp"

class LineNumberArea;
class Highlighter;

class TextEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	TextEditor(TextEditable& textObject, QWidget *parent = 0);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();


	inline TextEditable* textObject() const { return m_object; }

public slots:
	bool build();
	bool save();
	//void refresh();

signals:
	void saved(TextEditor* editor, bool);


protected:
	void resizeEvent(QResizeEvent * _event);

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &, int);
	void resetTimer();

	void setStartLineNumber(int n);

	void onTextEdited();
	void onDestroyObject(QObject* obj);

private:
	TextEditable* m_object;
	QString m_colorSyntax(QString in);
	QWidget * m_lineNumberArea;
	QString m_filename;
	Highlighter *m_highlighter;
	int m_id;
	QTimer *m_timer;
	int m_startLineNumber;
	bool m_saved;
};


class LineNumberArea : public QWidget
{

public:
	LineNumberArea(TextEditor * _editor) :
	 QWidget(_editor)
	{
		_codeEditor = _editor;
	}

	QSize sizeHint() const {
		return QSize(_codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent * _event) {
		_codeEditor->lineNumberAreaPaintEvent( _event );
	}

private:
	TextEditor * _codeEditor;
};


#endif // TEXTEDITOR_H
