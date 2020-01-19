#include "editorwidget.hpp"
#include "ui_editorwidget.h"

#include <iostream>

#include "framework.hpp"
#include "logmanager.hpp"
#include "project.hpp"
#include "scene.hpp"
#include "texteditor.hpp"
#include "texteditable.hpp"
#include "styles.hpp"

EditorWidget::EditorWidget(QWidget * _parent) :
		QDockWidget(_parent)
{
		m_ui.setupUi(this);
		SetupDockWidgetStyle( *this );
		m_ui.tab->setStyleSheet("background: transparent;");
		SetupPushButtonWidgetStyle( *m_ui.saveButton );
		SetupPushButtonWidgetStyle( *m_ui.buildButton );
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
	TextEditor* te = dynamic_cast<TextEditor*>(m_ui.tab->currentWidget());
	if (te != nullptr)
	{
		if (te->save())
		{
			pinfo( Log::File, te->textObject(), tr("saved") );
		}
		else
		{
			perror( Log::File, te->textObject(), tr("unable to save the file") );
		}
	}
}

void EditorWidget::on_buildButton_clicked(bool)
{
	TextEditor* te = dynamic_cast<TextEditor*>(m_ui.tab->currentWidget());
	if (te != nullptr)
	{
		if (te->build())
		{
			pinfo( Log::File, te->textObject(), tr("build success !") );
		}
		else
		{
			perror( Log::File, te->textObject(), tr("build failure !") );
		}
	}
}

void EditorWidget::on_tab_currentChanged(int _index)
{
	if (_index < 0)
	{
		return;
	}

	TextEditor* te = dynamic_cast<TextEditor*>(m_ui.tab->widget(_index));
	passert( Log::Code, te != nullptr );
	//te->refresh();
	m_ui.buildButton->setEnabled(te->textObject()->buildable());
	emit rendererChanged(te->textObject()->getRenderer());
}

void EditorWidget::appendTextEditable(TextEditable *te)
{
	passert( Log::Code, te );
  bool newWidget = true;
	for (int i = 0; i < m_ui.tab->count(); i++)
  {
		QWidget* widget = m_ui.tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor)
    {
      if (editor->textObject() == te)
      {
				m_ui.tab->setItemText(i,te->fileName());
        //editor->refresh();
        newWidget = false;
        break;
      }
    }
  }
  if (newWidget)
  {
		TextEditor* editor = new TextEditor(*te,m_ui.tab);

		m_ui.tab->addItem(editor,te->fileName());//m_ui.tab->addTab(editor,te->fileName());
    //editor->refresh();
    connect(editor,SIGNAL(saved(TextEditor*,bool)),this,SLOT(onTextEditorSaved(TextEditor*,bool)));
  }
}

void EditorWidget::saveAllShaders()
{
  QList<TextEditor*> frameworks;
  QList<TextEditor*> scenes;

	for (int i = 0; i < m_ui.tab->count(); i++)
  {
		QWidget* widget = m_ui.tab->widget(i);
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
	for (int i = 0; i < m_ui.tab->count(); i++)
  {
		QWidget* widget = m_ui.tab->widget(i);
    TextEditor* editor = dynamic_cast<TextEditor*>(widget);
    if (editor == e)
    {
      if (saved)
      {
				m_ui.tab->setItemText(i,editor->textObject()->fileName());
      }
      else
      {
				m_ui.tab->setItemText(i,editor->textObject()->fileName() + "*");
      }
      break;
    }
  }
}
