
#include "music.hpp"

#include <cstdlib>

#include <QThread>
#include <QMessageBox>
#include <QDebug>

#include "timeline.hpp"
#include "renderer.hpp"

Music::Music(const QString &filename, double length, QDomNode node, LogWidget &log, QObject *parent):
  NodeFile(filename,node,log,parent),
  m_audio(),
  m_length(length),
  m_playing(false)
{
  if (m_length <= 0)
  {
    emit error("[" + filename + "] error: invalid length");
  }
  if (m_audio.getDeviceCount() < 1)
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
  if (music->m_playing)
  {
    music->processAudio(outputBuffer,nBufferFrames,streamTime,status);
  }
  else
  {
    memset(outputBuffer,0,nBufferFrames*music->m_bytesPerFrame);
  }
  return 0;
}



