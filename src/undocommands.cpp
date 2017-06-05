
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
  BaseClass::redo();
  internalDo(newValue);
}

void ModifyPropertyCommand::undo()
{
  BaseClass::undo();
  internalDo(previousValue);
}

void ModifyPropertyCommand::internalDo(const QVariant& value)
{
  if (target)
  {
    if (!target->setProperty(propertyName.toStdString().c_str(), value))
      Q_ASSERT(false && "invalid property for target");
  }
  else
  {
    Q_ASSERT(false && "target has be destroyed !");
  }
}

/*
** AppendInVectorUndoCommand
*/

AppendInVectorUndoCommand::AppendInVectorUndoCommand(QObject& targetOwner, QObject& target, const QMetaMethod& appendMethod, const QMetaMethod& removeMethod, QUndoCommand *parent) :
  QUndoCommand(parent),
  targetOwner(&targetOwner), target(&target), appendMethod(appendMethod), removeMethod(removeMethod), ownTarget(true)
{

}

AppendInVectorUndoCommand::~AppendInVectorUndoCommand()
{
  if (ownTarget)
    delete target.data();
}

void AppendInVectorUndoCommand::redo()
{
  BaseClass::redo();
  Q_ASSERT(ownTarget && targetOwner && target);
  if (ownTarget && targetOwner && target)
  {
    bool res = appendMethod.invoke(targetOwner, Q_ARG(QObject*, target.data()));
    Q_ASSERT(res);
    ownTarget = false;
  }
}

void AppendInVectorUndoCommand::undo()
{
  BaseClass::undo();
  Q_ASSERT(!ownTarget && targetOwner && target);
  if (ownTarget && targetOwner && target)
  {
    bool res = removeMethod.invoke(targetOwner, Q_ARG(QObject*, target.data()));
    Q_ASSERT(res);
    ownTarget = true;
  }
}

/*
** RemoveFromVectorUndoCommand
*/

RemoveFromVectorUndoCommand::RemoveFromVectorUndoCommand(QObject& targetOwner, QObject& target, const QMetaMethod& removeMethod, const QMetaMethod& appendMethod, QUndoCommand *parent) :
QUndoCommand(parent),
targetOwner(&targetOwner), target(&target), removeMethod(removeMethod), appendMethod(appendMethod), ownTarget(false)
{

}

RemoveFromVectorUndoCommand::~RemoveFromVectorUndoCommand()
{
  if (ownTarget)
    delete target.data();
}

void RemoveFromVectorUndoCommand::redo()
{
  BaseClass::redo();
  Q_ASSERT(!ownTarget && targetOwner && target);
  if (ownTarget && targetOwner && target)
  {
    bool res = removeMethod.invoke(targetOwner, Q_ARG(QObject*, target.data()));
    Q_ASSERT(res);
    ownTarget = true;
  }
}

void RemoveFromVectorUndoCommand::undo()
{
  BaseClass::undo();
  Q_ASSERT(ownTarget && targetOwner && target);
  if (ownTarget && targetOwner && target)
  {
    bool res = appendMethod.invoke(targetOwner, Q_ARG(QObject*, target.data()));
    Q_ASSERT(res);
    ownTarget = false;
  }
}
