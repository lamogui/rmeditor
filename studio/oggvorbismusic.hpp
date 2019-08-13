#ifndef OGGVORBISMUSIC_HPP
#define OGGVORBISMUSIC_HPP

#include "music.hpp"
#include <vorbis\vorbisfile.h>
#include <ogg/ogg.h>

#include <QFile>

class OggVorbisMusic : public Music
{
  Q_OBJECT

public:
	OggVorbisMusic(QObject* parent = nullptr);
	~OggVorbisMusic() override;

	// Music
	qreal getTime() const final;
	qreal getLength() const final;

	void exportMusicCData(const QFile& source, const QFile& header) const override;

public slots:
	//MediaFile
	bool load() override;

	//Music
	void setPosition(double time) override;
	void updateTextures() override;

	bool createRtAudioStream() override;
	void processAudio(void *outputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status) override;

protected:

	static size_t read(void*, size_t, size_t, void*);
	static int seek64(void*, ogg_int64_t, int);
	static int close(void*);
	static long tell(void*);

	void handleOVError(int error) const;

	OggVorbis_File m_vorbisFile;
	QFile m_file;


};

#endif // !OGGVORBISMUSIC_HPP
