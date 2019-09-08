
#include "music.hpp"

#include <cstdlib>

#include <QThread>
#include <QMessageBox>

#include "logmanager.hpp"

#include "project.hpp"

/*
** Music
*/

Music::Music(Project* _parent, const QFileInfo& _path):
	MediaFile(_parent, _path),
  m_playing(false),
  m_audio(),
  m_noteVelocityTexture(QOpenGLTexture::Target2D),
  m_maxNoteVelocityTexture(QOpenGLTexture::Target2D)
{

  // FIXME : move this elsewhere
	if (m_audio.getDeviceCount() < 1) {
		perror(Log::System, this, tr("no audio device found"));
  }

}

int Music::rtAudioCallback(void* _outputBuffer, void * _inputBuffer, unsigned int _nBufferFrames, double _streamTime, RtAudioStreamStatus _status, void *_userData)
{
	(void) _inputBuffer;

	Music* music = reinterpret_cast<Music*>( _userData );
	passert(Log::Code, music, music && _outputBuffer);

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
	if (_status & RTAUDIO_OUTPUT_UNDERFLOW) {
		pwarning( Log::Audio, music, tr( "output buffer underflow !" ));
	}

  return 0;
}

void Music::rtAudioError(RtAudioError::Type _type, const std::string& _errorText)
{
	if ( _type == RtAudioError::WARNING || _type == RtAudioError::DEBUG_WARNING ) {
		pwarning( Log::Audio, nullptr, QString::fromStdString(_errorText) );
	} else if ( _type == RtAudioError::INVALID_PARAMETER || _type == RtAudioError::INVALID_USE) {
		perror( Log::Code, nullptr, QString::fromStdString(_errorText) );
	} else {
		perror( Log::Audio, nullptr, QString::fromStdString(_errorText) );
	}
}

void Music::allocateNoteVelocityTextures(int _nbInstruments, int _nbNotes, const void* _velocityBuffer, const void* _maxVelocityBuffer){
	if (!m_noteVelocityTexture.isCreated()) {
		createNoteVelocityTexture(m_noteVelocityTexture);
	}
	m_noteVelocityTexture.setSize( _nbNotes, _nbInstruments );
	m_noteVelocityTexture.setData(QOpenGLTexture::PixelFormat::Luminance, QOpenGLTexture::PixelType::Float32, _velocityBuffer);

	if (m_maxNoteVelocityTexture.isCreated()) {
		createNoteVelocityTexture(m_maxNoteVelocityTexture);
	}
	m_noteVelocityTexture.setSize( _nbNotes, _nbInstruments );
	m_noteVelocityTexture.setData(QOpenGLTexture::PixelFormat::Luminance, QOpenGLTexture::PixelType::Float32, _maxVelocityBuffer);
}

void Music::createNoteVelocityTexture(QOpenGLTexture& _texture) {
	_texture.create();
	_texture.setFormat(QOpenGLTexture::R32F);
	_texture.setWrapMode(QOpenGLTexture::WrapMode::Repeat);
	_texture.setMinMagFilters(QOpenGLTexture::Filter::LinearMipMapLinear, QOpenGLTexture::Filter::Linear);
}

/*
** UserSettedLengthMusic
*/

UserSettedLengthMusic::UserSettedLengthMusic(Project* _parent, const QFileInfo& _path) :
	Music(_parent, _path),
  m_length(0)
{
}


