#include <QDebug>

#include "tunefish4music.hpp"
#include <cassert>

Tunefish4Music::Tunefish4Music(const QString& filename, double length, QDomNode node ,LogWidget& log,QObject* parent):
  Music(filename,length,node,log,parent)
{
  eTfPlayerInit(player,44100);
}

Tunefish4Music::~Tunefish4Music()
{
  eTfPlayerUnloadSong(player);
}

double Tunefish4Music::getTime() const
{
  return (double)player.time;
}

bool Tunefish4Music::createRtAudioStream()
{

  try
  {
    RtAudio::StreamParameters parameters;
    parameters.deviceId = audio.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = TF_FRAMESIZE;
    bytesPerFrame = sizeof(eS16) * parameters.nChannels;
    audio.openStream(&parameters,nullptr,RTAUDIO_SINT16,sampleRate,&bufferFrames, rtAudioCallback, (void*) this, 0, Music::rtAudioError);
    if (bufferFrames != TF_FRAMESIZE)
    {
      emit error("[" + fileName() + "] Canno't set bufferFrames to " + QString::number(TF_FRAMESIZE));
      return false;
    }
    audio.startStream();
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
  eTfPlayerProcess(player,&playerOut);
  memcpy(outputBuffer,(const void*) playerOut,TF_FRAMESIZE*2*sizeof(eS16));

}



bool Tunefish4Music::load()
{
  eTfPlayerUnloadSong(player);
  if (!open(QFile::ReadOnly))
  {
    emit error("[" + fileName()+ "] Canno't open the file for reading");
    return false;
  }
  QByteArray array = readAll();
  eTfPlayerLoadSong(player,(const eU8*)array.constData(),array.size(),0);
  player.playing = eTRUE;
  close();

  noteVelocityBuffer.clear();
  noteVelocityBuffer.resize(TF_NUMFREQS * instrumentCount());
  noteVelocityTex.create(TF_NUMFREQS,(GLsizei)instrumentCount(),(const GLvoid*) noteVelocityBuffer.data(),GL_R32F,GL_RED, GL_FLOAT);

  return true;
}

void Tunefish4Music::setPosition(double time)
{
  //Panic notes
  for (eU32 i=0; i<TF_MAX_INSTR; i++)
  {
    eTfInstrument *instr = player.synth.instr[i];

    if (instr)
    {
      eTfInstrumentAllNotesOff(*instr);
    }
  }

  player.time = (eF32) time;
  //qDebug() << "set music time " << time;

}

void Tunefish4Music::updateTextures()
{

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

  noteVelocityTex.update((GLvoid*) noteVelocityBuffer.data());
}


void Tunefish4Music::exportMusicCData(const QFile& source, const QFile& header) const
{

}
