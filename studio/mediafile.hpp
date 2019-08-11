#ifndef MEDIAFILE_HPP
#define MEDIAFILE_HPP

#include <QFileInfo>

#include "camera.hpp" // QSharedPointer fail !
#include "renderer.hpp"
#include "renderfunctionscache.hpp"

/*
** MediaFile : Any media file, it' could be rendered or not and could have a fixed length or not...
*/
class MediaFile : public QObject 
{
  Q_OBJECT

public:
	MediaFile(QObject* parent = nullptr);

  // Renderer
	void initializeGL(RenderFunctionsCache& _gl); // Still needed ? git rid of this shit no ?
	Renderer* m_previewRenderer;

	inline const QFileInfo& getPath() const { return m_path; }

public slots:
  virtual bool load() = 0; // FIXME : one day make a reload slot which should call "reset" before load and put this as protected load virtual 

signals: 
  void pathChanged(QFileInfo); // Meta compiler doesn't support signals declared inside macros !

private:
	QFileInfo m_path;
};

#endif // !MEDIAFILE_HPP
