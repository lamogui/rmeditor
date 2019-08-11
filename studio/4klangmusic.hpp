
#ifndef _4KLANGMUSIC_HPP
#define _4KLANGMUSIC_HPP

#include <QtGlobal> // for Q_OS_WIN32
#ifdef Q_OS_WIN32

#include "music.hpp"
#include "system.hpp"
#include "tf4.hpp"
#include <thread>



class _4KlangMusic : public Music
{
  Q_OBJECT

public:
  _4KlangMusic(const QString& filename, double length, QDomNode node ,LogWidget& log,QObject* parent);
  virtual ~_4KlangMusic();

  double getTime() const override;

  void exportMusicCData(const QFile& source, const QFile& header) const override;

  /*
    RtAudio stuff
  */
  bool createRtAudioStream() override;
  void processAudio(void *outputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status) override;


public slots:
  bool load() override;
  void setPosition(double time) override;
  void updateTextures() override;

protected:
  float* _buffer; //Song buffer
  uint32_t _nbSamples; //size in "2.*float" of the buffer
  uint32_t _bufferPos; //Position in "float" into the buffer
  bool _ended; //Fetch reached the end...
  QMutex _mutex;
  std::thread _thread;
};

#endif // Q_OS_WIN32

#endif // !_4KLANGMUSIC_HPP
