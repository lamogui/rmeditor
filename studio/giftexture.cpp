
#include "giftexture.hpp"
#include "logmanager.hpp"
#include "gif.hpp"
#include <QFile>

Gif::Gif(const QString& _uniformName, const QString& _filename, QDomNode _node, QObject* _parent) :
		NodeFile(_filename,_node,_parent),
    m_data(nullptr),
		m_uniformName(_uniformName)
{
	fileChanged(_filename);
	m_watcher.addPath(_filename);
	QObject::connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &Gif::fileChanged);
}

Gif::~Gif()
{
	if (m_data)
	{
		delete_gif(m_data);
	}
}


#include <QImage>
void Gif::fileChanged(const QString& filename)
{
	if (m_data) {
		delete_gif(m_data);
		m_data = nullptr;
	}
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		perror( Log::File, this, tr("could not find ") + filename );
	}

	QByteArray data = file.readAll();
	m_data = load_gif_from_stream(reinterpret_cast<int8_t*>(data.data()),static_cast<unsigned int>(data.length()));
	if (!m_data) {
		perror( Log::File, this, tr("could not load ") + filename + " see debugger for details" );
		unsigned char red[4] = {255, 0, 0, 255};
		m_texture.create(1, 1, &red, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	}
	else {
		QByteArray screen(m_data->screen_descriptor.width * m_data->screen_descriptor.height * 4, 0);
		int errorCode = gif_draw_frame(reinterpret_cast<uint8_t*>(screen.data()), m_data, 0, reinterpret_cast<uint8_t*>(screen.data()), GIF_MODE_RGBA);
		if (errorCode != GIF_NO_ERROR) {

				perror( Log::System, this, tr( "could not draw the first frame, error code ") + QString::number(errorCode, 10) );
				unsigned char purple[4] = {255, 0, 255, 255};
				m_texture.create(1, 1, &purple, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);

		} else {
				QByteArray reversed_screen(m_data->screen_descriptor.width * m_data->screen_descriptor.height * 4, 0);
				for (int i = 0; i < m_data->screen_descriptor.height; ++i) {
						memcpy(reversed_screen.data() + m_data->screen_descriptor.width * i * 4, screen.data() + m_data->screen_descriptor.width * (m_data->screen_descriptor.height - i - 1) * 4, m_data->screen_descriptor.width * 4);
				}

				m_texture.create(m_data->screen_descriptor.width, m_data->screen_descriptor.height, reversed_screen.data(), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
				//QImage test(m_data->screen_descriptor.width, m_data->screen_descriptor.height, QImage::Format::Format_RGBA8888);
				//memcpy(test.bits(), screen.data(), screen.length());
				//test.save(filename + ".png");
				//emit info("file saved to " + filename + ".png");
		}
	}
}

