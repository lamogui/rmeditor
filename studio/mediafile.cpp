
#include "mediafile.hpp"
#include "camera.hpp"
#include "logwidget.hpp"

MediaFile::MediaFile(QObject* parent) : QObject(parent), m_previewRenderer(nullptr)
{
  connect(this, SIGNAL(pathChanged(QFileInfo)), this, SLOT(load()));
}

