#include "logmanager.hpp"

#include <QOpenGLDebugMessage>

LogManager::LogManager()
{
}

void LogManager::handleOpengGLLoggedMessage(const QOpenGLDebugMessage& debugMessage)
{
  if (debugMessage.type() & (QOpenGLDebugMessage::ErrorType))
    Log::Error("[OpenGL] " + debugMessage.message());
  else if (debugMessage.type() &
    (QOpenGLDebugMessage::DeprecatedBehaviorType | QOpenGLDebugMessage::UndefinedBehaviorType |
      QOpenGLDebugMessage::PortabilityType | QOpenGLDebugMessage::PerformanceType | QOpenGLDebugMessage::InvalidType))
    Log::Warning("[OpenGL] " + debugMessage.message());
  else
    Log::Info("[OpenGL] " + debugMessage.message());
}

namespace Log
{
  void Assertion(const QString& str)
  {
    qDebug() << "[Assertion Error]" << str.toStdString().c_str();
		emit g_logManager->assertion("[Assertion Error] " + str);
  }
  void Error(const QString& str)
  {
    qDebug() << "[Error]" << str.toStdString().c_str();
		emit g_logManager->error("[Error] " + str);
  }
  void Warning(const QString& str)
  {
    qDebug() << "[Warning]" << str.toStdString().c_str();
    emit LogManager::get()->warning("[Warning] " + str);
  }
  void Info(const QString& str)
  {
    qDebug() << "[Info]" << str.toStdString().c_str();
    emit LogManager::get()->info("[Info] " + str);
  }
}

