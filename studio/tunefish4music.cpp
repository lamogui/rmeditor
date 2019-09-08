#include "tunefish4music.hpp"
#include "logmanager.hpp"

Tunefish4Music::Tunefish4Music(Project* _parent, const QFileInfo& _path):
	UserSettedLengthMusic(_parent, _path)
{
  eTfPlayerInit(m_player);
  eTfPlayerSetSampleRate(m_player,44100 );
}

Tunefish4Music::~Tunefish4Music()
{
  eTfPlayerUnloadSong(m_player);
}

qreal Tunefish4Music::getTime() const
{
	return static_cast<qreal>(m_player.time);
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
		m_audio.openStream(&parameters,nullptr,RTAUDIO_SINT16,sampleRate,&bufferFrames, rtAudioCallback, reinterpret_cast<void*>(this));
    if (bufferFrames != TF_FRAMESIZE)
    {
			perror(Log::Audio, this, "cannot set bufferFrames to " + QString::number(TF_FRAMESIZE));
      return false;
    }
    m_audio.startStream();
  }
  catch (RtAudioError &err)
  {
		rtAudioError(err.getType(), err.getMessage());
    return false;
  }
  return true;
}


void Tunefish4Music::processAudio(void *outputBuffer, unsigned int nBufferFrames,
                                  double, RtAudioStreamStatus)
{
	passert(Log::Code, this, nBufferFrames == TF_FRAMESIZE);
  const eS16* playerOut;
  eTfPlayerProcess(m_player,&playerOut);
	memcpy(outputBuffer,reinterpret_cast<const void*>(playerOut),TF_FRAMESIZE*2*sizeof(eS16));
}



bool Tunefish4Music::load()
{
	QFile file(m_path.absoluteFilePath());
	eTfPlayerUnloadSong(m_player);
	if (!file.open(QFile::ReadOnly))
	{
		perror(Log::File, this, tr("Cannot open the file ") + m_path.absoluteFilePath() + tr(" for reading"));
		return false;
	}
	QByteArray array = file.readAll();
	bool loadSuccess = eTfPlayerLoadSong(m_player,reinterpret_cast<const eU8*>(array.constData()),static_cast<eU32>(array.size()),0);
	m_player.playing = eTRUE;
	file.close();

	if (!loadSuccess) {
			perror(Log::File, this, tr("Tunefish4 could not load the song..."));
			return false;
	}

	pinfo(Log::File, this, tr("Tunefish4 song loaded number of instruments: ") + QString::number(m_player.song.instrCount));

	memset(static_cast<void*>(m_noteVelocityBuffer.data()),0,m_noteVelocityBuffer.size()*sizeof(float));
	memset(static_cast<void*>(m_maxNoteVelocityBuffer.data()),0,m_maxNoteVelocityBuffer.size()*sizeof(float));

	allocateNoteVelocityTextures( static_cast<int>(m_player.song.instrCount), TF_NUMFREQS, static_cast<void*>(m_maxNoteVelocityBuffer.data()), static_cast<void*>(m_maxNoteVelocityBuffer.data()));
	return true;
	}

void Tunefish4Music::setPosition(qreal time)
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

	m_player.time = static_cast<eF32>(time);

}

void Tunefish4Music::updateTextures()
{
	for (size_t i =0 ; i < m_player.song.instrCount; i++)
	{
		for (size_t j = 0; j < TF_NUMFREQS; ++j)
		{
			m_noteVelocityBuffer[TF_NUMFREQS * i + j] = m_noteVelocityBuffer[TF_NUMFREQS * i + j] * .9f;
		}

		for (size_t k = 0; k < TF_MAXVOICES; k++)
		{
			if (m_player.synth.instr[i]->voice[k].noteIsOn)
			{
				eU32 note = static_cast<eU32>(m_player.synth.instr[i]->voice[k].currentNote)%TF_NUMFREQS;
				eF32 velocity = m_player.synth.instr[i]->voice[k].currentVelocity/128.0f;
				m_noteVelocityBuffer[note + TF_NUMFREQS * i] = std::max(velocity,m_noteVelocityBuffer[note + TF_NUMFREQS * i]);
			}
		}
	}

	for (size_t i =0 ; i < m_player.song.instrCount; i++)
	{
			eF32 maxVel = 0.f;
		for (size_t k = 0; k < TF_MAXVOICES; k++)
		{
			if (m_player.synth.instr[i]->voice[k].noteIsOn)
			{
				eF32 velocity = m_player.synth.instr[i]->voice[k].currentVelocity/128.0f;
				maxVel = std::max(maxVel, velocity);

			}
		}
		if (maxVel >= m_maxNoteVelocityBuffer[i])
		 m_maxNoteVelocityBuffer[i] = maxVel;
		else {
		 m_maxNoteVelocityBuffer[i] = m_maxNoteVelocityBuffer[i] * .9f;
		 //qDebug() << m_maxNoteVelocityBuffer[i];
		}
	}

	m_noteVelocityTexture.setData(QOpenGLTexture::PixelFormat::Luminance, QOpenGLTexture::PixelType::Float32, m_noteVelocityBuffer.data());
	m_maxNoteVelocityTexture.setData(QOpenGLTexture::PixelFormat::Luminance, QOpenGLTexture::PixelType::Float32, m_maxNoteVelocityBuffer.data());
}


void Tunefish4Music::exportMusicCData(const QFile&, const QFile&) const
{
	ptodo("Tunefish4Music::exportMusicCData");
}
