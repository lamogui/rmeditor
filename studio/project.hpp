

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "mediafile.hpp"
#include "undocommands.hpp"
#include "demotimeline.hpp"

#include <QDir>

class Music;
class RaymarchingScene;
class Project : public MediaFile
{
  Q_OBJECT

public:
  Project(QObject* parent = nullptr);

  static Project* get(const QObject& context);

  // GL 
	void initializeGL(RenderFunctionsCache& _gl);

  //Accessors
	QDir getDir() const;

  //Utils
  void reset(); // reset project without notify 



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

    void exportFrameworkSources(const QDir& dir) const;
    void exportScenesSources(const QDir& dir) const;
    void exportGifsSources(const QDir& dir) const;

  public slots:
    void resetProject();
    virtual bool build(const QString& text);
    void notifyDocumentChanged();
    void destroyNode(QDomNode& node);

    void exportAsLinuxDemo(const QDir& dir) const;

  signals:
    void appendTextEditable(TextEditable* object);
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

	Music* m_music; // TODO do not make a pointer for this, use an aggregation and hide impl
	DemoTimeline m_demoTimeline;
};

#endif
