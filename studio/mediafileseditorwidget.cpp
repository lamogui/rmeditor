#include "mediafileseditorwidget.hpp"


#include <iostream>

// essentials
#include "logmanager.hpp"

#include "scene.hpp" // Should be temp 
#include "texteditor.hpp"  // temp
#include "texteditable.hpp" // temp

MediaFilesEditorWidget::MediaFilesEditorWidget(QWidget *_parent) :
	QDockWidget(_parent),
{
  m_ui.setupUi(this);
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

void MediaFilesEditorWidget::appendMediaFile(MediaFile* _newMediaFile)
{
	jassert(_newMediaFile);
  //TODO : support all kind of editors
	TextEditable* te = qobject_cast<TextEditable*>(_newMediaFile);
  if (te)
  {
    //connect(newMediaFile, &QObject::destroyed, this, &MediaFilesEditorWidget::onMediaFileDestroyed);
    TextEditor* editor = new TextEditor(*te, this);
    m_ui.tab->addTab(editor, te->getPath().fileName());
  }
  else
    jassertfalse; // todo
}

void MediaFilesEditorWidget::on_saveButton_clicked(bool)
{
  TextEditor* te = dynamic_cast<TextEditor*>(m_ui.tab->currentWidget());
  if (te != nullptr)
  {
    if (te->save())
    {
      Log::Info(tr("saved ") + te->getTextObject().getPath().fileName());
    }
    else
    {
      Log::Error(tr("unable to save the file ") + te->getTextObject().getPath().fileName());
    }
  }
}

void MediaFilesEditorWidget::on_buildButton_clicked(bool)
{
  TextEditor* te = dynamic_cast<TextEditor*>(m_ui.tab->currentWidget());
  if (te != nullptr)
  {
    if (te->build())
    {
      Log::Info(QString("[") + te->getTextObject().getPath().fileName() + tr("] build success !"));
    }
    else
    {
      Log::Error(QString("[") + te->getTextObject().getPath().fileName() + tr("] build failure !"));
    }
  }
}

void MediaFilesEditorWidget::on_tab_currentChanged(int index)
{
  if (index < 0)
  {
    return;
  }

  TextEditor* te = dynamic_cast<TextEditor*>(m_ui.tab->widget(index));
  jassert(te != nullptr);
  //te->refresh();
  m_ui.buildButton->setEnabled(te->getTextObject().buildable());
  emit rendererChanged(te->getTextObject().getDefaultRenderer());

}

void MediaFilesEditorWidget::saveAllShaders()
{
  QList<TextEditor*> frameworks;
  QList<TextEditor*> scenes;

  for (int i = 0; i < m_ui.tab->count(); i++)
  {
    QWidget* widget = m_ui.tab->widget(i);
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
  for (int i = 0; i < m_ui.tab->count(); i++)
  {
    QWidget* widget = m_ui.tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor == e)
    {
      if (saved)
      {
        m_ui.tab->setTabText(i, editor->getTextObject().getPath().fileName());
      }
      else
      {
        m_ui.tab->setTabText(i, editor->getTextObject().getPath().fileName() + "*");
      }
      break;
    }
  }
}
