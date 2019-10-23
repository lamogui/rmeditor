#include "logmanager.hpp"

#include <QOpenGLDebugMessage>

#ifdef Q_OS_LINUX

#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cctype>

bool prout_debuggerIsAttached()
{
	char buf[4096];

	const int status_fd = ::open("/proc/self/status", O_RDONLY);
	if (status_fd == -1)
			return false;

	const ssize_t num_read = ::read(status_fd, buf, sizeof(buf) - 1);
	if (num_read <= 0)
	{
		return false;
	}

	buf[num_read] = '\0';
	constexpr char tracerPidString[] = "TracerPid:";
	const auto tracer_pid_ptr = ::strstr(buf, tracerPidString);
	if (!tracer_pid_ptr) {
		return false;
	}

	for (const char* characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + num_read; ++characterPtr)
	{
		if (::isspace(*characterPtr)) {
			continue;
		}
		else {
			return ::isdigit(*characterPtr) != 0 && *characterPtr != '0';
		}
	}
	return false;
}

#endif

/*
 * CppCodeOrigin
 */

Log::CppCodeOrigin::CppCodeOrigin(const char * _file, unsigned int  _lineNumber) :
	m_file( _file ),
	m_lineNumber( _lineNumber )
{
}

/*
 * Entry
 */

Log::Entry::Entry( Type _type, Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin ) :
	m_type( _type ),
	m_category( _category ),
	m_sender( _sender ),
	m_what( _what ),
	m_param( _param ),
	m_cppOrigin( _cppOrigin )
{
}

Log::Entry Log::Entry::Assertion( Category _category, const QObject* _sender, QString _what, const CppCodeOrigin& _cppOrigin ){
	return Log::Entry( Log::Type::Assertion, _category, _sender, _what, static_cast<qint64>(-1), _cppOrigin );
}
Log::Entry Log::Entry::Error( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin ) {
	return Log::Entry( Log::Type::Error, _category, _sender, _what, _param, _cppOrigin );
}
Log::Entry Log::Entry::Warning( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin ) {
	return Log::Entry( Log::Type::Warning, _category, _sender, _what, _param, _cppOrigin );
}
Log::Entry Log::Entry::Info( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin ) {
	return Log::Entry( Log::Type::Info, _category, _sender, _what, _param, _cppOrigin );
}

/*
 * Manager
 */

Log::Manager g_logManager;

Log::Manager::Manager() {
	connect( this, &Log::Manager::newEntry, this, &Log::Manager::handleNewEntry );
}

void Log::Manager::Assertion( Category _category, const QObject* _sender, QString _what, const CppCodeOrigin& _cppOrigin ) {
	emit g_logManager.newEntry( Log::Entry::Assertion(_category, _sender, _what, _cppOrigin) );
}

void Log::Manager::Error( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin ) {
	emit g_logManager.newEntry( Log::Entry::Error(_category, _sender, _what, _param, _cppOrigin) );
}

void Log::Manager::Warning( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin ) {
	emit g_logManager.newEntry( Log::Entry::Warning(_category, _sender, _what, _param, _cppOrigin) );
}

void Log::Manager::Info( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin ) {
	emit g_logManager.newEntry( Log::Entry::Info(_category, _sender, _what, _param, _cppOrigin) );
}

void Log::Manager::handleOpengGLLoggedMessage(const QOpenGLDebugMessage& debugMessage)
{
	if (debugMessage.type() & (QOpenGLDebugMessage::ErrorType)) {
		perror(Log::Category::OpenGL, nullptr, debugMessage.message());
	}
  else if (debugMessage.type() &
    (QOpenGLDebugMessage::DeprecatedBehaviorType | QOpenGLDebugMessage::UndefinedBehaviorType |
		 QOpenGLDebugMessage::PortabilityType | QOpenGLDebugMessage::PerformanceType | QOpenGLDebugMessage::InvalidType)) {
		pwarning(Log::Category::OpenGL, nullptr, debugMessage.message());
	}
	else {
		pinfo(Log::Category::OpenGL, nullptr, debugMessage.message());
	}
}

void Log::Manager::handleNewEntry( Entry _entry )
{
	QString type;
	switch (_entry.m_type) {
		case Log::Type::Assertion:
			type = "[ASSERT]";
			break;
		case Log::Type::Error:
			type = "[ERROR]";
			break;
		case Log::Type::Warning:
			type = "[WARN]";
			break;
		case Log::Type::Info:
			type = "[INFO]";
			break;
	}

	QString category;
	switch ( _entry.m_category ){
		case File:
			category = "[FILE]";
			break;
		case System:
			category = "[SYSTEM]";
			break;
		case Code:
			category = "[CODE]";
			break;
		case Shader:
			category = "[SHADER]";
			break;
		case OpenGL:
			category = "[OPENGL]";
			break;
		case Audio:
			category = "[AUDIO]";
			break;
		case Undefined:
			break;
	}

	m_entries.push_back( _entry );

	QString name = _entry.m_sender ? QString("[") + _entry.m_sender->objectName() + "]" : "";
	qDebug() << type << category << name << _entry.m_what;
}

bool pVerify( Log::Category _category, const QObject* _sender, bool _cond, const char* _condStr, const Log::CppCodeOrigin& _cppOrigin )
{
	if ( !_cond ) {
		if ( PROUT_IS_DEBUGGER_PRESENT ) {
			PROUT_DEBUG_BREAK;
		}
		Log::Manager::Assertion( _category, _sender, QString("VERIFY FAILED !") + " (" + QString(_condStr) + ")", _cppOrigin );
	}
	return _cond;
}

