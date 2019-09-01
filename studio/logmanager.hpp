#ifndef LOGMANAGER_HPP
#define LOGMANAGER_HPP

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

#define PROUT_BLOCK_WITH_FORCED_SEMICOLON(x) do { x } while (false)
#ifdef _MSC_VER
#include <Windows.h>

#ifndef __INTEL_COMPILER
#pragma intrinsic (__debugbreak)
#endif

#define PROUT_IS_DEBUGGER_PRESENT IsDebuggerPresent()
#define PROUT_DEBUG_BREAK __debugbreak()

#else  // TODO !

#define PROUT_IS_DEBUGGER_PRESENT
#define PROUT_DEBUG_BREAK

#endif

class QOpenGLDebugMessage;

namespace Log {

	enum Type {
		Info,
		Warning,
		Error,
		Assertion
	};

	enum Category {
		Undefined,
		Shader,
		OpenGL
	};

	struct CppCodeOrigin {
		QString m_file;
		unsigned int m_lineNumber;

		CppCodeOrigin() {} // DO NOT USE !
		CppCodeOrigin(const char * _file, unsigned int  _lineNumber);
	};

	struct Entry {
		Type m_type;
		Category m_category;
		QPointer<const QObject> m_sender;
		QString m_what;
		qint64 m_param; // line number for shader codes / text medias
		CppCodeOrigin m_cppOrigin;

		static Entry Assertion( Category _category, const QObject* _sender, QString _what, const CppCodeOrigin& _cppOrigin );
		static Entry Error( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin );
		static Entry Warning( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin );
		static Entry Info( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin );

		Entry() {} // DO NOT CALL ! just used by QVector

	private:
		Entry( Type _type, Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin );
	};

	class Manager : public QObject {
		Q_OBJECT

	signals:
		void newEntry( Entry _entry );

	public:
		Manager();

		static void Assertion( Category _category, const QObject* _sender, QString _what, const CppCodeOrigin& _cppOrigin );
		static void Error( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin );
		static void Warning( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin );
		static void Info( Category _category, const QObject* _sender, QString _what, qint64 _param, const CppCodeOrigin& _cppOrigin );

	public slots:
		void handleOpengGLLoggedMessage(const QOpenGLDebugMessage& debugMessage);

	protected slots:
		void handleNewEntry( Entry _entry );

	protected:
		QVector<Entry> m_entries;

	};
}

extern Log::Manager g_logManager; // global instance

#define PROUT_MAKE_CPP_ORIGIN() Log::CppCodeOrigin( __FILE__, __LINE__ )

// Asserts
#define passertmsg( _category, _sender, _cond, _what) \
	PROUT_BLOCK_WITH_FORCED_SEMICOLON( \
	if ( !_cond ) { \
		if (PROUT_DEBUG_BREAK) { \
			PROUT_DEBUG_BREAK;\
		} \
		Log::Manager::Assertion( _category, _sender, QString( _what ) + " (" + QString(#_cond) + ")", PROUT_MAKE_CPP_ORIGIN()); \
	} )
#define passertmsgf( _category, _sender, _cond, _what, ... ) passertmsg( _category, _sender, _cond, QString::asprinf( _what, __VA_ARGS__ ) )
#define passert( _category, _sender, _cond ) passertmsg( _category, _cond, tr("ASSERTION FAILED !") )

// Errors
#define perrorp( _category, _sender, _param, _what ) PROUT_BLOCK_WITH_FORCED_SEMICOLON( Log::Manager::Error( _category, _sender, QString( _what ), _param, PROUT_MAKE_CPP_ORIGIN()); )
#define perrorpf( _category, _sender, _param, _what, ... ) perrorp( _category, _sender, _param, QString::asprinf( _what, __VA_ARGS__ ) )
#define perror( _category, _sender, _what ) perrorp( _category, _sender, -1, _what )
#define perrorf( _category, _sender, _what, ... ) perror( _category, _sender, QString::asprinf( _what, __VA_ARGS__ ) )

// Warning
#define pwarningp( _category, _sender, _param, _what ) PROUT_BLOCK_WITH_FORCED_SEMICOLON( Log::Manager::Warning( _category, _sender, QString( _what ), _param, PROUT_MAKE_CPP_ORIGIN()); )
#define pwarningpf( _category, _sender, _param, _what, ... ) pwarningp( _category, _sender, _param, QString::asprinf( _what, __VA_ARGS__ ) )
#define pwarning( _category, _sender, _what ) pwarningp( _category, _sender, -1, _what )
#define pwarningf( _category, _sender, _what, ... ) pwarning( _category, _sender, QString::asprinf( _what, __VA_ARGS__ ) )

// Info
#define pinfop( _category, _sender, _param, _what ) PROUT_BLOCK_WITH_FORCED_SEMICOLON( Log::Manager::Info( _category, _sender, QString( _what ), _param, PROUT_MAKE_CPP_ORIGIN()); )
#define pinfopf( _category, _sender, _param, _what, ... ) pinfop( _category, _sender, _param, QString::asprinf( _what, __VA_ARGS__ ) )
#define pinfo( _category, _sender, _what ) pinfop( _category, _sender, -1, _what )
#define pinfof( _category, _sender, _what, ... ) pinfo( _category, _sender, QString::asprinf( _what, __VA_ARGS__ ) )

#endif // !LOGMANAGER_HPP
