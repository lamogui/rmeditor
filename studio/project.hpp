

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "texteditable.hpp"
#include "undocommands.hpp"
#include <QDir>

class Music;
class RaymarchingScene;
class Timeline;
class Project : public TextEditable
{
  Q_OBJECT

public:
  Project(QObject* parent = nullptr);

  static Project* get(const QObject& context);

  // GL 
  void initializeGL(RenderFunctionsCache& gl) override;

  // TextEditable 
  const QString& getText() const override;
  bool buildable() const { return true; }

  //Accessors
  QDir getDir() const;
  QUndoStack& getUndoStack() { return undoStack; }

  //Utils
  void reset(); // reset project without notify 

public slots:
  // TextEditable
  bool build(const QString& text) override;

signals:
  void mediaFileInserted(MediaFile* mediaFile); // attached target should connect to MediaFile destruction Qt style
  void mainTimelineChanged(Timeline* newTimeline);

private:
  DECLARE_PROPERTY(Music*, music, Music)
  DECLARE_PROPERTY_CONTAINER(StringMap, MediaFile*, mediaFile, MediaFile)
  QDomDocument document;

  QString xmlContent;
  QUndoStack undoStack; 

private:
  typedef TextEditable BaseClass;

    /*
    Project(const QDir& dir, const QString& filename, LogWidget& log,QObject* parent=nullptr);
    virtual ~Project();

    
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

    Music* m_music;
    DemoTimeline* m_demoTimeline;
    QString m_text;
    QDir m_dir;
    LogWidget* m_log; 
    QDomDocument m_document;
    QTimer* m_textUpdateTimer;*/
};

#endif
