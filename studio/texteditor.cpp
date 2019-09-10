#include <QtGui>
#include <iostream>
#include "texteditor.hpp"
#include "highlighter.hpp"
#include "project.hpp"
#include "shadercode.hpp"
#include "logmanager.hpp"

TextEditor::TextEditor(QWidget *_parent, TextEditable& _te) :
	QPlainTextEdit(_parent),
	m_object(_te),
  m_timer(new QTimer(this)),
  m_startLineNumber(0),
  m_saved(true)
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, &TextEditor::textChanged, this, &TextEditor::onTextEdited);
	connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
	connect(&m_object, &TextEditable::startLineNumberChanged, this, &TextEditor::setStartLineNumber);
	connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
	connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);
	connect(&m_object, &QObject::destroyed,this,&TextEditor::onDestroyTextEditable,Qt::DirectConnection);
	connect(&m_object,&TextEditable::objectTextChanged, this, &TextEditor::setPlainText);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();

	QFont font("consolas",9);
	const int tabStop = 2;  // 4 characters
	QFontMetrics metrics(font);
	setTabStopWidth(tabStop * metrics.width(' '));
	setFont(font);
	setLineWrapMode(QPlainTextEdit::NoWrap);

	m_highlighter = new Highlighter(document());



	m_timer->setSingleShot(true);
	if (dynamic_cast<GLSLShaderCode*>(&textObject) != nullptr)
	{
		m_highlighter->defineGLSLFragmentShaderRules();
		m_timer->setInterval(1000);  //1 second because we want see our result as fast as possible
		connect(m_timer, SIGNAL(timeout()), this, SLOT(build()));
		connect(this, SIGNAL(textChanged()), this, SLOT(resetTimer()));
	}
	else if (dynamic_cast<Project*>(&textObject) != nullptr)
	{
		m_highlighter->defineXMLRule();
	}

	this->setPlainText(m_object->text());

	setUndoRedoEnabled(true);
}

/*
void TextEditor::refresh()
{
  if (m_object->needTextRefresh())
  {
    this->setPlainText(m_object->text());
  }
}*/

bool TextEditor::save()
{
  build();
  m_saved = m_object->save();
  if (m_saved)
  {
    emit saved(this,m_saved);
  }
  return m_saved;
}

bool TextEditor::build()
{
  return m_object->build(this->toPlainText());
}

int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount() + m_startLineNumber);
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 5 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}



void TextEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}



void TextEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}



void TextEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}



void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::darkGray).darker(300);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}



void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
   // painter.fillRect(event->rect(), Qt::black);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + m_startLineNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void TextEditor::resetTimer()
{
  if (m_timer)
  {
    m_timer->start();
  }
}

void TextEditor::onTextEdited()
{

  if (m_saved)
  {
    m_saved = false;
  }
  emit saved(this,m_saved);

}

void TextEditor::onDestroyTextEditable(QObject* _obj)
{
	passert(Log::Code, this, _obj == &m_object);
  delete this;
}

void TextEditor::setStartLineNumber(int n)
{
  m_startLineNumber = n;
  updateLineNumberAreaWidth(n);
  lineNumberArea->repaint();
}
