#ifndef LOGWIDGET_HPP
#define LOGWIDGET_HPP

#include <QTextEdit>
#include "logmanager.hpp"

class LogWidget final : public QTextEdit
{
    Q_OBJECT
public:
  LogWidget(QWidget* parent=0);

  //Utils
  static QString getDate();

public slots:
	inline void setPrintTime(bool e) { m_printTime = e; }

private slots:
	void handleNewEntry( Log::Entry _newEntry );

private:
  bool m_printTime;

};

#endif // DATA_H
