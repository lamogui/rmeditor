
#include "mediafile.hpp"

MediaFile::MediaFile(QObject* _parent, const QFileInfo& _path) : QObject(_parent), m_path(_path), m_previewRenderer(nullptr)
{
}


