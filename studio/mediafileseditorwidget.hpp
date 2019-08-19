#ifndef MEDIAFILESWIDGET_H
#define MEDIAFILESWIDGET_H

#include <QDockWidget>
#include "ui_mediafileseditorwidget.h"

//class Project;
class Renderer;
class MediaFile;
class TextEditor;
class MediaFilesEditorWidget : public QDockWidget
{
    Q_OBJECT
    
public:
	explicit MediaFilesEditorWidget(QWidget* _parent = nullptr);

public slots:
	//void loadProject(Project& project);
	//void appendTextEditable(TextEditable* te);

	void appendMediaFile(MediaFile* _newMediaFile);
	//void onMediaFileDestroyed(MediaFile*);
	void saveAllShaders();


signals:
	void rendererChanged(QWeakPointer<Renderer> _renderer);

private slots:
	void on_buildButton_clicked(bool);
	void on_saveButton_clicked(bool);
	void on_tab_currentChanged(int _index);

	void onTextEditorSaved(TextEditor *_e, bool _saved);


private:
		Ui::MediaFilesEditorWidget m_ui;
};

#endif // MEDIAFILESWIDGET_H
