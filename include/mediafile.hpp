#ifndef MEDIAFILE_HPP
#define MEDIAFILE_HPP

#include <QFileInfo>

#include "camera.hpp" // QSharedPointer fail !
#include "renderer.hpp"
#include "renderfunctionscache.hpp"
#include "xmlsavedobject.hpp"

class MediaFile : public QObject /* Media */
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT

public:
  MediaFile();

  // OpenGL
  virtual void initializeGL(RenderFunctionsCache& gl);

  // Renderer
  virtual bool canBeRendered() const { return false; }
  virtual Renderer* createRenderer() const { return nullptr; }
  QWeakPointer<Renderer> getDefaultRenderer() { return defaultRenderer; }
  QWeakPointer<Camera> getDefaultCamera() { return defaultCamera; }

protected:
  QSharedPointer<Renderer> defaultRenderer; /* allow to see the media even without a timeline */
  QSharedPointer<Camera> defaultCamera;

private:
  DECLARE_PROPERTY_REFERENCE_NOTIFY(QFileInfo, path, Path)
};

#endif // !MEDIAFILE_HPP
