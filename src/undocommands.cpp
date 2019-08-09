
#include "undocommands.hpp"
#include "project.hpp"

/*
** ModifyPropertyCommand
*/

ModifyPropertyCommand::ModifyPropertyCommand(QObject& target, const QString& propertyName, const QVariant& previousValue, const QVariant& newValue, QUndoCommand *parent):
  QUndoCommand(parent),
  target(&target), propertyName(propertyName), previousValue(previousValue), newValue(newValue)
{
  setText("Set " + propertyName + " of " + target.objectName() + " to " + newValue.toString());
}

void ModifyPropertyCommand::redo()
{
  internalDo(newValue);
  BaseClass::redo();
}

void ModifyPropertyCommand::undo()
{
  BaseClass::redo();
  internalDo(previousValue);
}

void ModifyPropertyCommand::internalDo(const QVariant& value)
{
  if (target)
  {
    if (!target->setProperty(propertyName.toStdString().c_str(), value))
      jassert(false && "invalid property for target");
  }
  else
  {
    jassert(false && "target has be destroyed !");
  }
}

/*
** Utils
*/

namespace UndoStack
{
  QObject* getOwner(QObject& context)
  {
    return Project::get(context);
  }

  QUndoStack* get(QObject& context)
  {
    Project* project = (Project*)getOwner(context);
    if (project)
    {
      return &project->getUndoStack();
    }
    return nullptr;
  }

  bool sendUndoCommand(QObject& context, QUndoCommand* undoCommand)
  {
    QUndoStack* stack = get(context);
    if (!stack)
    {
      return false;
    }
    stack->push(undoCommand);
    return true;
  }


  void assertSendUndoCommand(QObject& context, QUndoCommand* undoCommand)
  {
    jassert(undoCommand);
    bool res = sendUndoCommand(context, undoCommand);
    jassert(res);
  }
}
