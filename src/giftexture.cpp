
#include "giftexture.hpp"
#include "gif.h"
#include <QFile>

Gif::Gif(const QString& uniformName, const QString& filename, QDomNode node, LogWidget& log,QObject* parent) :
    NodeFile(filename,node,log,parent),
    m_data(nullptr),
    m_uniformName(uniformName)
{
    fileChanged(filename);
    m_watcher.addPath(filename);
    QObject::connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &Gif::fileChanged);
}

Gif::~Gif()
{
    if (m_data)
    {
        delete_gif(m_data);
    }
}

void Gif::fileChanged(const QString& filename)
{
    if (m_data) {
        delete_gif(m_data);
        m_data = nullptr;
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error(QString("could not find ") + filename);
    }

    QByteArray data = file.readAll();
    m_data = load_gif_from_stream(reinterpret_cast<int8_t*>(data.data()), data.length());
    if (!m_data) {
        emit error(QString("could not load ") + filename + " see debugger for details");
    }
}

