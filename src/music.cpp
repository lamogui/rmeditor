﻿
#include "music.hpp"

#include <cstdlib>

#include <QThread>
#include <QMessageBox>

#include "logmanager.hpp"
#include "jassert.hpp"

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

  jassert(userData && outputBuffer);
  Music* music = (Music*) userData;
  if (music->playing)
  {
    music->processAudio(outputBuffer,nBufferFrames,streamTime,status);
  }
  else
  {
    memset(outputBuffer,0,nBufferFrames*music->bytesPerFrame);
  }
  if (status | RTAUDIO_OUTPUT_UNDERFLOW)
    Log::Warning("[RtAudio] output buffer underflow !"); // TODO : make an Log::ASyncWarning Log::ASyncError etc calls 

  return 0;
}

void Music::rtAudioError(RtAudioError::Type type, const std::string &errorText)
{
  (void) type;
  Log::Error(tr("[RtAudio] ") + QString::fromStdString(errorText));
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
    Log::Error("[" + getPath().fileName() + "] error: invalid length " + newlength);
  }
  SET_PROPERTY(double, length)
}


