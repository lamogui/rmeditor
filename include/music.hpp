#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <QMutex>
#include <QProgressBar>
#include "nodefile.hpp"
#include "texture.hpp"

#include "../rtaudio/RtAudio.h"


class RtAudio;
class Timeline;

class Music : public NodeFile
{
  Q_OBJECT

public:
  Music(const QString& filename, double length, QDomNode node ,LogWidget& log,QObject* parent);
  virtual ~Music();

  virtual double getTime() const = 0;

  inline double length() const { return m_length; }
  inline Texture2D& noteVelocityTex() { return m_noteVelocityTex; }

  bool playing() const { return m_playing; }



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

  inline void play() { m_playing = true; }
  inline void pause() { m_playing = false;}



protected:

  //Use this callback
  static int rtAudioCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                              double streamTime, RtAudioStreamStatus status, void *userData );

  static void rtAudioError(RtAudioError::Type type, const std::string &errorText);

  size_t m_bytesPerFrame; //Rt audio bytes per frame
  RtAudio m_audio;
  Texture2D m_noteVelocityTex;
  double m_length;
  volatile bool m_playing;
};

#endif
