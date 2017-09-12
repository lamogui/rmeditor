
#include "undocommands.hpp"

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
QObject* GetUndoReceiver(QObject& context)
{
  QObject* object = &context; 
  while (object)
  {
    const QMetaObject* meta = object->metaObject();
    if (meta)
    {
      if (meta->indexOfSlot("receiveUndoCommand(QUndoCommand*)"))
        return object;
    }
    object = object->parent();
  }
  return nullptr;
}

void ConnectToUndoReceiver(QObject& object)
{
  QObject* receiver = GetUndoReceiver(object);
  if (receiver)
    QObject::connect(&object, SIGNAL(sendUndoCommand(QUndoCommand*)), receiver, SLOT(receiveUndoCommand(QUndoCommand*)));
}

