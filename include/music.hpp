#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <QMutex>
#include <QOpenGLTexture>

#include "../rtaudio/RtAudio.h"
#include "mediafile.hpp"


class RtAudio;
class Timeline;
class Music : public MediaFile
{
  Q_OBJECT
  Q_PROPERTY(bool playing MEMBER playing READ isPlaying) // don't use macro because volatile bool + read only + non standard getter name

public:
  Music(QObject* parent);
  virtual ~Music();

  virtual double getPosition() const = 0;
  virtual double getLength() const = 0;

  inline QWeakPointer<QOpenGLTexture> getNoteVelocityTexture() { return noteVelocityTexture; }
  inline QWeakPointer<QOpenGLTexture> getMaxNoteVelocityTexture() { return maxNoteVelocityTexture; }
  bool playing() const { return m_playing; }

  virtual void exportMusicCData(const QFile& source, const QFile& header) const = 0;

  /*
    RtAudio stuff
  */
  virtual bool createRtAudioStream() = 0;
  virtual void processAudio(void *outputBuffer, unsigned int nBufferFrames,
                            double streamTime, RtAudioStreamStatus status) = 0;

public slots:
  virtual void setPosition(double time) = 0;
  virtual void updateTextures() = 0;

  inline void play() { m_playing = true; }
  inline void pause() { m_playing = false;}

protected:

  //Use this callback
  static int rtAudioCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                              double streamTime, RtAudioStreamStatus status, void *userData );

  static void rtAudioError(RtAudioError::Type type, const std::string &errorText);

  // RtAudio
  size_t m_bytesPerFrame; //Rt audio bytes per frame
  RtAudio m_audio;

  // Usefull variables
  QSharedPointer<QOpenGLTexture> m_noteVelocityTexture;
  QSharedPointer<QOpenGLTexture> m_maxNoteVelocityTexture;

  // Control
  volatile bool m_playing;

private:
  DECLARE_PROPERTY(Timeline*, mainTimeline, MainTimeline);
};

class ExternalLengthMusic : public Music
{
  Q_OBJECT

public:
  ExternalLengthMusic(QObject* parent);

private:
  DECLARE_PROPERTY(double, length, Length)
};

#endif // !MUSIC_HPP
