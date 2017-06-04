#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QDockWidget>


namespace Ui {
class EditorWidget;
}

//class Project;
class Renderer;
class TextEditable;
class TextEditor;
class EditorWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit EditorWidget(QWidget *parent = 0);
    ~EditorWidget();

public slots:
    //void loadProject(Project& project);
    void appendTextEditable(TextEditable* te);

    void saveAllShaders();


signals:
    void rendererChanged(QWeakPointer<Renderer> renderer);

    // Logs 
    void error(QString);
    void info(QString);

private slots:
    void on_buildButton_clicked(bool);
    void on_saveButton_clicked(bool);
    void on_tab_currentChanged(int index);

    void onTextEditorSaved(TextEditor *e, bool saved);


private:
    Ui::EditorWidget *ui;
};

#endif // EDITORWIDGET_H
