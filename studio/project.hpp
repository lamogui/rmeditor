

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QDomDocument>
#include <QMap>
#include <QDir>
#include <QTimer>

#include "texteditable.hpp"

class DemoTimeline;
class Music;
class Framework;
class Scene;
class Timeline;
class Gif;
class Texture2D;

class Project : public TextEditable
{
	Q_OBJECT

public:
	Project(const QDir& _dir, const QString& _filename, QObject* _parent=nullptr);
	virtual ~Project();

	virtual const QString& text() const;

	Scene *getRayMarchScene(const QString& _name) const;
	Framework *getFramework(const QString& _name) const;
	inline const QMap<QString,Scene*>& rayMarchScenes() const {return m_rmScenes;}
	inline const QMap<QString,Framework*>& frameworks() const {return m_frameworks;}
	inline const QMap<QString, Texture2D*>& textures() const {return m_textures; }
	static QString nodeTypeToString(QDomNode::NodeType _type);


	static QString getDefaultProjectText();

	virtual bool buildable() const { return true; }

	inline Music* music() const { return m_music; }
	inline DemoTimeline* demoTimeline() const { return m_demoTimeline; }

	inline QDomDocument& document() { return m_document; }

	void exportFrameworkSources(const QDir& _dir) const;
	void exportScenesSources(const QDir& _dir) const;
	void exportGifsSources(const QDir& _dir) const;

public slots:
	void resetProject();
	virtual bool build(const QString& _text);
	void notifyDocumentChanged();
	void destroyNode(QDomNode& _node);

	void exportAsLinuxDemo(const QDir& _dir) const;

signals:
	void appendTextEditable(TextEditable* _object);
	void demoTimelineChanged(Timeline* _t);

protected slots:
	void computeText();

private:
	bool parseTagFrameworks(QDomNode _node);
	bool parseTagScenes(QDomNode _node);
	bool parseTagMusic(QDomNode _node);
	bool parseTagTimeline(QDomNode _node);
	bool parseTagResources(QDomNode _node);

	void addMusicTextures();

	QMap<QString,Scene*> m_rmScenes;
	QMap<QString,Framework*> m_frameworks;
	QMap<QString, Gif*> m_gifs;
	QMap<QString, Texture2D* > m_textures;

	Music* m_music;
	DemoTimeline* m_demoTimeline;
	QString m_text;
	QDir m_dir;
	QDomDocument m_document;
	QTimer* m_textUpdateTimer;
};
#endif
