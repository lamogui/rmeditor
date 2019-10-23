
#ifndef TUNEFISH4MUSIC_HPP
#define TUNEFISH4MUSIC_HPP

#include "music.hpp"
#include "system.hpp"
#include "tf4.hpp"
#include "tf4player.hpp"

class Tunefish4Music : public Music
{
  Q_OBJECT

public:
	Tunefish4Music(const QString& _filename, double _length, QDomNode _node, QObject* _parent);
  virtual ~Tunefish4Music();

  virtual double getTime() const;

  virtual void exportMusicCData(const QFile& source, const QFile& header) const;

  /*
    RtAudio stuff
  */
  virtual bool createRtAudioStream();
	virtual void processAudio(void * _outputBuffer, unsigned int _nBufferFrames,
														double _streamTime, RtAudioStreamStatus _status);

  size_t instrumentCount() const { return m_player.song.instrCount; }



public slots:
  virtual bool load();
  virtual void setPosition(double time);
  virtual void updateTextures();

protected:
  eTfPlayer m_player;
  std::vector<eF32> m_noteVelocityBuffer;
  std::vector<eF32> m_maxNoteVelocityBuffer;
  QMutex m_mutex;

};

#endif
