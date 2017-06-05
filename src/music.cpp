
#include "music.hpp"

#include <cstdlib>

#include <QThread>
#include <QMessageBox>
#include <QDebug>

#include "timeline.hpp"
#include "renderer.hpp"

/*
** Music
*/

Music::Music():
  MediaFile(),
  audio(),
  playing(false)
{
  /* 
  // FIXME : move this elsewhere
  if (audio.getDeviceCount() < 1)
  {
    emit error("[" + filename + "] error: no audio device found");
  }
  */
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

/*
** ExternalLengthMusic
*/

ExternalLengthMusic::ExternalLengthMusic() : 
  Music(),
  length(0)
{
}

void ExternalLengthMusic::setLength(double newlength)
{
  if (newlength <= 0)
  {
    newlength = 0;
    emit error("[" + getPath().fileName() + "] error: invalid length " + newlength);
  }
  SET_PROPERTY(double, length)
}


