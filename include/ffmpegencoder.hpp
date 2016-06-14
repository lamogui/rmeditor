#ifndef FFMPEGENCODER_HPP
#define FFMPEGENCODER_HPP

#include <QImage>
#include <QTcpSocket>
#include <QThread>
#include <QProcess>
#include <QProgressBar>


class DemoTimeline;
class LogWidget;
class Music;

class FFmpegEncoder : public QThread
{
    Q_OBJECT

  public:
    FFmpegEncoder(QObject *parent,
                  const QString& filename,
                  DemoTimeline& timeline,
                  const QSize& resolution,
                  LogWidget&  log);

   virtual void connectLog(LogWidget& log);

  public slots:
    void cancel() { _cancel = true; }


  protected slots:
    void readStandardError();
    void readStandardOutput();
    void ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void ffmpegErrorOccurred(QProcess::ProcessError);

  signals:
    void logError(QString err);
    void logWarning(QString warn);
    void logInfo(QString txt);

    void requestRendering(QSize resolution, QImage* target);
    void newFrameEncoded(int frame);

  protected:

    virtual void run();


    QString _filename;
    QSize _resolution;
    DemoTimeline* _timeline;
    volatile bool _cancel;
    QProcess* _ffmpeg;


};

#endif
