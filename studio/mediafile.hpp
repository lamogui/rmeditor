#ifndef MEDIAFILE_HPP
#define MEDIAFILE_HPP

#include <QFileInfo>

#include "camera.hpp" // QSharedPointer fail !
#include "renderer.hpp"
#include "renderfunctionscache.hpp"
#include "xmlsavedobject.hpp"


/*
** MediaFile : Any media file, it' could be rendered or not and could have a fixed length or not...
*/
class MediaFile : public QObject 
{
  Q_OBJECT

public:
  MediaFile(QObject* parent = nullptr);

  // OpenGL
  virtual void initializeGL(RenderFunctionsCache& gl);

  // Renderer
  virtual bool canBeRendered() const { return false; }
  virtual Renderer* createRenderer() const { return nullptr; }
  QWeakPointer<Renderer> getDefaultRenderer() { return defaultRenderer; }
  QWeakPointer<Camera> getDefaultCamera() { return defaultCamera; }

public slots:
  virtual bool load() = 0; // FIXME : one day make a reload slot which should call "reset" before load and put this as protected load virtual 

signals: 
  void pathChanged(QFileInfo); // Meta compiler doesn't support signals declared inside macros !

protected:
  QSharedPointer<Renderer> defaultRenderer; /* allow to see the media even without a timeline */
  QSharedPointer<Camera> defaultCamera;

  // Graphics
  RenderFunctionsCache* renderCache;

private:
  DECLARE_PROPERTY_REFERENCE_NOTIFY(QFileInfo, path, Path)
};

#endif // !MEDIAFILE_HPP
