

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "mediafile.hpp"
#include "undocommands.hpp"
#include "demotimeline.hpp"

#include <QOpenGLTexture>

class QDDir;
class Music;
class RaymarchingScene;
class Framework;
class GifTexture;
class Project : public MediaFile // TODO do not inherit from media file ?
{
  Q_OBJECT

public:
	Project(QObject* parent = nullptr);

	// GL
	void initializeGL(RenderFunctionsCache& _gl);

	//Accessors
	QDir getDir() const;
	qreal getFramerate() const { return m_framerate; }
	qint64 getNumFrames() const;

	const RaymarchingScene* getRmScene( qint16 _index ) const; // should return nullptr id the index is invalid
	int indexOfRmScene( const RaymarchingScene* ) const;

	// Datas
	Music* m_music; // TODO do not make a pointer for this, use an aggregation and hide impl
	DemoTimeline m_demoTimeline;

	void exportCDatas(const QDir& dir) const;
	void exportFrameworkSources(const QDir& dir) const;
	void exportScenesSources(const QDir& dir) const;
	void exportGifsSources(const QDir& dir) const;

public slots:
	bool loadFromDisk() override;

protected:
	qreal m_framerate;

	QList<RaymarchingScene*> m_rmScenes;
	QList<Framework*> m_frameworks;
	QList<GifTexture*> m_gifs;
	QMap<QString, QOpenGLTexture*> m_textures; // table of all textures used for rendering

	/*
	void connectLog(LogWidget& log) override;

	Scene *getRayMarchScene(const QString& name) const;
	Framework *getFramework(const QString& name) const;
	inline const QMap<QString,Scene*>& rayMarchScenes() const {return m_rmScenes;}
	inline const QMap<QString,Framework*>& frameworks() const {return m_frameworks;}
	inline const QMap<QString, Texture2D*>& textures() const {return m_textures; }
	static QString nodeTypeToString(QDomNode::NodeType type);


	static QString getDefaultProjectText();

	virtual bool buildable() const { return true; }

	inline Music* music() const { return m_music; }
	inline DemoTimeline* demoTimeline() const { return m_demoTimeline; }

	inline QDomDocument& document() { return m_document; }


  public slots:
    void resetProject();
    virtual bool build(const QString& text);
    void notifyDocumentChanged();
    void destroyNode(QDomNode& node);



	signals:
    void demoTimelineChanged(Timeline* t);

  protected slots:
    void computeText();

  private:
    bool parseTagFrameworks(QDomNode node);
    bool parseTagScenes(QDomNode node);
    bool parseTagMusic(QDomNode node);
    bool parseTagTimeline(QDomNode node);
    bool parseTagResources(QDomNode node);

    void addMusicTextures();
*/


};

#endif
