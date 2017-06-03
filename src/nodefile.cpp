
#include "nodefile.hpp"
#include "camera.hpp"
#include "logwidget.hpp"

MediaFile::MediaFile() :
  defaultRenderer(createRenderer()),
  defaultCamera(defaultRenderer && defaultRenderer->hasDynamicCamera() ? new Camera : nullptr)
{
  connect(this, &MediaFile::propertyChanged, this, &MediaFile::onPropertyChanged);
}

MediaFile::MediaFile(const MediaFile& other):
  MediaFile()
{
  Q_ASSERT(false); // This constructor should not be called, only needed for registring meta type 

  // only copy properties
  setPath(other.getPath());
}

void MediaFile::setPath(const QFileInfo& newPath)
{
  if (newPath != path)
  {
    QFileInfo oldPath = path;
    path = newPath;
    emit propertyChanged(this, "path", QVariant::fromValue(oldPath), QVariant::fromValue(newPath));
    emit pathChanged(newPath);
  }
}

void MediaFile::onPropertyChanged(QObject* owner, QString propertyName, QVariant, QVariant newValue)
{
  Q_ASSERT(owner == this); // TODO recursive notification
  emit xmlPropertyChanged(this, propertyName, newValue);
}


