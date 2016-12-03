#include <QDebug>

#include "tunefish4music.hpp"
#include <cassert>

Tunefish4Music::Tunefish4Music(const QString& filename, double length, QDomNode node ,LogWidget& log,QObject* parent):
  Music(filename,length,node,log,parent)
{
  eTfPlayerInit(m_player,44100);
}

Tunefish4Music::~Tunefish4Music()
{
  eTfPlayerUnloadSong(m_player);
}

double Tunefish4Music::getTime() const
{
  return (double)m_player.time;
}

bool Tunefish4Music::createRtAudioStream()
{

  try
  {
    RtAudio::StreamParameters parameters;
    parameters.deviceId = m_audio.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = TF_FRAMESIZE;
    m_bytesPerFrame = sizeof(eS16) * parameters.nChannels;
    m_audio.openStream(&parameters,NULL,RTAUDIO_SINT16,sampleRate,&bufferFrames, rtAudioCallback, (void*) this, 0, Music::rtAudioError);
    if (bufferFrames != TF_FRAMESIZE)
    {
      emit error("[" + fileName() + "] Canno't set bufferFrames to " + QString::number(TF_FRAMESIZE));
      return false;
    }
    m_audio.startStream();
  }
  catch (RtAudioError &err)
  {
    emit error("[" + fileName() + "] " + QString::fromStdString(err.getMessage()));
    return false;
  }
  return true;
}


void Tunefish4Music::processAudio(void *outputBuffer, unsigned int nBufferFrames,
                                  double, RtAudioStreamStatus)
{
  assert(nBufferFrames == TF_FRAMESIZE);
  const eU8* playerOut;
  eTfPlayerProcess(m_player,&playerOut);
  memcpy(outputBuffer,(const void*) playerOut,TF_FRAMESIZE*2*sizeof(eS16));

}



bool Tunefish4Music::load()
{
  eTfPlayerUnloadSong(m_player);
  if (!open(QFile::ReadOnly))
  {
    emit error("[" + fileName()+ "] Canno't open the file for reading");
    return false;
  }
  QByteArray array = readAll();
  eTfPlayerLoadSong(m_player,(const eU8*)array.constData(),array.size(),0);
  m_player.playing = eTRUE;
  close();

  m_noteVelocityBuffer.clear();
  m_noteVelocityBuffer.resize(TF_NUMFREQS * instrumentCount());
  m_noteVelocityTex.create(TF_NUMFREQS,instrumentCount(),(const GLvoid*) m_noteVelocityBuffer.data(),GL_R32F,GL_RED, GL_FLOAT);

  return true;
}

void Tunefish4Music::setPosition(double time)
{
  //Panic notes
  for (eU32 i=0; i<TF_MAX_INSTR; i++)
  {
    eTfInstrument *instr = m_player.synth.instr[i];

    if (instr)
    {
      eTfInstrumentAllNotesOff(*instr);
    }
  }

  m_player.time = (eF32) time;
  //qDebug() << "set music time " << time;

}

void Tunefish4Music::updateTextures()
{

  memset((void*)m_noteVelocityBuffer.data(),0,m_noteVelocityBuffer.size()*sizeof(float));
  for (size_t i =0 ; i < instrumentCount(); i++)
  {
    for (size_t k = 0; k < TF_MAXVOICES; k++)
    {
      if (m_player.synth.instr[i]->voice[k].noteIsOn)
      {
        eU32 note = m_player.synth.instr[i]->voice[k].currentNote%TF_NUMFREQS;
        eF32 velocity = m_player.synth.instr[i]->voice[k].currentVelocity/128.0f;
        m_noteVelocityBuffer[note + TF_NUMFREQS * i] = std::max(velocity,m_noteVelocityBuffer[note + TF_NUMFREQS * i]);
      }
    }
  }

  m_noteVelocityTex.update((GLvoid*) m_noteVelocityBuffer.data());
}


void Tunefish4Music::exportMusicCData(const QFile& source, const QFile& header) const
{

}
