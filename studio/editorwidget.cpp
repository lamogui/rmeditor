#include "editorwidget.hpp"
#include "ui_editorwidget.h"

#include <iostream>

#include "framework.hpp"
#include "logwidget.hpp"
#include "project.hpp"
#include "scene.hpp"
#include "texteditor.hpp"
#include "texteditable.hpp"

EditorWidget::EditorWidget(LogWidget &log, QWidget *parent) :
    QDockWidget(parent),
    m_log(&log),
    ui(new Ui::EditorWidget)
{
    ui->setupUi(this);
}

EditorWidget::~EditorWidget()
{
    delete ui;
}

void EditorWidget::loadProject(Project &project)
{
  appendTextEditable(&project);
  foreach (QString key, project.rayMarchScenes().keys())
  {
    TextEditable* te = project.getRayMarchScene(key);
    appendTextEditable(te);
  }
  foreach (QString key, project.frameworks().keys())
  {
    TextEditable* te = project.getFramework(key);
    appendTextEditable(te);
  }
}

void EditorWidget::on_saveButton_clicked(bool)
{
  TextEditor* te = dynamic_cast<TextEditor*>(ui->tab->currentWidget());
  if (te != nullptr)
  {
    if (te->save())
    {
      m_log->writeInfo(tr("saved ") + te->textObject()->fileName());
    }
    else
    {
     m_log->writeError(tr("unable to save the file ") + te->textObject()->fileName());
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
      m_log->writeInfo(QString("[") + te->textObject()->fileName() + tr("] build success !"));
    }
    else
    {
      m_log->writeError(QString("[") + te->textObject()->fileName() + tr("] build failure !"));
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
  Q_ASSERT(te != nullptr);
  //te->refresh();
  ui->buildButton->setEnabled(te->textObject()->buildable());
  emit rendererChanged(te->textObject()->getRenderer());

}

void EditorWidget::appendTextEditable(TextEditable *te)
{
  Q_ASSERT(te);
  bool newWidget = true;
  for (int i = 0; i < ui->tab->count(); i++)
  {
    QWidget* widget = ui->tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor)
    {
      if (editor->textObject() == te)
      {
        ui->tab->setTabText(i,te->fileName());
        //editor->refresh();
        newWidget = false;
        break;
      }
    }
  }
  if (newWidget)
  {
    TextEditor* editor = new TextEditor(*te,ui->tab);
    ui->tab->addTab(editor,te->fileName());
    //editor->refresh();
    connect(editor,SIGNAL(saved(TextEditor*,bool)),this,SLOT(onTextEditorSaved(TextEditor*,bool)));
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
      FragmentShaderCode* shaderCode = dynamic_cast<FragmentShaderCode*>(editor->textObject());

      if (shaderCode)
      {
        if (dynamic_cast<Framework*>(shaderCode) != nullptr)
        {
          frameworks.append(editor);
        }
        else if (dynamic_cast<Scene*>(shaderCode) != nullptr)
        {
          scenes.append(editor);
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
        ui->tab->setTabText(i,editor->textObject()->fileName());
      }
      else
      {
        ui->tab->setTabText(i,editor->textObject()->fileName() + "*");
      }
      break;
    }
  }
}
