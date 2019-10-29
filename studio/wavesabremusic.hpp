#ifndef WAVESABREMUSIC_HPP
#define WAVESABREMUSIC_HPP

#include <QtGlobal>

#ifdef Q_OS_WIN

#include "../WaveSabre/WaveSabrePlayerLib/SongRenderer.h"
#include "music.hpp"
#include "logmanager.hpp"

class WaveSabreMusic : public Music
{
	Q_OBJECT

public:
	WaveSabreMusic(const QString& _filename, double _length, QDomNode _node, QObject* _parent);
	~WaveSabreMusic() override;

	double getTime() const override;

	void exportMusicCData(const QFile& source, const QFile& header) const override;

	/*
		RtAudio stuff
	*/
	bool createRtAudioStream() override;
	void processAudio(void * _outputBuffer, unsigned int _nBufferFrames,
										double _streamTime, RtAudioStreamStatus _status) override;

	size_t instrumentCount() const;

public slots:
	bool load() override;
	void setPosition(double _time) override;
	void updateTextures() override;

protected:
	WaveSabrePlayerLib::SongRenderer::Song m_song;
	WaveSabrePlayerLib::SongRenderer * m_renderer;
	QMutex m_mutex;
};

#endif // Q_OS_WIN
#endif // WAVESABREMUSIC_HPP
