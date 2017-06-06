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

class AppendInVectorUndoCommand : public QUndoCommand
{
public:
  AppendInVectorUndoCommand(QObject& targetOwner, QObject& target, const QMetaMethod& appendMethod, const QMetaMethod& removeMethod, QUndoCommand *parent = nullptr);
  ~AppendInVectorUndoCommand() override;

  void redo() override;
  void undo() override;

protected:
  QPointer<QObject> targetOwner;
  QPointer<QObject> target;
  QMetaMethod appendMethod;
  QMetaMethod removeMethod;
  bool ownTarget;

private:
  typedef QUndoCommand BaseClass;
};

class RemoveFromVectorUndoCommand : public QUndoCommand
{
public:
  RemoveFromVectorUndoCommand(QObject& targetOwner, QObject& target, const QMetaMethod& removeMethod, const QMetaMethod& appendMethod, QUndoCommand *parent = nullptr);
  ~RemoveFromVectorUndoCommand() override;

  void redo() override;
  void undo() override;

protected:
  QPointer<QObject> targetOwner;
  QPointer<QObject> target;
  QMetaMethod removeMethod;
  QMetaMethod appendMethod;
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
    QUndoStack& getUndoStack() { return undoStack; }
  public slots: \
    void receiveUndoCommand(QUndoCommand* cmd) \
    {
      undoStack->push(cmd);
    }
  protected: \
    QUndoStack undoStack; \
  private:

// Utils
QObject* GetUndoReceiver(QObject& context);

 

#endif // !UNDOCOMMANDS_HPP