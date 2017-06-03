#ifndef MEDIAFILE_HPP
#define MEDIAFILE_HPP

#include <QDomNode>
#include <QFileInfo>
#include "camera.hpp" // QSharedPointer fail !
#include "renderer.hpp"

class MediaFile : public QObject /* Media */
{

  Q_OBJECT
  Q_PROPERTY(QFileInfo path MEMBER path READ getPath WRITE setPath NOTIFY pathChanged)

public:
  MediaFile();
  MediaFile(const MediaFile& other);

  // File Path
  inline const QFileInfo& getPath() const { return path; }
  void setPath(const QFileInfo& newPath);

  // Xml
  inline QDomNode getNode() const { return node; } 
  inline void setNode(const QDomNode& n) { node = n; }

  // Renderer
  virtual bool canBeRendered() const { return false; }
  virtual Renderer* createRenderer() const { return nullptr; }
  QWeakPointer<Renderer> getDefaultRenderer() { return defaultRenderer; }
  QWeakPointer<Camera> getDefaultCamera() { return defaultCamera; }

signals:
  // File path 
  void pathChanged(QFileInfo); // slot should not change the path !

  // History
  void propertyChanged(QObject* owner, QString propertyName, QVariant oldValue, QVariant newValue);

  // Xml
  void xmlPropertyChanged(MediaFile* owner, QString propertyName, QVariant newValue);


private slots:
  void onPropertyChanged(QObject* owner, QString propertyName, QVariant oldValue, QVariant newValue); // not automatic slot

protected:
  QDomNode node; /* Physical link to the xml project file */
  QSharedPointer<Renderer> defaultRenderer; /* allow to see the media even without a timeline */
  QSharedPointer<Camera> defaultCamera;

private:
  QFileInfo path; 
};

Q_DECLARE_METATYPE(MediaFile);

#endif // !MEDIAFILE_HPP
