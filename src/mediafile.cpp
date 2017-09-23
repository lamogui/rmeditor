
#include "mediafile.hpp"
#include "camera.hpp"
#include "logwidget.hpp"

MediaFile::MediaFile(QObject* parent) : QObject(parent), renderCache(nullptr)
{
  CONNECT_XML_SAVED_OBJECT(MediaFile);
  connect(this, SIGNAL(pathChanged(QFileInfo)), this, SLOT(load()));
}

void MediaFile::initializeGL(RenderFunctionsCache& gl)
{
  renderCache = &gl;
  defaultRenderer.reset(createRenderer());
  if (defaultRenderer)
  {
    defaultRenderer->initializeGL(gl);
    defaultCamera.reset(defaultRenderer->hasDynamicCamera() ? new Camera : nullptr);
  }
}

GENERATE_PROPERTY_SETTER_REFERENCE_NOTIFY(MediaFile, QFileInfo, path, Path)

