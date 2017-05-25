
#include "music.hpp"

#include <cstdlib>

#include <QThread>
#include <QMessageBox>
#include <QDebug>

#include "timeline.hpp"
#include "renderer.hpp"

Music::Music(const QString &filename, double length, QDomNode node, LogWidget &log, QObject *parent):
  NodeFile(filename,node,log,parent),
  audio(),
  length(length),
  playing(false)
{
  if (length <= 0)
  {
    emit error("[" + filename + "] error: invalid length");
  }
  if (audio.getDeviceCount() < 1)
  {
    emit error("[" + filename + "] error: no audio device found");
  }

}

Music::~Music()
{
}

int Music::rtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
  (void) inputBuffer;

  Q_ASSERT(userData && outputBuffer);
  Music* music = (Music*) userData;
  if (music->playing)
  {
    music->processAudio(outputBuffer,nBufferFrames,streamTime,status);
  }
  else
  {
    memset(outputBuffer,0,nBufferFrames*music->bytesPerFrame);
  }
  return 0;
}

void Music::rtAudioError(RtAudioError::Type type, const std::string &errorText)
{
  (void) type;
  qDebug() << tr("rtAudio error: ") + QString::fromStdString(errorText);
}


