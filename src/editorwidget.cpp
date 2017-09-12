#include "editorwidget.hpp"

#include "ui_editorwidget.h"

#include <iostream>
#include "jassert.hpp"

#include "logwidget.hpp"
//#include "project.hpp"
#include "scene.hpp"
#include "texteditor.hpp"
#include "texteditable.hpp"

EditorWidget::EditorWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::EditorWidget)
{
  ui->setupUi(this);
}

EditorWidget::~EditorWidget()
{
  delete ui;
}

/*
void EditorWidget::loadProject(Project &project)
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

void EditorWidget::on_saveButton_clicked(bool)
{
  TextEditor* te = dynamic_cast<TextEditor*>(ui->tab->currentWidget());
  if (te != nullptr)
  {
    if (te->save())
    {
      emit info(tr("saved ") + te->textObject()->getPath().fileName());
    }
    else
    {
      emit error(tr("unable to save the file ") + te->textObject()->getPath().fileName());
    }
  }
}

void EditorWidget::on_buildButton_clicked(bool)
{
  TextEditor* te = dynamic_cast<TextEditor*>(ui->tab->currentWidget());
  if (te != nullptr)
  {
    if (te->build())
    {
      emit info(QString("[") + te->textObject()->getPath().fileName() + tr("] build success !"));
    }
    else
    {
      emit error(QString("[") + te->textObject()->getPath().fileName() + tr("] build failure !"));
    }
  }
}

void EditorWidget::on_tab_currentChanged(int index)
{
  if (index < 0)
  {
    return;
  }

  TextEditor* te = dynamic_cast<TextEditor*>(ui->tab->widget(index));
  jassert(te != nullptr);
  //te->refresh();
  ui->buildButton->setEnabled(te->textObject()->buildable());
  emit rendererChanged(te->textObject()->getDefaultRenderer());

}

void EditorWidget::appendTextEditable(TextEditable *te)
{
  jassert(te);
  bool newWidget = true;
  for (int i = 0; i < ui->tab->count(); i++)
  {
    QWidget* widget = ui->tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor)
    {
      if (editor->textObject() == te)
      {
        ui->tab->setTabText(i,te->getPath().fileName());
        //editor->refresh();
        newWidget = false;
        break;
      }
    }
  }
  if (newWidget)
  {
    TextEditor* editor = new TextEditor(*te,ui->tab);
    ui->tab->addTab(editor,te->getPath().fileName());
    //editor->refresh();
    connect(editor, &TextEditor::saved, this, &EditorWidget::onTextEditorSaved);
  }
}

void EditorWidget::saveAllShaders()
{
  QList<TextEditor*> frameworks;
  QList<TextEditor*> scenes;

  for (int i = 0; i < ui->tab->count(); i++)
  {
    QWidget* widget = ui->tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor)
    {
      GLSLShaderCode* shaderCode = dynamic_cast<GLSLShaderCode*>(editor->textObject());

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


void EditorWidget::onTextEditorSaved(TextEditor* e, bool saved)
{
  for (int i = 0; i < ui->tab->count(); i++)
  {
    QWidget* widget = ui->tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor == e)
    {
      if (saved)
      {
        ui->tab->setTabText(i,editor->textObject()->getPath().fileName());
      }
      else
      {
        ui->tab->setTabText(i,editor->textObject()->getPath().fileName() + "*");
      }
      break;
    }
  }
}
