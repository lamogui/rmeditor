#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QDockWidget>
#include "ui_editorwidget.h"

class Project;
class Renderer;
class TextEditable;
class TextEditor;

class EditorWidget : public QDockWidget
{
	Q_OBJECT
    
public:
	explicit EditorWidget(QWidget * _parent = nullptr);

public slots:
	void loadProject(Project& project);
	void appendTextEditable(TextEditable* te);

	void saveAllShaders();


signals:
	void rendererChanged(Renderer* renderer);

private slots:
	void on_buildButton_clicked(bool);
	void on_saveButton_clicked(bool);
	void on_tab_currentChanged(int index);

	void onTextEditorSaved(TextEditor *e, bool saved);


private:
	Ui::EditorWidget m_ui;
};

#endif // EDITORWIDGET_H
