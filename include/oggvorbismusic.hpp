#ifndef OGGVORBISMUSIC_HPP
#define OGGVORBISMUSIC_HPP

#include "music.hpp"

class OggVorbisMusic : public Music
{
  Q_OBJECT

public:
  OggVorbisMusic(QObject* parent = nullptr);
  
  double getPosition() const final;
  double getLength() const final;

};

#endif // !OGGVORBISMUSIC_HPP