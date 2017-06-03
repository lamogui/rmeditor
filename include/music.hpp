#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <QMutex>
#include <QProgressBar>
#include "nodefile.hpp"
#include "texture.hpp"

#include <rtaudio/RtAudio.h>


class RtAudio;
class Timeline;

class Music : public MediaFile
{
  Q_OBJECT

public:
  Music(const QString& filename, double length, QDomNode node, QObject* parent);
  ~Music() override;

  virtual double getTime() const = 0;

  inline double getLength() const { return length; }
  inline Texture2D& getNoteVelocityTex() { return noteVelocityTex; }

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

  size_t bytesPerFrame; //Rt audio bytes per frame
  RtAudio audio;
  Texture2D noteVelocityTex;
  double length;
  volatile bool playing;
};

#endif
