#include "ffmpegencoder.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
//This shall be replaced by signal because it can't be used in thread
//#include <QMessageBox>
#include <QTcpSocket>


#include "project.hpp"
#include "logwidget.hpp"
#include "music.hpp"
#include "renderer.hpp"


FFmpegEncoder::FFmpegEncoder(QObject* _parent,
															const QString &_filename,
															const Project &_project, const QSize &_resolution,
															LogWidget &_log) :
	QThread(_parent),
	m_filename(_filename),
	m_resolution(_resolution),
	m_project(_project),
	m_cancel(false)
{
	connectLog(_log);
}

void FFmpegEncoder::connectLog(LogWidget& _log)
{
	connect(this,SIGNAL(logError(QString)),&_log,SLOT(writeError(QString)),Qt::QueuedConnection);
	connect(this,SIGNAL(logWarning(QString)),&_log,SLOT(writeWarning(QString)),Qt::QueuedConnection);
	connect(this,SIGNAL(logInfo(QString)),&_log,SLOT(writeInfo(QString)),Qt::QueuedConnection);
}

void FFmpegEncoder::readStandardError()
{
  //FFMPEG Seems to send all in STDERR maybe because STDOUT is used for send video/audio in some cases ?
	QString msg = QString(m_ffmpeg->readAllStandardError());
  if (!msg.isEmpty())
  {
    emit logWarning(tr("[ffmpeg] ") + msg);
  }
}

void FFmpegEncoder::readStandardOutput()
{
	emit logInfo(tr("[ffmpeg]  ") + QString(m_ffmpeg->readAllStandardOutput()));
}

void FFmpegEncoder::ffmpegFinished(int _exitCode, QProcess::ExitStatus _exitStatus)
{
	if (_exitCode == 0 && _exitStatus == QProcess::NormalExit)
	{
		emit logInfo(tr("ffmpeg exited with code: ") + QString::number(_exitCode));
	}
	else
	{
		m_cancel = true;
		if (_exitStatus == QProcess::NormalExit)
		{
			emit logError(tr("ffmpeg exited with code: ") + QString::number(_exitCode));
		}
		else
		{
			emit logError(tr("ffmpeg crashed !"));
		}
	}
}

void FFmpegEncoder::ffmpegErrorOccurred(QProcess::ProcessError)
{
	emit logError(tr("[ffmpeg] error: ") + m_ffmpeg->errorString());
	m_cancel = true;
}

void FFmpegEncoder::run()
{

	m_ffmpeg = new QProcess();

  //Use signal/slot in queue because of opengl context
	connect(this,&FFmpegEncoder::requestRendering, &m_project.m_demoTimeline, &DemoTimeline::renderImage,Qt::BlockingQueuedConnection);

	connect(m_ffmpeg,&QProcess::readyReadStandardOutput,this,&FFmpegEncoder::readStandardOutput);
	connect(m_ffmpeg,&QProcess::readyReadStandardError,this,&FFmpegEncoder::readStandardError);
	connect(m_ffmpeg,&QProcess::errorOccurred,this,&FFmpegEncoder::ffmpegErrorOccurred);
	void (QProcess::*correctFinishedSignal)(int, QProcess::ExitStatus) = &QProcess::finished; // use this trick to get the correct signal see https://wiki.qt.io/New_Signal_Slot_Syntax#Overload
	connect(m_ffmpeg, correctFinishedSignal, this, &FFmpegEncoder::ffmpegFinished);

	Music* music = m_project.m_music;
  Q_ASSERT(music);

  qint64 startFrame = 0;
	qint64 endFrame = m_project.getNumFrames();

  Q_ASSERT(startFrame < endFrame);

	music->m_playing = false;

  //Ugly wait the music thread to pause shoud do a better pause func with mutex
  QThread::sleep(1);
	music->setPosition(static_cast<qreal>(startFrame)/m_project.getFramerate());

  size_t audioBufferSize = sizeof(qint16) * 2 * 512;
	int videoBufferSize = static_cast<int>(sizeof(uchar))*m_resolution.width()*m_resolution.height()*3;

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

	emit logInfo(tr("[ffmpeg] video resolution ") + QString::number(m_resolution.width()) + "x" + QString::number(m_resolution.height()));
  emit logInfo(tr("[ffmpeg] video buffer size ") + video_recv_buffer_size);
	emit logInfo(tr("[ffmpeg] audio buffer size ") + QString::number(audioBufferSize));

  //globals options
  arguments << "-y"; // << "-loglevel" <<  "56";

  //video input arguments
  arguments << "-vcodec" << "rawvideo"
            << "-f" << "rawvideo"
            << "-pixel_format" << "rgb24"
            << "-s" << res_str
						<< "-r" << QString::number(m_project.getFramerate())
            << "-i" << (QString("tcp://127.0.0.1:1911?listen&listen_timeout=3000&recv_buffer_size=") + video_recv_buffer_size);

  //audio input arguments
#if 0
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


  emit logInfo(tr("Launching ffmpeg: ") + arguments.join(QChar(' ')));
	m_ffmpeg->start(program,arguments,QIODevice::ReadWrite);
	if (!m_ffmpeg->waitForStarted(3000))
  {
		emit logError(tr("Error unable to start ffmpeg: ") + m_ffmpeg->errorString());
    //QMessageBox::critical(nullptr,"ffmpeg error",tr("Error unable to start ffmpeg: ") + _ffmpeg.errorString());
    QCoreApplication::processEvents();
		delete m_ffmpeg;
    return;
  }

  //connect video socket
	videoSocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, static_cast<qulonglong>(videoBufferSize));
  videoSocket.connectToHost(QHostAddress("127.0.0.1"),1911);

  if (!videoSocket.waitForConnected(3000))
  {
    emit logError(tr("Error can't connect video to ffmpeg server: ") + videoSocket.errorString());
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

		Q_ASSERT(image.size() == m_resolution);
    Q_ASSERT(image.byteCount() == videoBufferSize);

    qint64 writted = 0;
		qint64 tot_writted = 0;
    while (writted !=-1 && (tot_writted += writted) != image.byteCount())
    {
			writted = videoSocket.write(reinterpret_cast<char*>(image.bits() + tot_writted),image.byteCount()-tot_writted);
    }
    while (videoSocket.bytesToWrite() > 0)
    {
      videoSocket.flush();
      QCoreApplication::processEvents();
    }
    if (writted == - 1)
    {
      emit logError(tr("VIDEO TCP SOCKET ERROR: ") + videoSocket.errorString());
      //QMessageBox::critical(nullptr,tr("VIDEO TCP SOCKET ERROR"), videoSocket.errorString());
      QCoreApplication::processEvents();
			delete m_ffmpeg;
      return;
    }

    QCoreApplication::processEvents();
  }

  //Lets' go !
  void* outputBuffer = malloc(audioBufferSize);
	for (qint64 i = startFrame + 1; !m_cancel && i < endFrame; ++i)
  {
		qreal track_time = static_cast<qreal>(i)/m_project.getFramerate();
    while (track_time >= music->getTime())
    {
			music->processAudio(outputBuffer,512,track_time,0);
    }
		emit requestRendering(m_resolution,&image); //SHOULD BE BLOCKING
    image = image.convertToFormat(QImage::Format_RGB888);
    //image.save(QString::number(i) + ".png");

		Q_ASSERT(image.size() == m_resolution);
		Q_ASSERT(image.byteCount() == videoBufferSize);

    qint64 writted = 0;
		qint64 tot_writted = 0;
		while (writted !=-1 && (tot_writted += writted) != image.byteCount())
    {
			writted = videoSocket.write(reinterpret_cast<char*>(image.bits()) + tot_writted,image.byteCount()-tot_writted);
      QCoreApplication::processEvents();
    }
    while (videoSocket.bytesToWrite() > 0)
    {
      videoSocket.flush();
      QCoreApplication::processEvents();
    }
    if (writted == - 1)
    {
      emit logError(tr("VIDEO TCP SOCKET ERROR: ") + videoSocket.errorString());
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
#ifndef Q_OS_LINUX
  audioSocket.close();
#endif
	if (!m_ffmpeg->waitForFinished())
  {
		emit logError(tr("error ffmpeg process not finnished: ") + m_ffmpeg->errorString());
    //QMessageBox::critical(nullptr,tr("ffmpeg error"), _ffmpeg.errorString());
  }
  else
  {
		if (m_cancel)
    {
      emit logInfo(tr("Video rendering cancelled"));
    }
    else
    {
      emit logInfo(tr("Video rendering finnished !"));
    }
  }

  QCoreApplication::processEvents();
	delete m_ffmpeg;
}
