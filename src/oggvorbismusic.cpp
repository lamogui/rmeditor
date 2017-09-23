#include "oggvorbismusic.hpp"
#include "logmanager.hpp"

OggVorbisMusic::OggVorbisMusic(QObject* parent)
{

}

OggVorbisMusic::~OggVorbisMusic()
{
  ov_clear(&vorbisFile);
  file.close();
}

bool OggVorbisMusic::load()
{
  ov_clear(&vorbisFile);
  file.close();

  if (!getPath().exists())
  {
    Log::Error("The file \"" + getPath().absoluteFilePath() + "\" doesn't exists !");
    return false;
  }

  file.setFileName(getPath().absoluteFilePath());

  if (!file.open(QIODevice::ReadOnly))
  {
    Log::Error("[" + getPath().fileName() + "] could not open the file: " + file.errorString();
    return false;
  }

  ov_callbacks callbacks;
  callbacks.read_func = &OggVorbisMusic::read;
  callbacks.seek_func = &OggVorbisMusic::seek64;
  callbacks.close_func = &OggVorbisMusic::close;
  callbacks.tell_func = &OggVorbisMusic::tell;

  if (ov_open_callbacks((void*)&file, &vorbisFile, nullptr, 0, callbacks) != 0)
  {

  }


}

size_t OggVorbisMusic::read(void* buffer, size_t size, size_t count, void* f)
{
  QFile& file = *(QFile*)f;
  qint64 res = file.read((char*)buffer, size * count);
  if (res == -1)
    Log::Error("[" + QFileInfo(file.fileName()).fileName() + "] " + file.errorString());
  return res;
}

int OggVorbisMusic::seek64(void* file, ogg_int64_t off, int whence)
{

}

int OggVorbisMusic::close(void*);
long OggVorbisMusic::tell(void*);