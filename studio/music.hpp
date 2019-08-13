#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <QMutex>
#include <QOpenGLTexture>

#include "RtAudio.h"
#include "mediafile.hpp"


class RtAudio;
class Music : public MediaFile
{
	Q_OBJECT

public:
	Music(QObject* _parent);
	virtual ~Music();

	virtual qreal getTime() const = 0;
	virtual qreal getLength() const = 0;

	inline const QOpenGLTexture& getNoteVelocityTexture() { return m_noteVelocityTexture; }
	inline const QOpenGLTexture& getMaxNoteVelocityTexture() { return m_maxNoteVelocityTexture; }

	virtual void exportMusicCData(const QFile& _source, const QFile& _header) const = 0;

	// Control
	volatile bool m_playing;

	/*
		RtAudio stuff
	*/
	virtual bool createRtAudioStream() = 0;
	virtual void processAudio(void *_outputBuffer, unsigned int _nBufferFrames,
														double _streamTime, RtAudioStreamStatus _status) = 0;

public slots:
	virtual void setPosition(qreal _time) = 0;
	virtual void updateTextures() = 0;

protected:

	//Use this callback
	static int rtAudioCallback( void* _outputBuffer, void* _inputBuffer, unsigned int _nBufferFrames,
															double _streamTime, RtAudioStreamStatus _status, void* _userData );

	static void rtAudioError(RtAudioError::Type _type, const std::string &_errorText);

	// RtAudio
	size_t m_bytesPerFrame; //Rt audio bytes per frame
	RtAudio m_audio;

	// Usefull variables
	QOpenGLTexture m_noteVelocityTexture;
	QOpenGLTexture m_maxNoteVelocityTexture;
};

class UserSettedLengthMusic : public Music
{
  Q_OBJECT

public:
	UserSettedLengthMusic(QObject* parent);
	qreal m_length;

	inline qreal getLength() const override { return m_length; }
};

#endif // !MUSIC_HPP
