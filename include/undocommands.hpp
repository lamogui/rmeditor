#ifndef UNDOCOMMANDS_HPP
#define UNDOCOMMANDS_HPP

#include <QUndoCommand>
#include <QVariant>
#include <QPointer>
#include <QObject>
#include <QMetaMethod>

class ModifyPropertyCommand : public QUndoCommand
{
public:
  ModifyPropertyCommand(QObject& target, const QString& propertyName, const QVariant& previousValue, const QVariant& newValue, QUndoCommand *parent = nullptr);
  
  void redo() override;
  void undo() override;

protected:
  // utils
  void internalDo(const QVariant& value);

  QPointer<QObject> target;
  QString propertyName;
  QVariant previousValue;
  QVariant newValue;

private:
  typedef QUndoCommand BaseClass;
};

template <class TargetClass, typename ValueClass>
class InsertInContainerUndoCommand : public QUndoCommand
{
public:
  typedef void (TargetClass::*Method)(ValueClass& target);

  InsertInContainerUndoCommand(TargetClass& targetOwner, ValueClass& target, Method insertMethod, Method removeMethod, QUndoCommand *parent = nullptr) :
    QUndoCommand(parent),
    targetOwner(&targetOwner), target(&target), insertMethod(insertMethod), removeMethod(removeMethod), ownTarget(true)
  {
    setText("Insert " + target.objectName() + " into " + targetOwner.objectName());
  }

  ~InsertInContainerUndoCommand() override
  {
    if (ownTarget && target)
      delete target.data();
  }

  void redo() override
  {
    if (targetOwner && target)
    {
      targetOwner->*insertMethod(*target);
      ownTarget = false;
    }
    else
    {
      Q_ASSERT(false && "target or target owner has been destroyed !");
    }
    BaseClass::redo();
  }

  void undo() override
  {
    BaseClass::undo();
    if (target && targetOwner)
    {
      targetOwner->*removeMethod(*target);
      ownTarget = true;
    }
    else
    {
      Q_ASSERT(false && "target or target owner has been destroyed !");
    }
  }

protected:
  QPointer<TargetClass> targetOwner;
  QPointer<ValueClass> target;
  Method insertMethod;
  Method removeMethod;
  bool ownTarget;

private:
  typedef QUndoCommand BaseClass;
};

template <class TargetClass, typename ValueClass>
class RemoveFromContainerUndoCommand : public QUndoCommand
{
public:
  typedef void(TargetClass::*Method)(ValueClass& target);

  RemoveFromContainerUndoCommand(TargetClass& targetOwner, ValueClass& target, Method removeMethod, Method insertMethod, QUndoCommand *parent = nullptr) :
    QUndoCommand(parent),
    targetOwner(&targetOwner), target(&target), removeMethod(removeMethod), insertMethod(insertMethod), ownTarget(false)
  {
    setText("Remove " + target.objectName() + " from " + targetOwner.objectName());
  }

  ~RemoveFromContainerUndoCommand() override
  {
    if (ownTarget && target)
      delete target.data();
  }

  void redo() override
  {
    if (targetOwner && target)
    {
      targetOwner->*removeMethod(*target);
      ownTarget = true;
    }
    else
    {
      Q_ASSERT(false && "target or target owner has been destroyed !");
    }
    BaseClass::redo();
  }

  void undo() override
  {
    BaseClass::undo();
    if (target && targetOwner)
    {
      targetOwner->*insertMethod(*target);
      ownTarget = false;
    }
    else
    {
      Q_ASSERT(false && "target or target owner has been destroyed !");
    }
  }

protected:
  QPointer<TargetClass> targetOwner;
  QPointer<ValueClass> target;
  Method removeMethod;
  Method insertMethod;
  bool ownTarget;

private:
  typedef QUndoCommand BaseClass;
};

#define UNDOCOMMANDS_SENDER_OBJECT \
  signals: \
    void sendUndoCommand(QUndoCommand*); \
  private:

#define UNDOCOMMANDS_RECEIVER_OBJECT \
  public: \
    QUndoStack& getUndoStack() { return undoStack; } \
  public slots: \
    void receiveUndoCommand(QUndoCommand* cmd) \
    { \
      undoStack->push(cmd); \
    } \
  protected: \
    QUndoStack undoStack; \
  private:

#define ASSERT_IF_UNIQUE_RECEIVER(S) \
  Q_ASSERT(receivers(SIGNAL(S)) == 1); \
  if (receivers(SIGNAL(S)) == 1)

// Utils
QObject* GetUndoReceiver(QObject& context);
void ConnectToUndoReceiver(QObject& object);

#endif // !UNDOCOMMANDS_HPP