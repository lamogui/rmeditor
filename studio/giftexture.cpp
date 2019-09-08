
#include "giftexture.hpp"
#include "gif.h"
#include "logmanager.hpp"
#include <QFile>

GifTexture::GifTexture(const QString& _uniformName, const QString& _filename, QObject* _parent) :
		MediaFile(_parent),
    m_data(nullptr),
		m_texture(QOpenGLTexture::Target2D),
		m_uniformName(_uniformName)
{
	fileChanged(_filename);
	m_watcher.addPath(_filename);
	QObject::connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &GifTexture::fileChanged);
}

GifTexture::~GifTexture()
{
	if (m_data) {
			delete_gif(m_data);
	}
}


#include <QImage>
void GifTexture::fileChanged(const QString& filename)
{
    if (m_data) {
        delete_gif(m_data);
        m_data = nullptr;
    }
		if (m_texture.isCreated()) {
			m_texture.destroy();
		}

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
			  perror( Log::File, this, tr("could not find ") + filename );
    }

		m_texture.create();
		m_texture.setFormat(QOpenGLTexture::RGBA8U);
		m_texture.setWrapMode(QOpenGLTexture::WrapMode::Repeat);
		m_texture.setMinMagFilters(QOpenGLTexture::Filter::LinearMipMapLinear, QOpenGLTexture::Filter::Linear);


    QByteArray data = file.readAll();
    m_data = load_gif_from_stream(reinterpret_cast<int8_t*>(data.data()),static_cast<unsigned int>(data.length()));
    if (!m_data) {
			  perror( Log::File, this, tr("could not load ") + filename + " see debugger for details" );
        unsigned char red[4] = {255, 0, 0, 255};
				m_texture.setSize(1, 1);
				m_texture.setData(QOpenGLTexture::PixelFormat::RGBA, QOpenGLTexture::PixelType::UInt8, red);
    }
    else {
        QByteArray screen(m_data->screen_descriptor.width * m_data->screen_descriptor.height * 4, 0);
        int errorCode = gif_draw_frame(reinterpret_cast<uint8_t*>(screen.data()), m_data, 0, reinterpret_cast<uint8_t*>(screen.data()), GIF_MODE_RGBA);
        if (errorCode != GIF_NO_ERROR) {

					  perror(Log::File, this, QString("could not draw the first frame, error code ") + QString::number(errorCode, 10));
            unsigned char purple[4] = {255, 0, 255, 255};
						m_texture.setSize(1, 1);
						m_texture.setData(QOpenGLTexture::PixelFormat::RGBA, QOpenGLTexture::PixelType::UInt8, purple);

        } else {
            QByteArray reversed_screen(m_data->screen_descriptor.width * m_data->screen_descriptor.height * 4, 0);
            for (int i = 0; i < m_data->screen_descriptor.height; ++i) {
                memcpy(reversed_screen.data() + m_data->screen_descriptor.width * i * 4, screen.data() + m_data->screen_descriptor.width * (m_data->screen_descriptor.height - i - 1) * 4, m_data->screen_descriptor.width * 4);
            }
						m_texture.setSize(m_data->screen_descriptor.width, m_data->screen_descriptor.height);
						m_texture.setData(QOpenGLTexture::PixelFormat::RGBA, QOpenGLTexture::PixelType::UInt8, reversed_screen.data());

            //QImage test(m_data->screen_descriptor.width, m_data->screen_descriptor.height, QImage::Format::Format_RGBA8888);
            //memcpy(test.bits(), screen.data(), screen.length());
            //test.save(filename + ".png");
            //emit info("file saved to " + filename + ".png");
        }
    }

		m_texture.generateMipMaps();
}

