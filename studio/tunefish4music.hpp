
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
	Tunefish4Music(const QString& _filename, qreal _length, QDomNode _node ,LogWidget& _log, QObject* _parent);
  virtual ~Tunefish4Music();

	qreal getTime() const override;

	void exportMusicCData(const QFile& source, const QFile& header) const override;

  /*
    RtAudio stuff
  */
	bool createRtAudioStream() override;
	void processAudio(void* _outputBuffer, unsigned int _nBufferFrames,
										double _streamTime, RtAudioStreamStatus _status) override;

  size_t instrumentCount() const { return m_player.song.instrCount; }



public slots:
	bool load() override;
	void setPosition(qreal _time) override;
	void updateTextures() override;

protected:
	eTfPlayer m_player;
	std::vector<eF32> m_noteVelocityBuffer;
	std::vector<eF32> m_maxNoteVelocityBuffer;
	QMutex m_mutex;

};

#endif
