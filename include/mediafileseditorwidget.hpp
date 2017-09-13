#ifndef MEDIAFILESWIDGET_H
#define MEDIAFILESWIDGET_H

#include <QDockWidget>


namespace Ui {
class MediaFilesEditorWidget;
}

//class Project;
class Renderer;
class MediaFile;
class TextEditor;
class MediaFilesEditorWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit MediaFilesEditorWidget(QWidget *parent = 0);
    ~MediaFilesEditorWidget();

public slots:
    //void loadProject(Project& project);
    //void appendTextEditable(TextEditable* te);
    
    void appendMediaFile(MediaFile* newMediaFile);
    void onMediaFileDestroyed(MediaFile*);
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
    Ui::MediaFilesEditorWidget *ui;
};

#endif // MEDIAFILESWIDGET_H
