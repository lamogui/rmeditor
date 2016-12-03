

#include "project.hpp"

#include <QTextStream>

#include "demotimeline.hpp"
#include "framework.hpp"
#include "scene.hpp"
#include "shaderminifier.hpp"
#include "tunefish4music.hpp"


Project::Project(const QDir &dir, const QString &filename, LogWidget &log, QObject *parent):
  TextEditable(filename, QDomNode() ,log,parent),
  m_music(NULL),
  m_demoTimeline(NULL),
  m_dir(dir),
  m_log(&log),
  m_textUpdateTimer(new QTimer(this))
{
  load();
  m_textUpdateTimer->setSingleShot(true);
  m_textUpdateTimer->setInterval(1000);
  connect(m_textUpdateTimer,SIGNAL(timeout()),this,SLOT(computeText()));
}

Project::~Project()
{
  resetProject();
}
/*
bool Project::needTextRefresh() const
{
  bool temp = m_needRefreshText;
  m_needRefreshText = false;
  return temp;
}
*/

const QString& Project::text() const
{
  return m_text;
}

void Project::connectLog(LogWidget &log)
{
  m_log=&log;
  TextEditable::connectLog(log);
}

void Project::resetProject()
{
  m_textUpdateTimer->stop();
  if (m_demoTimeline)
  {
    emit demoTimelineChanged(NULL);
    delete m_demoTimeline;
  }
  m_demoTimeline = NULL;
  //m_text.clear();
  m_node = QDomNode();
  m_document.clear();
  //emit objectTextChanged(m_text);

  QList<QString> keys = m_rmScenes.keys();
  foreach (QString key, keys)
  {
    delete m_rmScenes[key];
  }
  m_rmScenes.clear();
  keys = m_frameworks.keys();
  foreach (QString key, keys)
  {
    delete m_frameworks[key];
  }
  m_frameworks.clear();


  if (m_music)
  {
    delete m_music;
  }
  m_music = NULL;

}

QString Project::nodeTypeToString(QDomNode::NodeType type)
{
  switch (type)
  {
    case QDomNode::ElementNode:
      return QString("Element");
    case QDomNode::AttributeNode:
      return QString("Attribute");
    case QDomNode::TextNode:
      return QString("Text");
    case QDomNode::CDATASectionNode:
      return QString("CDATA Section");
    case QDomNode::EntityReferenceNode:
      return QString("Entity Reference");
    case QDomNode::EntityNode:
      return QString("Entity");
    case QDomNode::ProcessingInstructionNode:
      return QString("Processing Instruction");
    case QDomNode::CommentNode:
      return QString("Comment");
    case QDomNode::DocumentNode:
      return QString("Document");
    case QDomNode::DocumentTypeNode:
      return QString("DocumentType");
    case QDomNode::DocumentFragmentNode:
      return QString("Document Fragment");
    case QDomNode::NotationNode:
      return QString("Notation");
    case QDomNode::BaseNode:
      return QString("Base");
    case QDomNode::CharacterDataNode:
      return QString("Character Data");
    default:
      return QString("Unknow");
  }
}

bool Project::build(const QString &text)
{
  resetProject();
  m_text = text;
  bool buildSuccess = true;
  QString errorMsg;
  int errorLine, errCol;

  if (!m_document.setContent(text,&errorMsg,&errorLine,&errCol))
  {
    emit error(QString("[") + fileName() + "]" + QString(" XML Error at line ") +
               QString::number(errorLine) + " col " + QString::number(errCol) + " " + errorMsg);
    buildSuccess = false;
  }
  else
  {
    QDomNode node = m_document.documentElement().firstChild();
    m_node = node;
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
  //m_needRefreshText = true;

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
      if (m_frameworks.contains(name))
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
        m_frameworks[name] = new Framework(filename,element,*m_log,this);
        m_frameworks[name]->setObjectName(name);
        emit appendTextEditable(m_frameworks[name]);
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
      if (m_rmScenes.contains(name))
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
        Framework* fw=NULL;
        if (!framework.isEmpty())
        {
          auto it = m_frameworks.find(framework);
          if (it==m_frameworks.end())
          {
            emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
                       "(" + element.nodeName() +  ")  '" + name + "' need the framework '" + framework + "' loaded before");
            return false;
          }
          fw = it.value();
        }

        emit info(QString("[") + fileName() + "]" + " loading scene '" + name + "' (file: " + filename + ")");
        m_rmScenes[name] = new Scene(filename,element,fw,*m_log,this);
        m_rmScenes[name]->setObjectName(name);
        emit appendTextEditable(m_rmScenes[name]);
      }
    }
    element = element.nextSiblingElement();
  }
  return true;
}

bool Project::parseTagMusic(QDomNode node)
{
  QDomElement element = node.toElement();
  if (m_music)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ") multiple music !");
    return false;
  }

  QString filename = element.attribute("file");
  QString name = element.attribute("name", filename.section(".",0,-2));
  QString ext = filename.section(".",-1);
  element.setAttribute("name",name);
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

  if (ext == "tf4m")
  {
    m_music = new Tunefish4Music(filename,length,node,*m_log,this);
    emit info(QString("[") + fileName() + "]" + " loading music '" + name + "' (file: " + filename + ")");
    bool s =  m_music->load();
    if (s)
        s = m_music->createRtAudioStream();
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
  if (m_demoTimeline)
  {
    emit error(QString("[") + fileName() + "]" + " error at line " + QString::number(element.lineNumber()) +
              "(" + element.nodeName() +  ") multiple timelines !");
    return false;
  }
  if (!m_music)
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
  m_demoTimeline = new DemoTimeline(element,*this,framerate,*m_log);
  emit demoTimelineChanged(m_demoTimeline);
  return true;
}


Scene* Project::getRayMarchScene(const QString& name) const
{
  QMap<QString,Scene*>::const_iterator it;
  it = m_rmScenes.find(name);
  if (it !=m_rmScenes.end())
  {
    return it.value();
  }
  return NULL;
}


Framework* Project::getFramework(const QString& name) const
{
  QMap<QString,Framework*>::const_iterator it;
  it = m_frameworks.find(name);
  if (it !=m_frameworks.end())
  {
    return it.value();
  }
  return NULL;
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
  m_textUpdateTimer->stop();
  m_textUpdateTimer->start();
}

void Project::computeText()
{
  m_text = m_document.toString(2);
  emit objectTextChanged(m_text);
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
  if (m_demoTimeline)
  {
    m_demoTimeline->exportSources(dir);
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
  for  (it = m_frameworks.constBegin(); it !=m_frameworks.constEnd(); it++)
  {
    ShaderMinifier minifier(*m_log);

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
  header_code << "#define " << scenes_count_str << " " << QString::number(m_rmScenes.size()) << "\n\n";

  QString scenes_src = "const char* const scenes_src[" +  scenes_count_str + "]";
  QString scenes_framework_src = "const char* const scenes_framework_src[" +  scenes_count_str + "]";
  QString scenes_programs = "GLuint* const scenes_programs[" +  scenes_count_str + "]";

  header_code << "extern " << scenes_src << ";\n"; scenes_src += " = {\n";
  header_code << "extern " << scenes_framework_src << ";\n"; scenes_framework_src += " = {\n";
  header_code << "extern " << scenes_programs << ";\n"; scenes_programs += " = {\n";

  QMap<QString,Scene*>::const_iterator it;
  for  (it = m_rmScenes.constBegin(); it !=m_rmScenes.constEnd(); it++)
  {
    ShaderMinifier minifier(*m_log);



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
    if (it == m_rmScenes.constEnd() - 1)
    {
      comma = "";
    }

    scenes_src += "  fs_" + it.value()->objectName() + comma + "\n";
    scenes_programs += "  &fs_" + it.value()->objectName() + "_program" + comma + "\n";
    if (it.value()->framework())
    {
      scenes_framework_src += "  fs_" + it.value()->framework()->objectName() + comma + "\n";
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


