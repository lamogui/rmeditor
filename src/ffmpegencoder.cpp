#include "ffmpegencoder.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
//This shall be replaced by signal because it can't be used in thread
//#include <QMessageBox>
#include <QOpenGLFunctions>
#include <QTcpSocket>


#include "demotimeline.hpp"
#include "logwidget.hpp"
#include "music.hpp"
#include "renderer.hpp"


FFmpegEncoder::FFmpegEncoder(QObject* parent,
                             const QString &filename,
                             DemoTimeline &timeline, const QSize &resolution,
                             LogWidget &log) :
  QThread(parent),
  _filename(filename),
  _resolution(resolution),
  _timeline(&timeline),
  _cancel(false)
{
  connectLog(log);
}

void FFmpegEncoder::connectLog(LogWidget& log)
{
  connect(this,SIGNAL(logError(QString)),&log,SLOT(writeError(QString)),Qt::QueuedConnection);
  connect(this,SIGNAL(logWarning(QString)),&log,SLOT(writeWarning(QString)),Qt::QueuedConnection);
  connect(this,SIGNAL(logInfo(QString)),&log,SLOT(writeInfo(QString)),Qt::QueuedConnection);
}

void FFmpegEncoder::readStandardError()
{
  //FFMPEG Seems to send all in STDERR maybe because STDOUT is used for send video/audio in some cases ?
  QString msg = QString(_ffmpeg->readAllStandardError());
  if (!msg.isEmpty())
  {
    emit logWarning(tr("[ffmpeg] ") + msg);
  }
}

void FFmpegEncoder::readStandardOutput()
{
  emit logInfo(tr("[ffmpeg]  ") + QString(_ffmpeg->readAllStandardOutput()));
}

void FFmpegEncoder::ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitCode == 0 && exitStatus == QProcess::NormalExit)
  {
    emit logInfo(tr("ffmpeg exited with code: ") + QString::number(exitCode));
  }
  else
  {
    _cancel = true;
    if (exitStatus == QProcess::NormalExit)
    {
      emit logError(tr("ffmpeg exited with code: ") + QString::number(exitCode));
    }
    else
    {
      emit logError(tr("ffmpeg crashed !"));
    }
  }
}

void FFmpegEncoder::ffmpegErrorOccurred(QProcess::ProcessError)
{
  emit logError(tr("[ffmpeg] error: ") + _ffmpeg->errorString());
  _cancel = true;
}




void FFmpegEncoder::run()
{

  _ffmpeg = new QProcess();

  //Use signal/slot in queue because of opengl context
  connect(this,SIGNAL(requestRendering(QSize,QImage*)),_timeline,SLOT(renderImage(QSize,QImage*)),Qt::BlockingQueuedConnection);

  connect(_ffmpeg,SIGNAL(readyReadStandardOutput()),this,SLOT(readStandardOutput()));
  connect(_ffmpeg,SIGNAL(readyReadStandardError()),this,SLOT(readStandardError()));
  connect(_ffmpeg,SIGNAL(errorOccurred(QProcess::ProcessError)),this,SLOT(ffmpegErrorOccurred(QProcess::ProcessError)));
  connect(_ffmpeg,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(ffmpegFinished(int,QProcess::ExitStatus)));


  Music* music = _timeline->music();
  Q_ASSERT(music);

  qint64 startFrame = 0;
  qint64 endFrame = _timeline->length();

  Q_ASSERT(startFrame < endFrame);

  music->pause();

  //Ugly wait the music thread to pause shoud do a better pause func with mutex
  QThread::sleep(1);
  music->setPosition((double)startFrame/_timeline->framerate());

  size_t audioBufferSize = sizeof(qint16) * 2 * 512;
  size_t videoBufferSize = sizeof(uchar)*_resolution.width()*_resolution.height()*3;

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

  QString res_str = QString::number(_resolution.width()) + "x" + QString::number(_resolution.height());
  QString video_recv_buffer_size = QString::number(videoBufferSize);
  QString audio_recv_buffer_size = QString::number(audioBufferSize);

  emit logInfo(tr("[ffmpeg] video resolution ") + QString::number(_resolution.width()) + "x" + QString::number(_resolution.height()));
  emit logInfo(tr("[ffmpeg] video buffer size ") + video_recv_buffer_size);
  emit logInfo(tr("[ffmpeg] audio buffer size ") + audioBufferSize);

  //globals options
  arguments << "-y"; // << "-loglevel" <<  "56";

  //video input arguments
  arguments << "-vcodec" << "rawvideo"
            << "-f" << "rawvideo"
            << "-pixel_format" << "rgb24"
            << "-s" << res_str
            << "-r" << QString::number(_timeline->framerate())
            << "-i" << (QString("tcp://127.0.0.1:1911?listen&listen_timeout=3000&recv_buffer_size=") + video_recv_buffer_size);

  //audio input arguments
#ifndef Q_OS_LINUX
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
  arguments << _filename;


  emit logInfo(tr("Launching ffmpeg: ") + arguments.join(QChar(' ')));
  _ffmpeg->start(program,arguments,QIODevice::ReadWrite);
  if (!_ffmpeg->waitForStarted(3000))
  {
    emit logError(tr("Error unable to start ffmpeg: ") + _ffmpeg->errorString());
    //QMessageBox::critical(NULL,"ffmpeg error",tr("Error unable to start ffmpeg: ") + _ffmpeg.errorString());
    QCoreApplication::processEvents();
    delete _ffmpeg;
    return;
  }

  //connect video socket
  videoSocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, (qulonglong)videoBufferSize);
  videoSocket.connectToHost(QHostAddress("127.0.0.1"),1911);

  if (!videoSocket.waitForConnected(3000))
  {
    emit logError(tr("Error can't connect video to ffmpeg server: ") + videoSocket.errorString());
    //QMessageBox::critical(NULL,"ffmpeg error",tr("Error can't connect video to ffmpeg server: ") + videoSocket.errorString());
    QCoreApplication::processEvents();
    delete _ffmpeg;
    return;
  }

  QImage image; //Image containing the renderer
  //Send first frame to force ffmpeg listen audio socket (I know it's ugly)
  {
    emit requestRendering(_resolution,&image); //SHOULD BE BLOCKING
    image = image.convertToFormat(QImage::Format_RGB888);

    Q_ASSERT(image.size() == _resolution);
    Q_ASSERT(image.byteCount() == videoBufferSize);

    qint64 writted = 0;
    size_t tot_writted = 0;
    while (writted != (qint64)-1 && (tot_writted += writted) != image.byteCount())
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
      emit logError(tr("VIDEO TCP SOCKET ERROR: ") + videoSocket.errorString());
      //QMessageBox::critical(NULL,tr("VIDEO TCP SOCKET ERROR"), videoSocket.errorString());
      QCoreApplication::processEvents();
      delete _ffmpeg;
      return;
    }

    QCoreApplication::processEvents();
  }

#ifndef Q_OS_LINUX
  //connect audio socket
  audioSocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, (qulonglong)audioBufferSize);
  audioSocket.connectToHost(QHostAddress("127.0.0.1"),777);
  if (!audioSocket.waitForConnected(3000))
  {
    emit logError(tr("Error can't connect audio to ffmpeg server: ") + audioSocket.errorString());
    //QMessageBox::critical(NULL,"ffmpeg error",tr("Error can't connect audio to ffmpeg server: ") + audioSocket.errorString());
    QCoreApplication::processEvents();
    delete _ffmpeg;
    return;
  }
#endif

  //Lets' go !
  void* outputBuffer = malloc(audioBufferSize);
  for (size_t i = startFrame + 1; !_cancel && (qint64)i < endFrame; ++i)
  {
    double track_time = (double)i/_timeline->framerate();
    while (track_time >= music->getTime())
    {
      music->processAudio(outputBuffer,512,track_time,0);
#ifndef Q_OS_LINUX
      qint64 writted = 0;
      size_t tot_writted = 0;
      while (writted !=-1 && (tot_writted += writted) != audioBufferSize) //love ugly loop conditions <333
      {
        writted = audioSocket.write(((const char*)outputBuffer) + tot_writted, audioBufferSize-tot_writted);
        QCoreApplication::processEvents();
      }
      if (writted == - 1)
      {
        emit logError(tr("AUDIO TCP SOCKET ERROR: ") + audioSocket.errorString());
        //QMessageBox::critical(NULL,tr("AUDIO TCP SOCKET ERROR"), audioSocket.errorString());
        break;
      }
#endif
    }


    emit requestRendering(_resolution,&image); //SHOULD BE BLOCKING
    image = image.convertToFormat(QImage::Format_RGB888);

    Q_ASSERT(image.size() == _resolution);
    Q_ASSERT(image.byteCount() == videoBufferSize);

    qint64 writted = 0;
    size_t tot_writted = 0;
    while (writted != (quint64)-1 && (tot_writted += writted) != image.byteCount())
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
      emit logError(tr("VIDEO TCP SOCKET ERROR: ") + videoSocket.errorString());
      //QMessageBox::critical(NULL,tr("VIDEO TCP SOCKET ERROR"), videoSocket.errorString());
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
  if (!_ffmpeg->waitForFinished())
  {
    emit logError(tr("error ffmpeg process not finnished: ") + _ffmpeg->errorString());
    //QMessageBox::critical(NULL,tr("ffmpeg error"), _ffmpeg.errorString());
  }
  else
  {
    if (_cancel)
    {
      emit logInfo(tr("Video rendering cancelled"));
    }
    else
    {
      emit logInfo(tr("Video rendering finnished !"));
    }
  }

  QCoreApplication::processEvents();
  delete _ffmpeg;
}
