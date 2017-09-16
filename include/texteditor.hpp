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
     TextEditor(TextEditable& textObject, QWidget *parent = 0);

     void lineNumberAreaPaintEvent(QPaintEvent *event);
     int lineNumberAreaWidth();

     TextEditable& getTextObject() const
       {return textObject;}


public slots:
     bool build();
     bool save();
     //void refresh();

signals:
     void saved(TextEditor* editor, bool);

protected:
     void resizeEvent(QResizeEvent *event);

private slots:
     void updateLineNumberAreaWidth(int newBlockCount);
     void highlightCurrentLine();
     void updateLineNumberArea(const QRect &, int);
     void resetTimer();

     void setStartLineNumber(int n);

     void onTextEdited();
     void onDestroyTextEditable(QObject* obj);

private:
     QString colorSyntax(QString in);

     TextEditable& textObject;
     QWidget *lineNumberArea;
     QString filename;
     Highlighter *highlighter;
     int id;
     QTimer *timer;
     int startLineNumber;
     bool isSaved;
 };


 class LineNumberArea : public QWidget
 {

 public:
     LineNumberArea(TextEditor *editor) :
       QWidget(editor)
     {
         codeEditor = editor;
     }

     QSize sizeHint() const {
         return QSize(codeEditor->lineNumberAreaWidth(), 0);
     }

 protected:
     void paintEvent(QPaintEvent *event) {
         codeEditor->lineNumberAreaPaintEvent(event);
     }

 private:
     TextEditor *codeEditor;
 };


#endif // TEXTEDITOR_H
