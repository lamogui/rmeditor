#ifndef GIFTEXTURE_HPP
#define GIFTEXTURE_HPP

#include "mediafile.hpp"
#include <QFileSystemWatcher>
#include <QOpenGLTexture>
#include "gif.h"



class Gif : public MediaFile
{
	Q_OBJECT

public:
	Gif(const QString& uniformName, const QString& filename, QObject* parent);
	virtual ~Gif();

	QOpenGLTexture& getTexture() {return m_texture;}
	const gif* getData() const {return m_data;}

public slots:
	void fileChanged(const QString& filename);

private:
	gif* m_data;
	QOpenGLTexture m_texture;
	QFileSystemWatcher m_watcher; // TODO move this global...
	QString m_uniformName;
};

#endif // GIFTEXTURE_H
