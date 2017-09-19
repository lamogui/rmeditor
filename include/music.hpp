#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <QMutex>
#include <QOpenGLTexture>

#include <rtaudio/RtAudio.h>
#include "mediafile.hpp"


class RtAudio;
class Timeline;
class Music : public MediaFile
{
  Q_OBJECT
  Q_PROPERTY(bool playing MEMBER playing READ isPlaying) // don't use macro because volatile bool + read only + non standard getter name

public:
  Music();
  ~Music() override;

  virtual double getPosition() const = 0;
  virtual double getLength() const = 0;

  inline QWeakPointer<QOpenGLTexture> getNoteVelocityTexture() { return noteVelocityTexture; }

  bool isPlaying() const { return playing; }

  virtual void exportMusicCData(const QFile& source, const QFile& header) const = 0;

  /*
    RtAudio stuff
  */
  virtual bool createRtAudioStream() = 0;
  virtual void processAudio(void *outputBuffer, unsigned int nBufferFrames,
                            double streamTime, RtAudioStreamStatus status) = 0;

public slots:
  virtual bool load() = 0;
  virtual void setPosition(double time) = 0;
  virtual void updateTextures() = 0;

  inline void play() { playing = true; }
  inline void pause() { playing = false;}

protected:

  //Use this callback
  static int rtAudioCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                              double streamTime, RtAudioStreamStatus status, void *userData );

  static void rtAudioError(RtAudioError::Type type, const std::string &errorText);

  // RtAudio
  size_t bytesPerFrame; //Rt audio bytes per frame
  RtAudio audio;

  // Usefull variables
  QSharedPointer<QOpenGLTexture> noteVelocityTexture;

  // Control
  volatile bool playing;
};

class ExternalLengthMusic : public Music
{
  Q_OBJECT

public:
  ExternalLengthMusic();

private:
  DECLARE_PROPERTY(double, length, Length)
};

#endif // !MUSIC_HPP
