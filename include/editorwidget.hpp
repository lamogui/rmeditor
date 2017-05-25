#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QDockWidget>


namespace Ui {
class EditorWidget;
}

class LogWidget;
class Project;
class Render;
class TextEditable;
class TextEditor;

class EditorWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit EditorWidget(LogWidget& log, QWidget *parent = 0);
    ~EditorWidget();

public slots:
    void loadProject(Project& project);
    void appendTextEditable(TextEditable* te);

    void saveAllShaders();


signals:
    void rendererChanged(Render* renderer);

private slots:
    void on_buildButton_clicked(bool);
    void on_saveButton_clicked(bool);
    void on_tab_currentChanged(int index);

    void onTextEditorSaved(TextEditor *e, bool saved);


private:
    LogWidget* log;
    Ui::EditorWidget *ui;
};

#endif // EDITORWIDGET_H
