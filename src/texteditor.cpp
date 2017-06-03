#include <QtGui>
#include <iostream>
#include "texteditor.hpp"
#include "highlighter.hpp"
//#include "project.hpp"
#include "shadercode.hpp"

TextEditor::TextEditor(TextEditable &textObject, QWidget *parent) :
  QPlainTextEdit(parent),
  object(&textObject),
  timer(new QTimer(this)),
  startLineNumber(0),
  isSaved(true)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &TextEditor::textChanged, this, &TextEditor::onTextEdited);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(&textObject, SIGNAL(startLineNumberChanged(int)), this, SLOT(setStartLineNumber(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(object,SIGNAL(destroyed(QObject*)),this,SLOT(onDestroyObject(QObject*)),Qt::DirectConnection);
    connect(object,SIGNAL(objectTextChanged(QString)),SLOT(setPlainText(QString)));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QFont font("consolas",9);
    const int tabStop = 2;  // 4 characters
    QFontMetrics metrics(font);
    setTabStopWidth(tabStop * metrics.width(' '));
    setFont(font);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    highlighter = new Highlighter(document());



    timer->setSingleShot(true);
    if (dynamic_cast<GLSLShaderCode*>(object) != nullptr)
    {
      highlighter->defineGLSLFragmentShaderRules();
      timer->setInterval(1000);  //1 second because we want see our result as fast as possible
      connect(timer, SIGNAL(timeout()), this, SLOT(build()));
      connect(this, SIGNAL(textChanged()), this, SLOT(resetTimer()));
    }
    /*else if (dynamic_cast<Project*>(object) != nullptr)
    {
      highlighter->defineXMLRule();
    }*/

    this->setPlainText(object->getText());

    setUndoRedoEnabled(true);
}

/*
void TextEditor::refresh()
{
  if (object->needTextRefresh())
  {
    this->setPlainText(object->text());
  }
}*/

bool TextEditor::save()
{
  build();
  isSaved = object->save();
  if (isSaved)
  {
    emit saved(this,isSaved);
  }
  return isSaved;
}

bool TextEditor::build()
{
  return object->build(this->toPlainText());
}

int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount() + startLineNumber);
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
            QString number = QString::number(blockNumber + startLineNumber + 1);
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

void TextEditor::onDestroyObject(QObject *obj)
{
  //Q_ASSERT(dynamic_cast<TextEditable*>(obj)!=nullptr);
  delete this;
}

void TextEditor::resetTimer()
{
  if (timer)
  {
    timer->start();
  }
}

void TextEditor::onTextEdited()
{
  if (isSaved)
  {
    isSaved = false;
  }
  emit saved(this,isSaved);
}

void TextEditor::setStartLineNumber(int n)
{
  startLineNumber = n;
  updateLineNumberAreaWidth(n);
  lineNumberArea->repaint();
}
