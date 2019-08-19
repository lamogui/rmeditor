#ifndef LOGMANAGER_HPP
#define LOGMANAGER_HPP

#include <QObject>
#include <QString>

class QOpenGLDebugMessage;
class LogManager final : public QObject
{
  Q_OBJECT

signals:
  void assertion(QString);
  void error(QString);
  void warning(QString);
  void info(QString);

public slots:
  void handleOpengGLLoggedMessage(const QOpenGLDebugMessage& debugMessage);

private:
	LogManager(); // global non-static
};

namespace Log
{
  void Assertion(const QString& str);
  void Error(const QString& str);
  void Warning(const QString& str);
  void Info(const QString& str);
}

#endif // !LOGMANAGER_HPP
