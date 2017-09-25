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

double OggVorbisMusic::getPosition() const
{
  if (!file.isOpen())
    return 0;
  return ov_time_tell(&const_cast<OggVorbis_File&>(vorbisFile));
}

double OggVorbisMusic::getLength() const
{
  if (!file.isOpen())
    return 0;
  return ov_time_total(&const_cast<OggVorbis_File&>(vorbisFile), -1);
}

void OggVorbisMusic::exportMusicCData(const QFile& source, const QFile& header) const
{
  jassertfalse; // TODO
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
    Log::Error("[" + getPath().fileName() + "] could not open the file: " + file.errorString());
    return false;
  }

  ov_callbacks callbacks;
  callbacks.read_func = &OggVorbisMusic::read;
  callbacks.seek_func = &OggVorbisMusic::seek64;
  callbacks.close_func = &OggVorbisMusic::close;
  callbacks.tell_func = &OggVorbisMusic::tell;

  int ret = ov_open_callbacks((void*)&file, &vorbisFile, nullptr, 0, callbacks);
  if (ret != 0)
  {
    handleOVError(ret);
    return false;
  }
  
  /* Throw the comments plus a few lines about the bitstream we're
  decoding */
  {
    char **ptr = ov_comment(&vorbisFile, -1)->user_comments;
    vorbis_info *vi = ov_info(&vorbisFile, -1);
    while (*ptr) {
      Log::Info("[" + getPath().fileName() + "] " + *ptr);
      ++ptr;
    }
    Log::Info("[" + getPath().fileName() + "] Bitstream is " + QString::number(vi->channels) + "channel, " + QString::number(vi->rate) + "Hz");
    Log::Info("[" + getPath().fileName() + "] Decoded length: " + QString::number(ov_pcm_total(&vorbisFile, -1)) + " samples");
    Log::Info("[" + getPath().fileName() + "] Encoded by: " + ov_comment(&vorbisFile, -1)->vendor);
  }

  return createRtAudioStream();
}


void OggVorbisMusic::setPosition(double time)
{
  int ret = ov_time_seek(&vorbisFile, time);
  if (ret != 0)
    handleOVError(ret);
}

void OggVorbisMusic::updateTextures()
{
  jassertfalse; // TODO
}

bool OggVorbisMusic::createRtAudioStream()
{
  try
  {
    vorbis_info *vi = ov_info(&vorbisFile, -1);
    RtAudio::StreamParameters parameters;
    parameters.deviceId = audio.getDefaultOutputDevice();
    parameters.nChannels = vi->channels;
    parameters.firstChannel = 0;
    unsigned int sampleRate = vi->rate;
    unsigned int bufferFrames = 512;  // Hum maybe music should control the global framerate and use an appropriate value here ? 
    bytesPerFrame = sizeof(float) * parameters.nChannels; // For Music 
    audio.openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, rtAudioCallback, (void*) this, 0, Music::rtAudioError);
    audio.startStream();
  }
  catch (RtAudioError &err)
  {
    Log::Error("[" + getPath().fileName() + "] " + QString::fromStdString(err.getMessage()));
    return false;
  }
  return true;
}

void OggVorbisMusic::processAudio(void *outputBuffer, unsigned int nBufferFrames, double, RtAudioStreamStatus)
{
  int requestSize = nBufferFrames;
  int startOffset = 0;
  float* out = (float*)outputBuffer;
  while (requestSize > 0)
  {
    float** ov_buffers;
    int currentBitstream;
    int readed = ov_read_float(&vorbisFile, &ov_buffers, requestSize, &currentBitstream);
    if (readed < 0)
    {
      handleOVError(readed);
      break;
    }

    jassert(readed <= requestSize); // wrongly understand the documentation
    
    vorbis_info *vi = ov_info(&vorbisFile, -1);
    for (int c = 0; c < vi->channels; ++c)
    {
      for (int s = 0; s < readed; ++s)
      {
        out[startOffset + s * vi->channels + c] = ov_buffers[c][s];
      }
    }

    startOffset += readed;
    requestSize -= readed;
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

int OggVorbisMusic::seek64(void* f, ogg_int64_t off, int whence)
{
  QFile& file = *(QFile*)f;
  bool success = true;
  switch (whence)
  {
    case SEEK_SET:
      success = file.seek(off);
      break;
    case SEEK_CUR:
      success = file.seek(off + file.pos());
      break;
    case SEEK_END:
      success = file.seek(off + file.size());
      break;
    default:
      jassertfalse;
      return -1;
  }
  if (!success)
    Log::Error("[" + QFileInfo(file.fileName()).fileName() + "] " + file.errorString());
  return -1;
}

int OggVorbisMusic::close(void* f)
{
  QFile& file = *(QFile*)f;
  file.close();
  return 0;
}

long OggVorbisMusic::tell(void* f)
{
  QFile& file = *(QFile*)f;
  return file.pos();
}

void OggVorbisMusic::handleOVError(int error) const
{
  QString err;
  switch (error)
  {
    case OV_EOF: err = "End of file reached !"; break;
    case OV_HOLE: err = "Lost OGG packet !"; break;
    case OV_EREAD: err = "Error while trying reading the file"; break;
    default: jassertfalse; // todo
  }

  Log::Error("[" + QFileInfo(file.fileName()).fileName() + "] " + err);
}


