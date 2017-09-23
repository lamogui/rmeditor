#ifndef OGGVORBISMUSIC_HPP
#define OGGVORBISMUSIC_HPP

#include "music.hpp"
#include "vorbis\vorbisfile.h"

#include <ogg/ogg.h>

#include <QFile>

class OggVorbisMusic : public Music
{
  Q_OBJECT

public:
  OggVorbisMusic(QObject* parent = nullptr);
  ~OggVorbisMusic() override;
  
  // Music
  double getPosition() const final;
  double getLength() const final;

public slots:
  //MediaFile
  bool load() override;
  
protected:

  static size_t read(void*, size_t, size_t, void*);
  static int seek64(void*, ogg_int64_t, int);
  static int close(void*);
  static long tell(void*);

  OggVorbis_File vorbisFile;
  QFile file;


};

#endif // !OGGVORBISMUSIC_HPP