

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "texteditable.hpp"
#include "undocommands.hpp"
#include <QDir>

class Music;
class RaymarchingScene;
class Project : public TextEditable
{
  UNDOCOMMANDS_RECEIVER_OBJECT
  Q_OBJECT

public:
  Project(QObject* parent = nullptr);

  // GL 
  void initializeGL(RenderFunctionsCache& gl) override;

  // TextEditable 
  const QString& getText() const override;
  bool buildable() const { return true; }

  //Accessors
  QDir getDir() const;

  //Utils
  void reset(); // reset project without notify 

public slots:
  // TextEditable
  bool build(const QString& text) override;

signals:
  void mediaFileInserted(MediaFile* mediaFile); // attached target should connect to MediaFile destruction Qt style

private:
  DECLARE_PROPERTY(Music*, music, Music)
  DECLARE_PROPERTY_CONTAINER(StringMap, MediaFile*, mediaFile, MediaFile)
  QDomDocument document;

  QString xmlContent;

private:
  typedef TextEditable BaseClass;

    /*
    Project(const QDir& dir, const QString& filename, LogWidget& log,QObject* parent=nullptr);
    ~Project() override;

    
    void connectLog(LogWidget& log) override;

    Scene *getRayMarchScene(const QString& name) const;
    Framework *getFramework(const QString& name) const;
    inline const QMap<QString,Scene*>& rayMarchScenes() const {return rmScenes;}
    inline const QMap<QString,Framework*>& getFrameworks() const {return frameworks;}
    static QString nodeTypeToString(QDomNode::NodeType type);


    static QString getDefaultProjectText();

    bool buildable() const override { return true; }

    inline Music* getMusic() const { return music; }
    inline DemoTimeline* getDemoTimeline() const { return demoTimeline; }

    inline QDomDocument& getDocument() { return document; }

    void exportFrameworkSources(const QDir& dir) const;
    void exportScenesSources(const QDir& dir) const;


  public slots:
    void resetProject();
    bool build(const QString& text) override;
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

    QMap<QString,Scene*> rmScenes;
    QMap<QString,Framework*> frameworks;

    Music* music;
    DemoTimeline* demoTimeline;
    QString text;
    QDir dir;
    LogWidget* log; 
    QDomDocument document;
    QTimer* textUpdateTimer;*/
};

#endif // !PROJECT_HPP
