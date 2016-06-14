
#ifndef TUNEFISH4MUSIC_HPP
#define TUNEFISH4MUSIC_HPP

#include "music.hpp"
#include "system.hpp"
#include "tf4.hpp"

class Tunefish4Music : public Music
{
  Q_OBJECT

public:
  Tunefish4Music(const QString& filename, double length, QDomNode node ,LogWidget& log,QObject* parent);
  virtual ~Tunefish4Music();

  virtual double getTime() const;

  virtual void exportMusicCData(const QFile& source, const QFile& header) const;

  /*
    RtAudio stuff
  */
  virtual bool createRtAudioStream();
  virtual void processAudio(void *outputBuffer, unsigned int nBufferFrames,
                            double streamTime, RtAudioStreamStatus status);

  size_t instrumentCount() const { return m_player.song.instrCount; }



public slots:
  virtual bool load();
  virtual void setPosition(double time);
  virtual void updateTextures();

protected:
  eTfPlayer m_player;
  std::vector<eF32> m_noteVelocityBuffer;
  QMutex m_mutex;

};

#endif
