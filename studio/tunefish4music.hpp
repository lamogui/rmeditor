
#ifndef TUNEFISH4MUSIC_HPP
#define TUNEFISH4MUSIC_HPP

#include "music.hpp"
#include "system.hpp"
#include "tf4.hpp"
#include "tf4player.hpp"

class Tunefish4Music : public UserSettedLengthMusic
{
  Q_OBJECT

public:
	Tunefish4Music(Project* _parent, const QFileInfo& _path);
	~Tunefish4Music() override;

	qreal getTime() const override;

	void exportMusicCData(const QFile& _source, const QFile& _header) const override;

	/*
		RtAudio stuff
	*/
	bool createRtAudioStream() override;
	void processAudio(void* _outputBuffer, unsigned int _nBufferFrames,
										double _streamTime, RtAudioStreamStatus _status) override;

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
