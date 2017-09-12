

#include "project.hpp"
#include "music.hpp"
#include "mediafile.hpp"
#include "renderfunctionscache.hpp"
#include "jassert.hpp"

Project::Project(QObject* parent) :
  TextEditable(parent),
  music(nullptr)
{

}

void Project::setMusic(Music* newMusic)
{
  if (music != newMusic)
  {
    Music* oldMusic = music;
    music = newMusic;
    if (oldMusic)
      oldMusic->setParent(nullptr);
    if (music)
      music->setParent(this);
    
    QVariant oldValue = QVariant::fromValue(oldMusic);
    QVariant newValue = QVariant::fromValue(newMusic);

    emit propertyChanged(this, "music", oldValue, newValue);
  }
}

void Project::insertMediaFile(MediaFile* mediaFile)
{
  jassert(mediaFile);
  jassert(mediaFiles.constFind(mediaFile->getPath().fileName()) == mediaFiles.constEnd());

  mediaFile->setParent(this);
  if (renderCache)
  {
    mediaFile->initializeGL(*renderCache);
  }
  mediaFiles.insert(mediaFile->getPath().fileName(), mediaFile);
  emit mediaFileInserted(mediaFile);

  QVariant oldValue;
  QVariant newValue = QVariant::fromValue(mediaFile);
  emit propertyChanged(this, "mediaFiles", oldValue, newValue);
}

void Project::removeMediaFile(MediaFile* mediaFile)
{
  jassert(mediaFile);
  jassert(mediaFiles.constFind(mediaFile->getPath().fileName()) != mediaFiles.constEnd());

  mediaFiles.remove(mediaFile->getPath().fileName());

  QVariant oldValue = QVariant::fromValue(mediaFile);
  QVariant newValue;
  emit propertyChanged(this, "mediaFiles", oldValue, newValue);
}

QDir Project::getDir() const
{
  return getPath().absoluteDir();
}

void Project::reset()
{
  document.clear();
  for (StringMap<MediaFile*>::iterator it = mediaFiles.begin(); it != mediaFiles.end(); ++it)
  {
    delete it.value();
  }
  mediaFiles.clear();
}

bool Project::build(const QString& text)
{
  reset();
  xmlContent = text;
  bool buildSuccess = true;
  QString errorMsg;
  int errorLine, errCol;

  if (!document.setContent(xmlContent, &errorMsg, &errorLine, &errCol))
  {
    emit error(QString("[") + getPath().fileName() + "]" + QString(" XML Error at line ") +
      QString::number(errorLine) + " col " + QString::number(errCol) + ": " + errorMsg);
    buildSuccess = false;
  }
  else
  {
    QDomNode node = document.documentElement().firstChild();
    QString failureReason;
    QStringList warnings;
    buildSuccess = LoadObjectFromXmlNode(*this, node, failureReason, warnings);
    foreach(QString w, warnings)
    {
      emit warning(w);
    }
    if (!buildSuccess)
    {
      emit error(failureReason);
    }
  }
  return buildSuccess;
}

void Project::onPathChanged(QFileInfo newPath)
{
  load();
}

const QString& Project::getText() const
{
  return xmlContent;
}

//#include "tunefish4music.hpp"
//#include "4klangmusic.hpp"
#if 0
Project::Project(const QDir &dir, const QString &filename, LogWidget &log, QObject *parent):
  TextEditable(filename, QDomNode() ,log,parent),
  music(nullptr),
  demoTimeline(nullptr),
  dir(dir),
  log(&log),
  textUpdateTimer(new QTimer(this))
{
  load();
  textUpdateTimer->setSingleShot(true);
  textUpdateTimer->setInterval(1000);
  connect(textUpdateTimer,SIGNAL(timeout()),this,SLOT(computeText()));
}

Project::~Project()
{
  resetProject();
}
/*
bool Project::needTextRefresh() const
{
  bool temp = needRefreshText;
  needRefreshText = false;
  return temp;
}
*/

const QString& Project::getText() const
{
  return text;
}

void Project::connectLog(LogWidget &log)
{
  this->log = &log;
  TextEditable::connectLog(log);
}

void Project::resetProject()
{
  textUpdateTimer->stop();
  if (demoTimeline)
  {
    emit demoTimelineChanged(nullptr);
    delete demoTimeline;
  }
  demoTimeline = nullptr;
  //text.clear();
  node = QDomNode();
  document.clear();
  //emit objectTextChanged(text);

  QList<QString> keys = rmScenes.keys();
  foreach (QString key, keys)
  {
    delete rmScenes[key];
  }
  rmScenes.clear();
  keys = frameworks.keys();
  foreach (QString key, keys)
  {
    delete frameworks[key];
  }
  frameworks.clear();


  if (music)
  {
    delete music;
  }
  music = nullptr;

}

bool Project::build(const QString &text)
{
  resetProject();
  this->text = text;
  bool buildSuccess = true;
  QString errorMsg;
  int errorLine, errCol;

  if (!document.setContent(text,&errorMsg,&errorLine,&errCol))
  {
    emit error(QString("[") + fileName() + "]" + QString(" XML Error at line ") +
               QString::number(errorLine) + " col " + QString::number(errCol) + " " + errorMsg);
    buildSuccess = false;
  }
  else
  {
    QDomNode node = document.documentElement().firstChild();
    node = node;
    while (!node.isNull())
    {
      QDomElement element = node.toElement();
      if (element.isNull())
      {
        emit warning(QString("[") + fileName() + "]" + " warning at line " + QString::number(node.lineNumber()) +
                     "(" + node.nodeName() +  ") excepted '" + Project::nodeTypeToString(QDomNode::ElementNode)
                      + "'' node but got '" + Project::nodeTypeToString(node.nodeType()) + "' ignoring the block");
      }
      else if (element.tagName() == "scenes")
      {
        if (!parseTagScenes(node))
        {
          buildSuccess = false;
        }
      }
      else if (element.tagName() == "music")
      {
        if (!parseTagMusic(node))
        {
          buildSuccess = false;
        }
      }
      else if (element.tagName() == "frameworks")
      {
        if (!parseTagFrameworks(node))
        {
          buildSuccess = false;
        }
      }
      else if (element.tagName() == "timeline")
      {
        if (!parseTagTimeline(node))
        {
          buildSuccess = false;
        }
      }
      else
      {
        emit warning(QString("[") + fileName() + "]" + " warning at line " + QString::number(node.lineNumber()) +
                     + " unknown tag name '" + element.tagName() + "' ignoring the block");
      }
      node = node.nextSibling();
    }
  }
  //needRefreshText = true;

  if (buildSuccess)
  {
    emit info(QString("[") + fileName() + "]" + " build project '" + fileName() + "' success !");
    computeText();
  }
  return buildSuccess;
}

bool Project::parseTagFrameworks(QDomNode node)
{
  QDomElement element = node.firstChildElement();
  while (!element.isNull())
  {
    if (element.tagName() == "framework")
    {
      QString filename = element.attribute("file");
      QString name = element.attribute("name", filename.section(".",0,-2));
      element.setAttribute("name",name);
      if (frameworks.contains(name))
      {
        emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
                   "(" + element.nodeName() +  ") duplicate framework '" + name + "'");
        return false;
      }
      else if (filename.isEmpty())
      {
        emit warning(QString("[") + fileName() + "]" + " warning framework without filename line " + QString::number(element.lineNumber()));
      }
      else if (name.isEmpty())
      {
        emit warning(QString("[") + fileName() + "]" + " warning framework without name line " + QString::number(element.lineNumber()));
      }
      else
      {
        emit info(QString("[") + fileName() + "]" + " loading framework '" + name + "' (file: " + filename + ")");
        frameworks[name] = new Framework(filename,element,*log,this);
        frameworks[name]->setObjectName(name);
        emit appendTextEditable(frameworks[name]);
      }
    }
    element = element.nextSiblingElement();
  }
  return true;
}

bool Project::parseTagScenes(QDomNode node)
{
  QDomElement element = node.firstChildElement();
  while (!element.isNull())
  {
    if (element.tagName() == "scene")
    {
      QString filename = element.attribute("file");
      QString name = element.attribute("name", filename.section(".",0,-2));
      QString framework = element.attribute("framework","");
      element.setAttribute("name",name);
      if (rmScenes.contains(name))
      {
        emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
                   "(" + element.nodeName() +  ") duplicate scene '" + name + "'");
        return false;
      }
      else if (filename.isEmpty())
      {
        emit warning(QString("[") + fileName() + "]" + " warning scene without filename line " + QString::number(element.lineNumber()));
      }
      else
      {
        Framework* fw=nullptr;
        if (!framework.isEmpty())
        {
          auto it = frameworks.find(framework);
          if (it==frameworks.end())
          {
            emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
                       "(" + element.nodeName() +  ")  '" + name + "' need the framework '" + framework + "' loaded before");
            return false;
          }
          fw = it.value();
        }

        emit info(QString("[") + fileName() + "]" + " loading scene '" + name + "' (file: " + filename + ")");
        rmScenes[name] = new Scene(filename,element,fw,*log,this);
        rmScenes[name]->setObjectName(name);
        emit appendTextEditable(rmScenes[name]);
      }
    }
    element = element.nextSiblingElement();
  }
  return true;
}

bool Project::parseTagMusic(QDomNode node)
{
  QDomElement element = node.toElement();
  if (music)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ") multiple music !");
    return false;
  }

  QString filename = element.attribute("file");
  QString lstr = element.attribute("length","");
  bool lok = false;
  double length = lstr.toDouble(&lok);
  if (!lok)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ")  '" + filename + "' require the attribute 'length'");
    return false;
  }
  else if (length <= 0)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ")  '" + filename + "' invalid length '" + QString::number(length) + "'");
    return false;
  }

#ifdef Q_OS_WIN32
  if (filename=="4klang") // Special case for 4klang
  {
      music = new _4KlangMusic(filename,length,node,*log,this);
      return music->load() && music->createRtAudioStream();
  }
#endif // Q_OS_WIN32

  QString name = element.attribute("name", filename.section(".",0,-2));
  QString ext = filename.section(".",-1);
  element.setAttribute("name",name);

  if (ext == "tf4m")
  {
    music = new Tunefish4Music(filename,length,node,*log,this);
    emit info(QString("[") + fileName() + "]" + " loading music '" + name + "' (file: " + filename + ")");
    bool s =  music->load();
    if (s)
        s = music->createRtAudioStream();
    return s;
  }
  else
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ")  '" + filename + "' unsupported files type '" + ext + "'");
    return false;
  }

  return true;
}

bool Project::parseTagTimeline(QDomNode node)
{
  QDomElement element = node.toElement();
  if (demoTimeline)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ") multiple timelines !");
    return false;
  }
  if (!music)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ") you need to include a music before include a timeline");
    return false;
  }
  QString framerate_str = element.attribute("framerate","60.0");

  bool ok = false;
  double framerate = framerate_str.toDouble(&ok);
  if (!ok)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ") invalid framerate '" + framerate_str + "'");
    return false;
  }
  demoTimeline = new DemoTimeline(element,*this,framerate,*log);
  emit demoTimelineChanged(demoTimeline);
  return true;
}


Scene* Project::getRayMarchScene(const QString& name) const
{
  QMap<QString,Scene*>::const_iterator it;
  it = rmScenes.find(name);
  if (it !=rmScenes.end())
  {
    return it.value();
  }
  return nullptr;
}


Framework* Project::getFramework(const QString& name) const
{
  QMap<QString,Framework*>::const_iterator it;
  it = frameworks.find(name);
  if (it !=frameworks.end())
  {
    return it.value();
  }
  return nullptr;
}

QString Project::getDefaultProjectText()
{
  return QString("<?xml version='1.0' encoding='UTF-8'?>\n"
                 "<rmdemo>\n"
                   "<music file='' length='0.0'/>\n"
                   "<frameworks>\n"
                     "<framework file='' name=''/>\n"
                   "</frameworks>"
                   "<scenes>\n"
                       "<scene file='' name='' framework=''/>\n"
                   "</scenes>"
                 "</rmdemo>\n");
}


void Project::notifyDocumentChanged()
{
  textUpdateTimer->stop();
  textUpdateTimer->start();
}

void Project::computeText()
{
  text = document.toString(2);
  emit objectTextChanged(text);
}

void Project::destroyNode(QDomNode &node)
{
  node.parentNode().removeChild(node);
  notifyDocumentChanged();
}

void Project::exportAsLinuxDemo(const QDir &dir) const
{
  emit info(QString("[") + fileName() + "]" + tr("building linux project into directory: ") + dir.path());
  exportFrameworkSources(dir);
  exportScenesSources(dir);
  if (demoTimeline)
  {
    demoTimeline->exportSources(dir);
  }
}

void Project::exportFrameworkSources(const QDir &dir) const
{
  QFile header(dir.absoluteFilePath("frameworks.hpp"));
  QFile source(dir.absoluteFilePath("frameworks.cpp"));

  if (!header.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) || !source.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
  {
    emit error(tr("error: cannot open files for writing"));
    return;
  }

  QTextStream header_code(&header);
  QTextStream source_code(&source);

  source_code << ShaderMinifier::generatedSourceString();
  header_code << ShaderMinifier::generatedHeaderString();

  source_code << "#include \"frameworks.hpp\"\n\n";

  header_code << "#ifndef FRAMEWORKS_H\n";
  header_code << "#define FRAMEWORKS_H\n\n";

  QMap<QString,Framework*>::const_iterator it;
  for  (it = frameworks.constBegin(); it !=frameworks.constEnd(); it++)
  {
    ShaderMinifier minifier(*log);

    header_code << "extern const char* const fs_" << it.value()->objectName() << ";\n";
    header_code << "extern const unsigned int fs_" << it.value()->objectName() << "_len;\n";

    source_code << it.value()->cFormatedShaderCode(minifier);
  }

  header_code << "#endif\n";

  header_code.flush();
  source_code.flush();
}

void Project::exportScenesSources(const QDir &dir) const
{
  QFile header(dir.absoluteFilePath("scenes.hpp"));
  QFile source(dir.absoluteFilePath("scenes.cpp"));

  if (!header.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) || !source.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
  {
    emit error(tr("error: cannot open files for writing"));
    return;
  }

  QTextStream header_code(&header);
  QTextStream source_code(&source);

  source_code << ShaderMinifier::generatedSourceString();
  header_code << ShaderMinifier::generatedHeaderString();

  source_code << "#include \"scenes.hpp\"\n\n";

  QString scenes_count_str = "SCENES_COUNT";

  header_code << "#ifndef SCENES_H\n";
  header_code << "#define SCENES_H\n\n";
  header_code << "#include \"frameworks.hpp\"\n";
  header_code << "#include <GL/gl.h>\n\n";
  header_code << "#define " << scenes_count_str << " " << QString::number(rmScenes.size()) << "\n\n";

  QString scenes_src = "const char* const scenes_src[" +  scenes_count_str + "]";
  QString scenes_framework_src = "const char* const scenes_framework_src[" +  scenes_count_str + "]";
  QString scenes_programs = "GLuint* const scenes_programs[" +  scenes_count_str + "]";

  header_code << "extern " << scenes_src << ";\n"; scenes_src += " = {\n";
  header_code << "extern " << scenes_framework_src << ";\n"; scenes_framework_src += " = {\n";
  header_code << "extern " << scenes_programs << ";\n"; scenes_programs += " = {\n";

  QMap<QString,Scene*>::const_iterator it;
  for  (it = rmScenes.constBegin(); it !=rmScenes.constEnd(); it++)
  {
    ShaderMinifier minifier;
    log->findAndConnectLogSignalsRecursively(minifier);



    /*if (it.value()->framework())
    {
      header_code << "extern const char* fs_" << it.value()->objectName() << "_brut;\n";
      //header_code << "extern const unsigned int fs_" << it.value()->objectName() << "_brut_len;\n";

      header_code << "extern char* fs_" << it.value()->objectName() << ";\n";
      //header_code << "extern unsigned int fs_" << it.value()->objectName() << "_len;\n";

      source_code << "char* fs_" << it.value()->objectName() << " = 0;\n";
      //source_code << "unsigned int fs_" << it.value()->objectName() << "_len=" + + ";\n";

      minifier.cFormatedShaderCode(it.value()->objectName(),QString("fs_") + it.value()->objectName() + "_brut",it.value()->minifiedShaderCode(minifier));
    }
    else*/
    {
      header_code << "extern const char* const fs_" << it.value()->objectName() << ";\n";
      //header_code << "extern const unsigned int fs_" << it.value()->objectName() << "_len;\n";

      source_code << it.value()->cFormatedShaderCode(minifier);
    }

    header_code << "extern GLuint fs_" << it.value()->objectName() << "_program;\n";
    source_code << "GLuint fs_" << it.value()->objectName() << "_program;\n";

    QString comma = ",";
    if (it == rmScenes.constEnd() - 1)
    {
      comma = "";
    }

    scenes_src += "  fs_" + it.value()->objectName() + comma + "\n";
    scenes_programs += "  &fs_" + it.value()->objectName() + "_program" + comma + "\n";
    if (it.value()->getFramework())
    {
      scenes_framework_src += "  fs_" + it.value()->getFramework()->objectName() + comma + "\n";
    }
    else
    {
      scenes_framework_src += "  0 " + comma + "\n";
    }


  }

  source_code << scenes_src << "};\n";
  source_code << scenes_framework_src << "};\n";
  source_code << scenes_programs << "};\n";

  header_code << "#endif\n";

  header_code.flush();
  source_code.flush();
}


#endif 