#include "oggvorbismusic.hpp"
#include "logmanager.hpp"
#include "project.hpp"

OggVorbisMusic::OggVorbisMusic(Project* _parent, const QFileInfo& _path) :
	Music(_parent,_path),
  m_vorbisFile({0})
{

}

OggVorbisMusic::~OggVorbisMusic()
{
	ov_clear(&m_vorbisFile);
	m_file.close();
}

double OggVorbisMusic::getTime() const
{
	if (!m_file.isOpen())
		return 0;
	return ov_time_tell(&const_cast<OggVorbis_File&>(m_vorbisFile));
}

double OggVorbisMusic::getLength() const
{
	if (!m_file.isOpen())
		return 0;
	return ov_time_total(&const_cast<OggVorbis_File&>(m_vorbisFile), -1);
}

void OggVorbisMusic::exportMusicCData(const QFile& _source, const QFile& _header) const
{
	(void)_source;(void) _header;
	ptodo( "OggVorbisMusic::exportMusicCData" );
}

bool OggVorbisMusic::load()
{
	ov_clear(&m_vorbisFile);
	m_file.close();

	if (!m_path.exists())
	{
		perror( Log::File, this, tr("The file \"") + m_path.absoluteFilePath() + tr("\" doesn't exists !") );
		return false;
	}

	m_file.setFileName(m_path.absoluteFilePath());

	if (!m_file.open(QIODevice::ReadOnly))
	{
		perror( Log::Code, this, tr( "could not open the file: " ) + m_file.errorString());
		return false;
	}

	ov_callbacks callbacks;
	callbacks.read_func = &OggVorbisMusic::read;
	callbacks.seek_func = &OggVorbisMusic::seek64;
	callbacks.close_func = &OggVorbisMusic::close;
	callbacks.tell_func = &OggVorbisMusic::tell;

	int ret = ov_open_callbacks(reinterpret_cast<void*>(&m_file), &m_vorbisFile, nullptr, 0, callbacks);
	if (ret != 0)
	{
		handleOVError(ret);
		return false;
	}

	/* Throw the comments plus a few lines about the bitstream we're
	decoding */
	{
		char **ptr = ov_comment(&m_vorbisFile, -1)->user_comments;
		vorbis_info *vi = ov_info(&m_vorbisFile, -1);
		while (*ptr) {
			pinfo(Log::Audio, this, *ptr);
			++ptr;
		}
		pinfo(Log::Audio, this, tr( "Bitstream is " ) + QString::number(vi->channels) + tr( " channel, " ) + QString::number(vi->rate) + tr( "Hz" ));
		pinfo(Log::Audio, this, tr( "Decoded length: ") + QString::number(ov_pcm_total(&m_vorbisFile, -1)) + tr( " samples") );
		pinfo(Log::Audio, this, tr( "Encoded by: ") + ov_comment(&m_vorbisFile, -1)->vendor);
	}
	return createRtAudioStream();
}


void OggVorbisMusic::setPosition(double time)
{
	if (time < 0.0)
		time = 0.0;
	int ret = ov_time_seek(&m_vorbisFile, time);
	if (ret != 0)
		handleOVError(ret);
}

void OggVorbisMusic::updateTextures()
{
	ptodo("OggVorbisMusic::updateTextures");
}

bool OggVorbisMusic::createRtAudioStream()
{
	try
	{
		vorbis_info* vi = ov_info(&m_vorbisFile, -1);
		RtAudio::StreamParameters parameters;
		parameters.deviceId = m_audio.getDefaultOutputDevice();
		parameters.nChannels = static_cast<unsigned int >(vi->channels);
		parameters.firstChannel = 0;
		unsigned int sampleRate = static_cast<unsigned int >(vi->rate);
		unsigned int bufferFrames = 1024;  // Hum maybe music should control the global framerate and use an appropriate value here ?
		m_bytesPerFrame = sizeof(float) * parameters.nChannels; // For Music
		m_audio.openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, rtAudioCallback, reinterpret_cast<void*>(this), nullptr, Music::rtAudioError);
		m_audio.startStream();
	}
	catch (RtAudioError &err)
	{
		Music::rtAudioError( err.getType(), err.getMessage() );
		return false;
	}
	return true;
}

void OggVorbisMusic::processAudio(void* _outputBuffer, unsigned int _nBufferFrames, double, RtAudioStreamStatus)
{
	int requestSize = static_cast<int>(_nBufferFrames);
	int startOffset = 0;
	float* out = static_cast<float*>(_outputBuffer);
	while (requestSize > 0)
	{
		float** ov_buffers;
		int currentBitstream;
		int readed = ov_read_float(&m_vorbisFile, &ov_buffers, requestSize, &currentBitstream);
		if (readed < 0)
		{
			handleOVError(readed);
			break;
		}

		passertmsg(Log::Code, this, readed <= requestSize, "wrongly understanded the documentation");

		vorbis_info *vi = ov_info(&m_vorbisFile, -1);
		for (int c = 0; c < vi->channels; ++c)
		{
			for (int s = 0; s < readed; ++s)
			{
				out[startOffset + s * vi->channels + c] = ov_buffers[c][s] * 0.5f;
			}
		}

		startOffset += readed * vi->channels;
		requestSize -= readed;
	}
}

size_t OggVorbisMusic::read(void* _buffer, size_t _size, size_t _count, void* _f)
{
	QFile& file = *reinterpret_cast<QFile*>(_f);
	qint64 res = file.read(reinterpret_cast<char*>(_buffer), static_cast<qint64>(_size * _count));
	if (res == -1) {
		perror(Log::File, &file, file.errorString());
	}
	return static_cast<size_t>(res);
}

int OggVorbisMusic::seek64(void* _f, ogg_int64_t _off, int _whence)
{
	QFile& file = *reinterpret_cast<QFile*>(_f);
	bool success = true;
	switch (_whence)
	{
		case SEEK_SET:
			success = file.seek(_off);
			break;
		case SEEK_CUR:
			success = file.seek(_off + file.pos());
			break;
		case SEEK_END:
			success = file.seek(_off + file.size());
			break;
		default:
			ptodo("OggVorbisMusic::seek64");
			return -1;
	}
	if (!success)
	{
		perror(Log::File, &file, file.errorString());
		return -1;
	}
	return 0;
}

int OggVorbisMusic::close(void* _f)
{
	QFile& file = *reinterpret_cast<QFile*>(_f);
  file.close();
  return 0;
}

long OggVorbisMusic::tell(void* _f)
{
	QFile& file = *reinterpret_cast<QFile*>(_f);
	return static_cast<long>(file.pos());
}

void OggVorbisMusic::handleOVError(int _error) const
{
  QString err;
	Log::Category cat = Log::File;
	switch (_error)
  {
    case OV_EOF: err = "End of file reached !"; break;
    case OV_HOLE: err = "Lost OGG packet !"; break;
    case OV_EREAD: err = "Error while trying reading the file"; break;
    case OV_ENOSEEK: err = "Bitstream is not seekable."; break;
		case OV_EINVAL: err = "Incorect value submitted to oggvorbis library."; cat = Log::Code; break;
		default: ptodo("OggVorbisMusic::handleOVError");
  }

	perror(cat, this, err);
}


