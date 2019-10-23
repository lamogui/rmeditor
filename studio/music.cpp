
#include "music.hpp"

#include <cstdlib>

#include <QThread>
#include <QMessageBox>

#include "timeline.hpp"
#include "renderer.hpp"
#include "logmanager.hpp"

Music::Music(const QString &_filename, double _length, QDomNode _node, QObject *_parent):
	NodeFile(_filename,_node,_parent),
  m_audio(),
	m_length(_length),
  m_playing(false)
{
	if (m_length <= 0)
	{
		perror( Log::Code, this, tr( "invalid length" ) );
	}
	if (m_audio.getDeviceCount() < 1)
	{
		perror( Log::System, this, tr( "no audio device found" ) );
	}
}

Music::~Music()
{
}

int Music::rtAudioCallback(void *_outputBuffer, void *_inputBuffer, unsigned int _nBufferFrames, double _streamTime, RtAudioStreamStatus _status, void *_userData)
{
	(void) _inputBuffer;
	Music* music = static_cast< Music* >( _userData );
	passert( Log::Audio, music, _userData && _outputBuffer);

	if (music->m_playing)
	{
		music->processAudio(_outputBuffer,_nBufferFrames,_streamTime,_status);
		//for (int i = 0; i < nBufferFrames; i++)
		//{
		//    reinterpret_cast<short*>(outputBuffer)[i*2] = sin(static_cast<float>(i)/static_cast<float>(nBufferFrames)) * 16000;
		//    reinterpret_cast<short*>(outputBuffer)[i*2+1] = cos(static_cast<float>(i)/static_cast<float>(nBufferFrames)) * 16000;
		//}
	}
	else
	{
		memset(_outputBuffer,0,_nBufferFrames*music->m_bytesPerFrame);
	}
	return 0;
}



