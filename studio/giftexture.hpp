#ifndef GIFTEXTURE_HPP
#define GIFTEXTURE_HPP

#include "nodefile.hpp"
#include "texture.hpp"
#include <QFileSystemWatcher>
#include "gif.h"

class Gif : public NodeFile
{
    Q_OBJECT

public:
    Gif(const QString& uniformName, const QString& filename, QDomNode node, LogWidget& log,QObject* parent);
    virtual ~Gif();

    Texture2D& texture() {return m_texture;}
    gif* data() {return m_data;}
    const gif* data() const {return m_data;}

public slots:
    void fileChanged(const QString& filename);

private:
    gif* m_data;
    Texture2D m_texture;
    QFileSystemWatcher m_watcher; // TODO move this global...
    QString m_uniformName;
};

#endif // GIFTEXTURE_H
