#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <QMutex>
#include <QOpenGLTexture>

#include "RtAudio.h"
#include "mediafile.hpp"


class RtAudio;
class Timeline;
class Music : public MediaFile
{
	Q_OBJECT

public:
  Music(QObject* parent);
  virtual ~Music();

  virtual double getPosition() const = 0;
  virtual double getLength() const = 0;

	inline const QOpenGLTexture& getNoteVelocityTexture() { return m_noteVelocityTexture; }
	inline const QOpenGLTexture& getMaxNoteVelocityTexture() { return m_maxNoteVelocityTexture; }

  virtual void exportMusicCData(const QFile& source, const QFile& header) const = 0;

	// Control
	volatile bool m_playing;

  /*
    RtAudio stuff
  */
  virtual bool createRtAudioStream() = 0;
  virtual void processAudio(void *outputBuffer, unsigned int nBufferFrames,
                            double streamTime, RtAudioStreamStatus status) = 0;

public slots:
  virtual void setPosition(double time) = 0;
  virtual void updateTextures() = 0;

protected:

  //Use this callback
  static int rtAudioCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                              double streamTime, RtAudioStreamStatus status, void *userData );

  static void rtAudioError(RtAudioError::Type type, const std::string &errorText);

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
	double m_length;

	inline double getLength() const override { return m_length; }
};

#endif // !MUSIC_HPP
