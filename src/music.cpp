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

Music::Music(QObject* parent):
  MediaFile(parent),
  m_audio(),
  m_length(length),
  m_playing(false)
{
  /* 
  // FIXME : move this elsewhere
  if (m_audio.getDeviceCount() < 1)
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
  if (music->m_playing)
  {
    music->processAudio(outputBuffer,nBufferFrames,streamTime,status);
    //for (int i = 0; i < nBufferFrames; i++)
    //{
    //    reinterpret_cast<short*>(outputBuffer)[i*2] = sin(static_cast<float>(i)/static_cast<float>(nBufferFrames)) * 16000;
    //    reinterpret_cast<short*>(outputBuffer)[i*2+1] = cos(static_cast<float>(i)/static_cast<float>(nBufferFrames)) * 16000;
    //}
  }
  else
  {
    memset(outputBuffer,0,nBufferFrames*music->m_bytesPerFrame);
  }
  if (status & RTAUDIO_OUTPUT_UNDERFLOW)
    Log::Warning("[RtAudio] output buffer underflow !"); // TODO : make an Log::ASyncWarning Log::ASyncError etc calls 

  return 0;
}

void Music::rtAudioError(RtAudioError::Type type, const std::string &errorText)
{
  (void) type;
  Log::Error(tr("[RtAudio] ") + QString::fromStdString(errorText));
}

void Music::setMainTimeline(Timeline* timeline)
{
  if (mainTimeline != timeline)
  {
    QVariant oldValue = QVariant::fromValue(mainTimeline);
    QVariant newValue = QVariant::fromValue(timeline);
    if (timeline)
      timeline->setParent(this);
    mainTimeline = timeline;
    emit propertyChanged(this, "mainTimeline", oldValue, newValue);
  }
}

/*
** ExternalLengthMusic
*/

ExternalLengthMusic::ExternalLengthMusic(QObject* parent) : 
  Music(parent),
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


