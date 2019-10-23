
#include "tunefish4music.hpp"
#include "logmanager.hpp"

Tunefish4Music::Tunefish4Music(const QString& _filename, double _length, QDomNode _node , QObject* _parent):
  Music( _filename, _length, _node, _parent)
{
	eTfPlayerInit(m_player);
	eTfPlayerSetSampleRate(m_player,44100 );
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
		m_audio.openStream(&parameters,nullptr,RTAUDIO_SINT16,sampleRate,&bufferFrames, rtAudioCallback, (void*) this);
		if (bufferFrames != TF_FRAMESIZE)
		{
			perror( Log::Audio, this, tr("cannot set bufferFrames to ") + QString::number(TF_FRAMESIZE));
			return false;
		}
		m_audio.startStream();
	}
	catch (RtAudioError &err)
	{
		perror( Log::Audio, this, QString::fromStdString(err.getMessage() ) );
		return false;
	}
	return true;
}


void Tunefish4Music::processAudio(void * _outputBuffer, unsigned int _nBufferFrames,
                                  double _streamTime, RtAudioStreamStatus _status)
{
	passert( Log::Audio, _nBufferFrames == TF_FRAMESIZE );
	const eS16* playerOut;
	eTfPlayerProcess(m_player,&playerOut);
	memcpy(_outputBuffer,(const void*) playerOut,TF_FRAMESIZE*2*sizeof(eS16));
}



bool Tunefish4Music::load()
{
	eTfPlayerUnloadSong(m_player);
	if (!open(QFile::ReadOnly))
	{
		perror( Log::File, this, tr("cannot open the file for reading" ) );
		return false;
	}
	QByteArray array = readAll();
	bool loadSuccess = eTfPlayerLoadSong(m_player,(const eU8*)array.constData(),array.size(),0);
	m_player.playing = eTRUE;
	close();

	if (!loadSuccess) {
		perror( Log::File, this, tr("tunefish4 could not load the song..." ) );
		return false;
	}

	pinfo( Log::File, this, tr( "tunefish4 song loaded number of instruments: " ) + QString::number(instrumentCount() ) );

	m_noteVelocityBuffer.clear();
	m_noteVelocityBuffer.resize(TF_NUMFREQS * instrumentCount());
	m_noteVelocityTex.create(TF_NUMFREQS,instrumentCount(),(const GLvoid*) m_noteVelocityBuffer.data(),GL_R32F,GL_RED, GL_FLOAT);

	m_maxNoteVelocityBuffer.clear();
	m_maxNoteVelocityBuffer.resize( instrumentCount());
	m_maxNoteVelocityTex.create(1,instrumentCount(),(const GLvoid*) m_maxNoteVelocityBuffer.data(),GL_R32F,GL_RED, GL_FLOAT);
	memset((void*)m_maxNoteVelocityBuffer.data(),0,m_maxNoteVelocityBuffer.size()*sizeof(float));
	memset((void*)m_noteVelocityBuffer.data(),0,m_noteVelocityBuffer.size()*sizeof(float));
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
}

void Tunefish4Music::updateTextures()
{
	for (size_t i =0 ; i < instrumentCount(); i++)
	{
		for (size_t j = 0; j < TF_NUMFREQS; ++j)
		{
			m_noteVelocityBuffer[TF_NUMFREQS * i + j] = m_noteVelocityBuffer[TF_NUMFREQS * i + j] * .9f;
		}

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

	//memset((void*)m_maxNoteVelocityBuffer.data(),0,m_maxNoteVelocityBuffer.size()*sizeof(float));
	for (size_t i =0 ; i < instrumentCount(); i++)
	{
		eF32 maxVel = 0.f;
		for (size_t k = 0; k < TF_MAXVOICES; k++)
		{
			if (m_player.synth.instr[i]->voice[k].noteIsOn)
			{
				eU32 note = m_player.synth.instr[i]->voice[k].currentNote%TF_NUMFREQS;
				eF32 velocity = m_player.synth.instr[i]->voice[k].currentVelocity/128.0f;
				maxVel = std::max(maxVel, velocity);

			}
		}
		if (maxVel >= m_maxNoteVelocityBuffer[i]) {
			m_maxNoteVelocityBuffer[i] = maxVel;
		}
		else {
			m_maxNoteVelocityBuffer[i] = m_maxNoteVelocityBuffer[i] * .9f;
		}
	}
  m_noteVelocityTex.update((GLvoid*) m_noteVelocityBuffer.data());
  m_maxNoteVelocityTex.update((GLvoid*) m_maxNoteVelocityBuffer.data());
}


void Tunefish4Music::exportMusicCData(const QFile& source, const QFile& header) const
{
	// TODO ?
}
