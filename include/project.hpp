

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

class Project : public TextEditable
{
  Q_OBJECT

  public:
    Project(const QDir& dir, const QString& filename, LogWidget& log,QObject* parent=NULL);
    virtual ~Project();

    virtual const QString& text() const;
    virtual void connectLog(LogWidget& log);

    Scene *getRayMarchScene(const QString& name) const;
    Framework *getFramework(const QString& name) const;
    inline const QMap<QString,Scene*>& rayMarchScenes() const {return m_rmScenes;}
    inline const QMap<QString,Framework*>& frameworks() const {return m_frameworks;}
    static QString nodeTypeToString(QDomNode::NodeType type);


    static QString getDefaultProjectText();

    virtual bool buildable() const { return true; }

    inline Music* music() const { return m_music; }
    inline DemoTimeline* demoTimeline() const { return m_demoTimeline; }

    inline QDomDocument& document() { return m_document; }

    void exportFrameworkSources(const QDir& dir) const;
    void exportScenesSources(const QDir& dir) const;


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

    QMap<QString,Scene*> m_rmScenes;
    QMap<QString,Framework*> m_frameworks;

    Music* m_music;
    DemoTimeline* m_demoTimeline;
    QString m_text;
    QDir m_dir;
    LogWidget* m_log;
    QDomDocument m_document;
    QTimer* m_textUpdateTimer;
};

#endif
