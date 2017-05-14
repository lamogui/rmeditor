
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

  double getTime() const override;

  void exportMusicCData(const QFile& source, const QFile& header) const override;

  /*
    RtAudio stuff
  */
  bool createRtAudioStream() override;
  void processAudio(void *outputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status) override;

  size_t instrumentCount() const { return m_player.song.instrCount; }



public slots:
  bool load() override;
  void setPosition(double time) override;
  void updateTextures() override;

protected:
  eTfPlayer m_player;
  std::vector<eF32> m_noteVelocityBuffer;
  QMutex m_mutex;

};

#endif
