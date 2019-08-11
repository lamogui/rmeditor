
#include "4klangmusic.hpp"
#ifdef Q_OS_WIN32

#include <QDebug>


#include <cassert>
#define MAX_SAMPLES 1
_4KlangMusic::_4KlangMusic(const QString& filename, double length, QDomNode node ,LogWidget& log,QObject* parent):
  Music(filename,length,node,log,parent),
_buffer(new float[MAX_SAMPLES*2]),
_bufferPos(0)/*,
_thread(_4klang_render, ((void*)_buffer))*/
{

}

_4KlangMusic::~_4KlangMusic()
{
    _thread.join();
    delete _buffer;
}

double _4KlangMusic::getTime() const
{
    return (double)_bufferPos / (2.0 * 44100.0);
}

bool _4KlangMusic::createRtAudioStream()
{

  try
  {
    RtAudio::StreamParameters parameters;
    parameters.deviceId = audio.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 512;
    bytesPerFrame = sizeof(float) * parameters.nChannels;
    audio.openStream(&parameters,nullptr,RTAUDIO_FLOAT32,sampleRate,&bufferFrames, rtAudioCallback, (void*) this, 0, Music::rtAudioError);
    audio.startStream();
  }
  catch (RtAudioError &err)
  {
    emit error("[" + fileName() + "] " + QString::fromStdString(err.getMessage()));
    return false;
  }
  return true;
}


void _4KlangMusic::processAudio(void *outputBuffer, unsigned int nBufferFrames,
                                  double, RtAudioStreamStatus)
{
  _mutex.lock();
  uint32_t toCopy = nBufferFrames * 2 > (2*MAX_SAMPLES) - _bufferPos ? (2*MAX_SAMPLES) - _bufferPos : nBufferFrames * 2;
  memcpy(outputBuffer,(const void*)&( _buffer[_bufferPos]),toCopy * sizeof(float));

  _bufferPos += toCopy;
  if (_bufferPos >= 2*MAX_SAMPLES)
    _bufferPos = 0;
  _mutex.unlock();
}



bool _4KlangMusic::load()
{
  return true;
}

void _4KlangMusic::setPosition(double time)
{
    _mutex.lock();
    _bufferPos = (uint32_t)(time * 2.0 * 44100.0);
    _mutex.unlock();
}

void _4KlangMusic::updateTextures()
{
     // TODO
/*
  memset((void*)noteVelocityBuffer.data(),0,noteVelocityBuffer.size()*sizeof(float));
  for (size_t i =0 ; i < instrumentCount(); i++)
  {
    for (size_t k = 0; k < TF_MAXVOICES; k++)
    {
      if (player.synth.instr[i]->voice[k].noteIsOn)
      {
        eU32 note = player.synth.instr[i]->voice[k].currentNote%TF_NUMFREQS;
        eF32 velocity = player.synth.instr[i]->voice[k].currentVelocity/128.0f;
        noteVelocityBuffer[note + TF_NUMFREQS * i] = std::max(velocity,noteVelocityBuffer[note + TF_NUMFREQS * i]);
      }
    }
  }

  noteVelocityTex.update((GLvoid*) noteVelocityBuffer.data());*/
}


void _4KlangMusic::exportMusicCData(const QFile&, const QFile&) const
{
    // TODO
}

#endif // Q_OS_WIN32
