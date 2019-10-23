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

class DemoTimeline;
class Music;
class FFmpegEncoder : public QThread
{
	Q_OBJECT

public:
	FFmpegEncoder(QObject *_parent,
							const QString& m_filename,
							DemoTimeline& m_timeline,
							const QSize& m_resolution);

public slots:
	void cancel() { m_cancel = true; }

protected slots:
	void readStandardError();
	void readStandardOutput();
	void ffmpegFinished(int exitCode, QProcess::ExitStatus _exitStatus);
	void ffmpegErrorOccurred(QProcess::ProcessError);

signals:
	void requestRendering(QSize _resolution, QImage* _target);
	void newFrameEncoded(int _frame);

protected:
	virtual void run();

	QString m_filename;
	QSize m_resolution;
	DemoTimeline* m_timeline;
	volatile bool m_cancel;
	QProcess* m_ffmpeg;
};

#endif
