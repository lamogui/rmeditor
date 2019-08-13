#ifndef FFMPEGENCODER_HPP
#define FFMPEGENCODER_HPP

#if defined(_WIN32)
#include <windows.h> // fix windows kit opengl header compile error
#endif
#include <QImage>
#include <QTcpSocket>
#include <QThread>
#include <QProcess>
#include <QProgressBar>

class Project;
class LogWidget;
class Music;

class FFmpegEncoder : public QThread
{
	Q_OBJECT

	public:
		FFmpegEncoder(QObject *_parent,
									const QString& _filename,
									const Project& _timeline,
									const QSize& _resolution,
									LogWidget&  _log);

   virtual void connectLog(LogWidget& log);

	public slots:
		void cancel() { m_cancel = true; }


	protected slots:
		void readStandardError();
		void readStandardOutput();
		void ffmpegFinished(int _exitCode, QProcess::ExitStatus _exitStatus);
		void ffmpegErrorOccurred(QProcess::ProcessError);

	signals:
		void logError(QString _err);
		void logWarning(QString _warn);
		void logInfo(QString _txt);

		void requestRendering(QSize _resolution, QImage* _target);
		void newFrameEncoded(int _frame);

	protected:
		void run() override; // QThread

		QString m_filename;
		QSize m_resolution;
		const Project& m_project;
		volatile bool m_cancel;
		QProcess* m_ffmpeg;
};

#endif
