#include <QTime>
#include <QString>
#include "logwidget.hpp"

LogWidget::LogWidget(QWidget *parent):
  QTextEdit(parent),
  m_printTime(true)
{
	connect( &(*g_logManager), &Log::Manager::newEntry, this, &LogWidget::handleNewEntry );
}

QString LogWidget::getDate()
{
    return QString("[")+QString::number(QTime::currentTime().hour())+":"+QString::number(QTime::currentTime().minute())+":"+QString::number(QTime::currentTime().second())+QString("] ");
}

void LogWidget::handleNewEntry( Log::Entry _newEntry )
{
	QTextCursor cur = this->textCursor();
	cur.movePosition(QTextCursor::End);
	this->setTextCursor(cur);
	if ( _newEntry.m_type == Log::Type::Info ) {
		this->setTextColor(QColor(0,150,0) ); // info
	} else if ( _newEntry.m_type == Log::Type::Warning ) {
		this->setTextColor( QColor(255,127,39) ); // warning
	} else {
		this->setTextColor( QColor(255,0,0) ); // error
	}
	QString txt;
	if(m_printTime) {
		txt = getDate();
	}
	txt += Log::Entry::typeTag( _newEntry.m_type );
	txt += Log::Entry::categoryTag( _newEntry.m_category );
	if ( _newEntry.m_sender && !_newEntry.m_sender->objectName().isEmpty() ) {
	 txt += QString("[") + _newEntry.m_sender->objectName() + "]";
	}
	txt += " " + _newEntry.m_what;
	this->insertPlainText(txt+"\n");
}
