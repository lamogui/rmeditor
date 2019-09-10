#ifndef MEDIAFILE_HPP
#define MEDIAFILE_HPP

#include <QFileInfo>
#include "renderfunctionscache.hpp"

class Renderer;

/*
** MediaFile : Any media file, it' could be rendered or not and could have a fixed length or not...
*/
class MediaFile : public QObject 
{
  Q_OBJECT

public:
	MediaFile(QObject* _parent, const QFileInfo& _path);

	// Attribute
	const QFileInfo m_path; // MediaFile is RAII for now

  // Renderer
	void initializeGL(RenderFunctionsCache& _gl); // Still needed ? git rid of this shit no ?
	Renderer* m_previewRenderer;



public slots:
	virtual bool load() = 0; // FIXME : one day make a reload slot which should call "reset" before load and put this as protected load virtual
};

#endif // !MEDIAFILE_HPP
