#include "wavesabremusic.hpp"

#ifdef Q_OS_WIN

using namespace WaveSabrePlayerLib;

static WaveSabreCore::Device *SongFactory(SongRenderer::DeviceId id)
{
	switch (id)
	{
	case SongRenderer::DeviceId::Falcon: return new WaveSabreCore::Falcon();
	case SongRenderer::DeviceId::Slaughter: return new WaveSabreCore::Slaughter();
	case SongRenderer::DeviceId::Thunder: return new WaveSabreCore::Thunder();
	case SongRenderer::DeviceId::Scissor: return new WaveSabreCore::Scissor();
	case SongRenderer::DeviceId::Leveller: return new WaveSabreCore::Leveller();
	case SongRenderer::DeviceId::Crusher: return new WaveSabreCore::Crusher();
	case SongRenderer::DeviceId::Echo: return new WaveSabreCore::Echo();
	case SongRenderer::DeviceId::Smasher: return new WaveSabreCore::Smasher();
	case SongRenderer::DeviceId::Chamber: return new WaveSabreCore::Chamber();
	case SongRenderer::DeviceId::Twister: return new WaveSabreCore::Twister();
	case SongRenderer::DeviceId::Cathedral: return new WaveSabreCore::Cathedral();
	case SongRenderer::DeviceId::Adultery: return new WaveSabreCore::Adultery();
	case SongRenderer::DeviceId::Specimen: return new WaveSabreCore::Specimen();
	}
	return nullptr;
}

WaveSabreMusic::WaveSabreMusic(const QString& _filename, double _length, QDomNode _node, QObject* _parent) :
	Music( _filename, _length, _node, _parent),
	m_song({ SongFactory, nullptr }),
	m_renderer( nullptr )
{

}

WaveSabreMusic::~WaveSabreMusic()
{
	delete m_renderer;
	delete m_song.blob;
}

double WaveSabreMusic::getTime() const
{
	ptodo("WaveSabreMusic::getTime");
	return 0.0;
}

void WaveSabreMusic::exportMusicCData(const QFile& source, const QFile& header) const
{
	// TODO
}

bool WaveSabreMusic::createRtAudioStream()
{
	ptodo("WaveSabreMusic::createRtAudioStream");
	return false;
}

void WaveSabreMusic::processAudio(void * _outputBuffer, unsigned int _nBufferFrames,
									double _streamTime, RtAudioStreamStatus _status)
{

}

size_t WaveSabreMusic::instrumentCount() const
{
	ptodo( "make a pull request to expose WaveSabrePlayerLib::SongRenderer::numDevices" );
	return /*m_renderer ? m_renderer->numDevices :*/ 0;
}

bool WaveSabreMusic::load()
{
	ptodo( "WaveSabreMusic::load(" );
	return false;
}

void WaveSabreMusic::setPosition(double _time)
{

}

void WaveSabreMusic::updateTextures()
{

}

#endif // Q_OS_WIN
