#ifndef LOGWIDGET_HPP
#define LOGWIDGET_HPP

#include <QTextEdit>
#include <QOpenGLDebugMessage>

class LogWidget : public QTextEdit
{
    Q_OBJECT
public:
  LogWidget(QWidget* parent=0);

  //Utils
  static QString getDate();

public slots:
  //Log
  void writeInfo(QString txt);
  void writeWarning(QString txt);
  void writeError(QString txt);

  inline void setPrintTime(bool e) { printTime = e; }

  void handleOpengGLLoggedMessage(const QOpenGLDebugMessage& debugMessage);

signals:

private:
  

  bool printTime;

};

#endif // !LOGWIDGET_HPP
