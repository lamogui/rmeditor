#include "ffmpegencoder.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
//This shall be replaced by signal because it can't be used in thread
//#include <QMessageBox>
#include <QOpenGLFunctions>
#include <QTcpSocket>


#include "demotimeline.hpp"
#include "logmanager.hpp"
#include "music.hpp"
#include "renderer.hpp"


FFmpegEncoder::FFmpegEncoder(QObject* _parent,
														 const QString & _filename,
														 DemoTimeline & _timeline, const QSize & _resolution) :
	QThread(_parent),
	m_filename(_filename),
	m_resolution(_resolution),
	m_timeline(&_timeline),
	m_cancel(false)
{
}

void FFmpegEncoder::readStandardError()
{
  //FFMPEG Seems to send all in STDERR maybe because STDOUT is used for send video/audio in some cases ?
	QString msg = QString(m_ffmpeg->readAllStandardError());
  if (!msg.isEmpty())
  {
		pwarning( Log::FFMpeg, this, msg );
  }
}

void FFmpegEncoder::readStandardOutput()
{
	pinfo( Log::FFMpeg, this, QString(m_ffmpeg->readAllStandardOutput() ) );
}

void FFmpegEncoder::ffmpegFinished(int _exitCode, QProcess::ExitStatus _exitStatus)
{
	if (_exitCode == 0 && _exitStatus == QProcess::NormalExit)
	{
		perror( Log::FFMpeg, this, tr( "ffmpeg exited with code: ") + QString::number(_exitCode) );
	}
	else
	{
		m_cancel = true;
		if (_exitStatus == QProcess::NormalExit)
		{
			perror( Log::FFMpeg, this, tr("ffmpeg exited with code: ") + QString::number(_exitCode));
		}
		else
		{
			perror( Log::FFMpeg, this, tr("ffmpeg crashed !") );
		}
	}
}

void FFmpegEncoder::ffmpegErrorOccurred(QProcess::ProcessError)
{
	perror( Log::FFMpeg, this, m_ffmpeg->errorString() );
	m_cancel = true;
}

void FFmpegEncoder::run()
{
	m_ffmpeg = new QProcess();

	//Use signal/slot in queue because of opengl context
	connect(this,SIGNAL(requestRendering(QSize,QImage*)),m_timeline,SLOT(renderImage(QSize,QImage*)),Qt::BlockingQueuedConnection);

	connect(m_ffmpeg,SIGNAL(readyReadStandardOutput()),this,SLOT(readStandardOutput()));
	connect(m_ffmpeg,SIGNAL(readyReadStandardError()),this,SLOT(readStandardError()));
	connect(m_ffmpeg,SIGNAL(errorOccurred(QProcess::ProcessError)),this,SLOT(ffmpegErrorOccurred(QProcess::ProcessError)));
	connect(m_ffmpeg,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(ffmpegFinished(int,QProcess::ExitStatus)));


	Music* music = m_timeline->music();
	passert( Log::FFMpeg, music );

	qint64 startFrame = 0;
	qint64 endFrame = m_timeline->length();

	passert( Log::FFMpeg, startFrame < endFrame );

	music->pause();

	//Ugly wait the music thread to pause shoud do a better pause func with mutex
	QThread::sleep(1);
	music->setPosition((double)startFrame/m_timeline->framerate());

	size_t audioBufferSize = sizeof(qint16) * 2 * 512;
	size_t videoBufferSize = sizeof(uchar)*m_resolution.width()*m_resolution.height()*3;

	QTcpSocket videoSocket;
	#ifndef Q_OS_LINUX
	QTcpSocket audioSocket;
	#endif

	QString program("ffmpeg");
	QStringList arguments;

#ifdef Q_OS_WIN
	program = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
	program.replace('/','\\');
#endif

	QString res_str = QString::number(m_resolution.width()) + "x" + QString::number(m_resolution.height());
	QString video_recv_buffer_size = QString::number(videoBufferSize);
	QString audio_recv_buffer_size = QString::number(audioBufferSize);

	pinfo( Log::FFMpeg, this, tr("video resolution ") + QString::number(m_resolution.width()) + "x" + QString::number(m_resolution.height()));
	pinfo( Log::FFMpeg, this, tr("video buffer size ") + video_recv_buffer_size );
	pinfo( Log::FFMpeg, this, tr("audio buffer size ") + audio_recv_buffer_size );

	//globals options
	arguments << "-y"; // << "-loglevel" <<  "56";

	//video input arguments
	arguments << "-vcodec" << "rawvideo"
						<< "-f" << "rawvideo"
						<< "-pixel_format" << "rgb24"
						<< "-s" << res_str
						<< "-r" << QString::number(m_timeline->framerate())
						<< "-i" << (QString("tcp://127.0.0.1:1911?listen&listen_timeout=3000&recv_buffer_size=") + video_recv_buffer_size);

	//audio input arguments
#if 0 //ndef Q_OS_LINUX
	arguments << "-f" << "s16le"
						<< "-ar" << "44100"
						<< "-ac" << "2"
						<< "-i" << (QString("tcp://127.0.0.1:777?listen&listen_timeout=3000&recv_buffer_size=") + audio_recv_buffer_size);
#endif
	//video output arguments
	arguments << "-c:v" << "libx264"
						<< "-crf" << "22"
						<< "-pix_fmt" << "yuv420p";

	//output filename argument
	arguments << m_filename;


	pinfo( Log::FFMpeg, this, tr("Launching ffmpeg: ") + arguments.join(QChar(' ')));
	m_ffmpeg->start(program,arguments,QIODevice::ReadWrite);
	if (!m_ffmpeg->waitForStarted(3000))
	{
		perror( Log::FFMpeg, this, tr("Error unable to start ffmpeg: ") + m_ffmpeg->errorString());
		//QMessageBox::critical(nullptr,"ffmpeg error",tr("Error unable to start ffmpeg: ") + _ffmpeg.errorString());
		QCoreApplication::processEvents();
		delete m_ffmpeg;
		return;
	}

	//connect video socket
	videoSocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, (qulonglong)videoBufferSize);
	videoSocket.connectToHost(QHostAddress("127.0.0.1"),1911);

	if (!videoSocket.waitForConnected(3000))
	{
		perror( Log::FFMpeg, this, tr("Error can't connect video to ffmpeg server: ") + videoSocket.errorString());
		//QMessageBox::critical(nullptr,"ffmpeg error",tr("Error can't connect video to ffmpeg server: ") + videoSocket.errorString());
		QCoreApplication::processEvents();
		delete m_ffmpeg;
		return;
	}

	QImage image; //Image containing the renderer
	//Send first frame to force ffmpeg listen audio socket (I know it's ugly)
	{
		emit requestRendering(m_resolution,&image); //SHOULD BE BLOCKING
		image = image.convertToFormat(QImage::Format_RGB888);
		//image.save("0.png");

		passert( Log::FFMpeg, image.size() == m_resolution );
		passert( Log::FFMpeg, image.byteCount() == videoBufferSize );

		qint64 writted = 0;
		size_t tot_writted = 0;
		while (writted !=-1 && (tot_writted += writted) != image.byteCount())
		{
			writted = videoSocket.write((char*)(image.bits() + tot_writted),image.byteCount()-tot_writted);
		}
		while (videoSocket.bytesToWrite() > 0)
		{
			videoSocket.flush();
			QCoreApplication::processEvents();
		}
		if (writted == - 1)
		{
			perror( Log::FFMpeg, this, tr("VIDEO TCP SOCKET ERROR: ") + videoSocket.errorString());
			//QMessageBox::critical(nullptr,tr("VIDEO TCP SOCKET ERROR"), videoSocket.errorString());
			QCoreApplication::processEvents();
			delete m_ffmpeg;
			return;
		}

		QCoreApplication::processEvents();
	}

#if 0 //ndef Q_OS_LINUX
	//connect audio socket
	audioSocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, (qulonglong)audioBufferSize);
	audioSocket.connectToHost(QHostAddress("127.0.0.1"),777);
	if (!audioSocket.waitForConnected(3000))
	{
		perror( Log::FFMpeg, this, tr("Error can't connect audio to ffmpeg server: ") + audioSocket.errorString());
		//QMessageBox::critical(nullptr,"ffmpeg error",tr("Error can't connect audio to ffmpeg server: ") + audioSocket.errorString());
		QCoreApplication::processEvents();
		delete _ffmpeg;
		return;
	}
#endif

	//Lets' go !
	void* outputBuffer = malloc(audioBufferSize);
	for (size_t i = startFrame + 1; !m_cancel && i < endFrame; ++i)
	{
		double track_time = (double)i/m_timeline->framerate();
		while (track_time >= music->getTime())
		{
			music->processAudio(outputBuffer,512,track_time,0);
#if 0 //ndef Q_OS_LINUX
			qint64 writted = 0;
			size_t tot_writted = 0;
			while (writted !=-1 && (tot_writted += writted) != audioBufferSize) //love ugly loop conditions <333
			{
				writted = audioSocket.write(((const char*)outputBuffer) + tot_writted, audioBufferSize-tot_writted);
				QCoreApplication::processEvents();
			}
			if (writted == - 1)
			{
				perror( Log::FFMpeg, this, tr("AUDIO TCP SOCKET ERROR: ") + audioSocket.errorString());
				//QMessageBox::critical(nullptr,tr("AUDIO TCP SOCKET ERROR"), audioSocket.errorString());
				break;
			}
#endif
		}


		emit requestRendering(m_resolution,&image); //SHOULD BE BLOCKING
		image = image.convertToFormat(QImage::Format_RGB888);
		//image.save(QString::number(i) + ".png");

		passert( Log::FFMpeg, nullptr, image.size() == m_resolution );
		passert( Log::FFMpeg, image.byteCount() == videoBufferSize);

		qint64 writted = 0;
		size_t tot_writted = 0;
		while (writted !=-1 && (tot_writted += writted) != image.byteCount())
		{
			writted = videoSocket.write((char*)(image.bits() + tot_writted),image.byteCount()-tot_writted);
			QCoreApplication::processEvents();
		}
		while (videoSocket.bytesToWrite() > 0)
		{
			videoSocket.flush();
			QCoreApplication::processEvents();
		}
		if (writted == - 1)
		{
			perror( Log::FFMpeg, this, tr("VIDEO TCP SOCKET ERROR: ") + videoSocket.errorString());
			//QMessageBox::critical(nullptr,tr("VIDEO TCP SOCKET ERROR"), videoSocket.errorString());
			break;
		}
		emit newFrameEncoded(i);
		QCoreApplication::processEvents();
	}

	free(outputBuffer);

	do
	{
		QCoreApplication::processEvents();
	} while (videoSocket.bytesToWrite() > 0);

	videoSocket.close();
#if 0 //ndef Q_OS_LINUX
	audioSocket.close();
#endif
	if (!m_ffmpeg->waitForFinished())
	{
		perror( Log::FFMpeg, this, tr("error ffmpeg process not finnished: ") + m_ffmpeg->errorString());
		//QMessageBox::critical(nullptr,tr("ffmpeg error"), _ffmpeg.errorString());
	}
	else
	{
		if (m_cancel)
		{
			pinfo( Log::FFMpeg, this, tr("Video rendering cancelled"));
		}
		else
		{
			pinfo( Log::FFMpeg, this, tr("Video rendering finnished !"));
		}
	}

	QCoreApplication::processEvents();
	delete m_ffmpeg;
}
