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
// TextEditor is a MediaFileEditor for TextEditable
// Note: editors depends on their target heavily so there are destroyed when target is destroyed 
class TextEditor : public QPlainTextEdit
{
     Q_OBJECT

 public:
	TextEditor(QWidget *_parent, TextEditable& _textObject);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

	const TextEditable& getTextObject() const
	 {return m_object;}


public slots:
	bool build();
	bool save();
	//void refresh();

signals:
	void saved(TextEditor* _editor, bool);

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
	void updateLineNumberAreaWidth(int _newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &, int);
	void resetTimer();

	void setStartLineNumber(int _n);

	void onTextEdited();
	void onDestroyTextEditable(QObject* _obj);

private:
	QString colorSyntax(QString in);

	TextEditable& m_object;
	QWidget* m_lineNumberArea;
	QString m_filename;
	Highlighter *m_highlighter;
	int m_id;
	QTimer* m_timer;
	int m_startLineNumber;
	bool m_saved;
 };


class LineNumberArea : public QWidget
{

public:
	LineNumberArea(TextEditor* _editor) :
	 QWidget(_editor)
	{
		 m_codeEditor = _editor;
	}

	QSize sizeHint() const {
		 return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent* _event) {
		m_codeEditor->lineNumberAreaPaintEvent(_event);
	}

private:
	TextEditor* m_codeEditor;
};


#endif // TEXTEDITOR_H
