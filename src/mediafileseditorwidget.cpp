#include "mediafileseditorwidget.hpp"

#include "ui_mediafileseditorwidget.h"

#include <iostream>
#include "jassert.hpp"

#include "logwidget.hpp"
//#include "project.hpp"
#include "scene.hpp"
#include "texteditor.hpp"
#include "texteditable.hpp"

MediaFilesEditorWidget::MediaFilesEditorWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::MediaFilesEditorWidget)
{
  ui->setupUi(this);
}

MediaFilesEditorWidget::~MediaFilesEditorWidget()
{
  delete ui;
}

/*
void MediaFilesEditorWidget::loadProject(Project &project)
{
  appendTextEditable(&project);
  foreach (QString key, project.rayMarchScenes().keys())
  {
    TextEditable* te = project.getRayMarchScene(key);
    appendTextEditable(te);
  }
  foreach (QString key, project.getFrameworks().keys())
  {
    TextEditable* te = project.getFramework(key);
    appendTextEditable(te);
  }
}
*/

void MediaFilesEditorWidget::appendMediaFile(MediaFile* newMediaFile)
{
  jassert(newMediaFile);
  //TODO : support all kind of editors
  TextEditable* te = qobject_cast<TextEditable*>(newMediaFile);
  if (te)
  {
    //connect(newMediaFile, &QObject::destroyed, this, &MediaFilesEditorWidget::onMediaFileDestroyed);
    TextEditor* editor = new TextEditor(*te, this);
    ui->tab->addTab(editor, te->getPath().fileName());
  }
  else
    jassertfalse; // todo
}

void MediaFilesEditorWidget::on_saveButton_clicked(bool)
{
  TextEditor* te = dynamic_cast<TextEditor*>(ui->tab->currentWidget());
  if (te != nullptr)
  {
    if (te->save())
    {
      emit info(tr("saved ") + te->getTextObject().getPath().fileName());
    }
    else
    {
      emit error(tr("unable to save the file ") + te->getTextObject().getPath().fileName());
    }
  }
}

void MediaFilesEditorWidget::on_buildButton_clicked(bool)
{
  TextEditor* te = dynamic_cast<TextEditor*>(ui->tab->currentWidget());
  if (te != nullptr)
  {
    if (te->build())
    {
      emit info(QString("[") + te->getTextObject().getPath().fileName() + tr("] build success !"));
    }
    else
    {
      emit error(QString("[") + te->getTextObject().getPath().fileName() + tr("] build failure !"));
    }
  }
}

void MediaFilesEditorWidget::on_tab_currentChanged(int index)
{
  if (index < 0)
  {
    return;
  }

  TextEditor* te = dynamic_cast<TextEditor*>(ui->tab->widget(index));
  jassert(te != nullptr);
  //te->refresh();
  ui->buildButton->setEnabled(te->getTextObject().buildable());
  emit rendererChanged(te->getTextObject().getDefaultRenderer());

}

void MediaFilesEditorWidget::saveAllShaders()
{
  QList<TextEditor*> frameworks;
  QList<TextEditor*> scenes;

  for (int i = 0; i < ui->tab->count(); i++)
  {
    QWidget* widget = ui->tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor)
    {
      GLSLShaderCode* shaderCode = dynamic_cast<GLSLShaderCode*>(&editor->getTextObject());

      if (shaderCode)
      {
        if (dynamic_cast<RaymarchingScene*>(shaderCode) != nullptr)
        {
          scenes.append(editor);
        }
        else
        {
          frameworks.append(editor);
        }
      }
    }
  }

  //Save framework before becauses scenes depends on it
  foreach (TextEditor* e, frameworks)
  {
    e->save();
  }
  foreach (TextEditor* e,scenes)
  {
    e->save();
  }
}


void MediaFilesEditorWidget::onTextEditorSaved(TextEditor* e, bool saved)
{
  for (int i = 0; i < ui->tab->count(); i++)
  {
    QWidget* widget = ui->tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor == e)
    {
      if (saved)
      {
        ui->tab->setTabText(i, editor->getTextObject().getPath().fileName());
      }
      else
      {
        ui->tab->setTabText(i, editor->getTextObject().getPath().fileName() + "*");
      }
      break;
    }
  }
}
