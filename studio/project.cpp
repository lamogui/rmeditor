﻿

#include "project.hpp"

#include <QTextStream>

#include "demotimeline.hpp"
#include "framework.hpp"
#include "scene.hpp"
#include "shaderminifier.hpp"
#include "tunefish4music.hpp"
#include "giftexture.hpp"
#include "logmanager.hpp"

Project::Project(const QDir & _dir, const QString & _filename, QObject * _parent):
	TextEditable(_filename, QDomNode(), _parent),
  m_music(nullptr),
  m_demoTimeline(nullptr),
	m_dir(_dir),
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

void Project::resetProject()
{
  m_textUpdateTimer->stop();
  if (m_demoTimeline)
  {
    emit demoTimelineChanged(nullptr);
    delete m_demoTimeline;
  }
  m_demoTimeline = nullptr;
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
  keys = m_gifs.keys();
  foreach (QString key, keys)
  {
    delete m_gifs[key];
  }
  m_gifs.clear();
  m_textures.clear();


  if (m_music)
  {
    delete m_music;
  }
  m_music = nullptr;

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
		//default: //Removed because it cause a warning with qt creator !
		//  return QString("Unknow");
	}
	return QString("Unknow");
}

bool Project::build(const QString &_text)
{
	resetProject();
	m_text = _text;
	bool buildSuccess = true;
	QString errorMsg;
	int errorLine, errCol;

	if (!m_document.setContent(_text,&errorMsg,&errorLine,&errCol))
	{
		perror( Log::File, this, tr( "XML Error at line ") +
							 QString::number(errorLine) + " col " + QString::number(errCol) + " " + errorMsg );
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
				pwarning( Log::File, this, tr("warning at line ") + QString::number(node.lineNumber()) +
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
			else if (element.tagName() == "resources")
			{
					if (!parseTagResources(node)){
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
				pwarning( Log::File, this, tr("warning at line ") + QString::number(node.lineNumber()) +
										 + " unknown tag name '" + element.tagName() + "' ignoring the block");
			}
			node = node.nextSibling();
		}
	}
	//m_needRefreshText = true;

	if (buildSuccess)
	{
		pinfo( Log::File, this, tr("build project '") + fileName() + "' success !");
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
				perror( Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
                   "(" + element.nodeName() +  ") duplicate framework '" + name + "'");
        return false;
      }
      else if (filename.isEmpty())
      {
				pwarning( Log::File, this, tr("warning framework without filename line ") + QString::number(element.lineNumber() ) );
      }
      else if (name.isEmpty())
      {
				pwarning( Log::File, this, tr("warning framework without name line ") + QString::number(element.lineNumber() ) );
      }
      else
      {
				pinfo( Log::File, this, tr("loading framework '") + name + "' (file: " + filename + ")" );
				m_frameworks[name] = new Framework(filename,element,this);
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
				perror( Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
									 "(" + element.nodeName() +  ") duplicate scene '" + name + "'" );
				return false;
			}
			else if (filename.isEmpty())
			{
				pwarning( Log::File, this, tr("warning scene without filename line ") + QString::number(element.lineNumber() ) );
			}
			else
			{
				Framework* fw=nullptr;
				if (!framework.isEmpty())
				{
					auto it = m_frameworks.find(framework);
					if (it==m_frameworks.end())
					{
						perror( Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
											 "(" + element.nodeName() +  ")  '" + name + "' need the framework '" + framework + "' loaded before");
						return false;
					}
					fw = it.value();
				}

				pinfo( Log::File, this, tr("loading scene '") + name + "' (file: " + filename + ")" );
				m_rmScenes[name] = new Scene(filename,element, *this,fw,this);
				m_rmScenes[name]->setObjectName(name);
				emit appendTextEditable(m_rmScenes[name]);
			}
		}
		element = element.nextSiblingElement();
	}
	return true;
}

bool Project::parseTagResources(QDomNode node)
{
    QDomElement element = node.firstChildElement();
    while (!element.isNull())
    {
      if (element.tagName() == "texture")
      {
        QString filename = element.attribute("file");
        QString name = element.attribute("name", filename.section(".",0,-2));
        element.setAttribute("name",name);
        if (m_textures.contains(name))
        {
					perror( Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
                     "(" + element.nodeName() +  ") duplicate texture '" + name + "'");
          return false;
        }
        else if (filename.isEmpty())
        {
					pwarning(Log::File, this, tr("warning texture without filename line ") + QString::number(element.lineNumber() ) );
        }
        else
        {
					QString ext = filename.section(".",-1);
					element.setAttribute("name",name);
					pinfo( Log::File, this, tr("loading texture '") + name + "' (file: " + filename + ")" );
					if (ext == "gif")
					{
						m_gifs[name] = new Gif(name, filename,element,this);
						m_textures[name] = &(m_gifs[name]->texture());
					}
					else
					{
						perror( Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
											"(" + element.nodeName() +  ")  '" + filename + "' unsupported files type '" + ext + "'");
						return false;
					}
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
		perror(Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
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
		perror(Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
							"(" + element.nodeName() +  ")  '" + filename + "' require the attribute 'length'");
		return false;
	}
	else if (length <= 0)
	{
		perror(Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
							"(" + element.nodeName() +  ")  '" + filename + "' invalid length '" + QString::number(length) + "'");
		return false;
	}

	if (ext == "tfm")
	{
		m_music = new Tunefish4Music(filename,length,node,this);
		addMusicTextures();
		pinfo( Log::File, this, tr("loading music '") + name + "' (file: " + filename + ")");
		bool s =  m_music->load();
		if (s) {
			s = m_music->createRtAudioStream();
		}
		return s;
	}
	else
	{
		perror(Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
							"(" + element.nodeName() +  ")  '" + filename + "' unsupported files type '" + ext + "'");
		return false;
	}

}

bool Project::parseTagTimeline(QDomNode _node)
{
	QDomElement element = _node.toElement();
	if (m_demoTimeline)
	{
		perror(Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
							"(" + element.nodeName() +  ") multiple timelines !");
		return false;
	}
	if (!m_music)
	{
		perror(Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
							"(" + element.nodeName() +  ") you need to include a music before include a timeline");
		return false;
	}
	QString framerate_str = element.attribute("framerate","60.0");

	bool ok = false;
	double framerate = framerate_str.toDouble(&ok);
	if (!ok)
	{
		perror(Log::File, this, tr("error at line ") + QString::number(element.lineNumber()) +
							"(" + element.nodeName() +  ") invalid framerate '" + framerate_str + "'");
		return false;
	}
	m_demoTimeline = new DemoTimeline(element,*this,framerate);
	emit demoTimelineChanged(m_demoTimeline);
	return true;
}

void Project::addMusicTextures()
{
	passert( Log::Code, m_music );
	m_textures["notes_velocity"] = &(m_music->noteVelocityTex());
	m_textures["max_notes_velocity"] = &(m_music->maxNoteVelocityTex());
}

Scene* Project::getRayMarchScene(const QString& _name) const
{
	QMap<QString,Scene*>::const_iterator it;
	it = m_rmScenes.find(_name);
	if (it !=m_rmScenes.end())
	{
		return it.value();
	}
	return nullptr;
}


Framework* Project::getFramework(const QString& _name) const
{
	QMap<QString,Framework*>::const_iterator it;
	it = m_frameworks.find(_name);
	if (it !=m_frameworks.end())
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
	m_textUpdateTimer->stop();
	m_textUpdateTimer->start();
}

void Project::computeText()
{
	m_text = m_document.toString(2);
	emit objectTextChanged(m_text);
}

void Project::destroyNode(QDomNode & _node)
{
	_node.parentNode().removeChild( _node );
	notifyDocumentChanged();
}

void Project::exportAsLinuxDemo(const QDir & _dir) const
	{
	pinfo(Log::File, this, tr("building linux project into directory: ") + _dir.path() );
	exportFrameworkSources(_dir);
	exportScenesSources(_dir);
	exportGifsSources(_dir);
	if (m_demoTimeline)
	{
		m_demoTimeline->exportSources(_dir);
	}
}

void Project::exportFrameworkSources(const QDir & _dir) const
{
	QFile header(_dir.absoluteFilePath("frameworks.hpp"));
	QFile source(_dir.absoluteFilePath("frameworks.cpp"));

	if (!header.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) || !source.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		perror(Log::File, this, tr("cannot open files for writing"));
		return;
	}

	QTextStream header_code(&header);
	QTextStream source_code(&source);

	//source_code << ShaderMinifier::generatedSourceString();
	//header_code << ShaderMinifier::generatedHeaderString();
	source_code << "#include \"frameworks.hpp\"\n\n";

	header_code << "#ifndef FRAMEWORKS_H\n";
	header_code << "#define FRAMEWORKS_H\n\n";

	QMap<QString,Framework*>::const_iterator it;
	for  (it = m_frameworks.constBegin(); it !=m_frameworks.constEnd(); it++)
	{
		header_code << "extern const char* const fs_" << it.value()->objectName() << ";\n";
		header_code << "extern const unsigned int fs_" << it.value()->objectName() << "_len;\n";

		source_code << it.value()->cFormatedShaderCode();
	}

	header_code << "#endif\n";

	header_code.flush();
	source_code.flush();
}

void Project::exportScenesSources(const QDir &_dir) const
{
	QFile header(_dir.absoluteFilePath("scenes.hpp"));
	QFile source(_dir.absoluteFilePath("scenes.cpp"));

	if (!header.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) || !source.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		perror(Log::File, this, tr("error: cannot open files for writing"));
		return;
	}

	QTextStream header_code(&header);
	QTextStream source_code(&source);

	//source_code << ShaderMinifier::generatedSourceString();
	//header_code << ShaderMinifier::generatedHeaderString();

	source_code << "#include \"scenes.hpp\"\n\n";

	QString scenes_count_str = "SCENES_COUNT";

	header_code << "#ifndef SCENES_H\n";
	header_code << "#define SCENES_H\n\n";
	header_code << "#include \"frameworks.hpp\"\n";
#ifdef _WIN32
	header_code << "#include <Windows.h>\n\n";
#endif
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
		header_code << "extern const char* const fs_" << it.value()->objectName() << ";\n";
		source_code << it.value()->cFormatedShaderCode();

		header_code << "extern GLuint fs_" << it.value()->objectName() << "_program;\n";
		source_code << "GLuint fs_" << it.value()->objectName() << "_program;\n";

		QString comma = ",";
		if (it+1 == m_rmScenes.constEnd())
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

void Project::exportGifsSources(const QDir& _dir) const
{
	QFile header(_dir.absoluteFilePath("gifs.hpp"));
	QFile source(_dir.absoluteFilePath("gifs.cpp"));

	if (!header.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) || !source.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		perror(Log::File, this, tr("error: cannot open files for writing"));
		return;
	}

	QTextStream header_code(&header);
	QTextStream source_code(&source);

	//source_code << ShaderMinifier::generatedSourceString();
	//header_code << ShaderMinifier::generatedHeaderString();

	source_code << "#include \"gifs.hpp\"\n\n";

	QString gifs_count_str = "GIFS_COUNT";

	header_code << "#ifndef GIFS_H\n";
	header_code << "#define GIFS_H\n\n";
#ifdef _WIN32
	header_code << "#include <Windows.h>\n\n";
#endif
	header_code << "#include <GL/gl.h>\n\n";

	header_code << "#define " << gifs_count_str << " " << QString::number(m_gifs.size()) << "\n\n";

	QString gifs_palettes = " unsigned char*  gifs_palettes[" +  gifs_count_str + "]";
	QString gifs_names = " char* gifs_names[" +  gifs_count_str + "]";
	QString gifs_images = " unsigned char*  gifs_images[" +  gifs_count_str + "]";
	QString gifs_images_width = " unsigned int  gifs_images_width[" +  gifs_count_str + "]";
	QString gifs_images_height = " unsigned int  gifs_images_height[" +  gifs_count_str + "]";
	QString gifs_textures = "GLuint gifs_textures[" +  gifs_count_str + "]";

	header_code << "extern " << gifs_palettes << ";\n";
	header_code << "extern " << gifs_names << ";\n";
	header_code << "extern " << gifs_images << ";\n";
	header_code << "extern " << gifs_images_width << ";\n";
	header_code << "extern " << gifs_images_height << ";\n";
	header_code << "extern " << gifs_textures << ";\n";

	gifs_palettes += "= {";
	gifs_names += "= {";
	gifs_images += "= {";
	gifs_images_width += "= {";
	gifs_images_height += "= {";
	gifs_textures += ";";
	QMap<QString,Gif*>::const_iterator it;
	for  (it = m_gifs.constBegin(); it !=m_gifs.constEnd(); it++)
	{
		const gif* g = it.value()->data();
		if (!g->images || g->images_count != 1) {
			perror(Log::File, this, tr("unsupported gif ") + it.value()->fileName() + " with multiple images !");
			return;
		}
		gif_image* f = f=g->images[0];
		if (f->local_color_table_size && f->local_color_table) {
			source_code << " unsigned char " << (it.key() + "_palette[" + QString::number(f->local_color_table_size<<2)+ "] = {");
			const unsigned int size = f->local_color_table_size << 2;
			unsigned i=0,j=0;
			for (;i<size;i+=4)
			{
				if (f->control && f->control->transparent_flag && f->control->transparent_color_index == (j-1) / 3)
				{
					source_code << QString::number(static_cast<int>(0))+ ",";
					source_code << QString::number(static_cast<int>(0))+ ",";
					source_code << QString::number(static_cast<int>(0))+ ",";

					source_code << QString::number(static_cast<int>(0)); // TODO do this operation in the player...
					j+=3;
				}
				else {
					source_code << QString::number(static_cast<int>(f->local_color_table[j++])) + ",";
					source_code << QString::number(static_cast<int>(f->local_color_table[j++])) + ",";
					source_code << QString::number(static_cast<int>(f->local_color_table[j++])) + ",";
					source_code << QString::number(static_cast<int>(255));
				}

				if (i+4 != size) {
					source_code << ",";
				}
			}
			source_code <<"};\n";
		}
		else if (g->global_color_table_size && g->global_color_table) {
			source_code << " unsigned char " << (it.key() + "_palette[" + QString::number(g->global_color_table_size<<2)+ "] = {");
			const unsigned int size = g->global_color_table_size << 2;
			unsigned i=0,j=0;
			for (;i<size;i+=4)
			{
				source_code << QString::number(static_cast<int>(g->global_color_table[j++])) + ",";
				source_code << QString::number(static_cast<int>(g->global_color_table[j++])) + ",";
				source_code << QString::number(static_cast<int>(g->global_color_table[j++])) + ",";
				if (f->control && f->control->transparent_flag && f->control->transparent_color_index == (j-1) / 3)
				{
					source_code << QString::number(static_cast<int>(0));
				}
				else {
					source_code << QString::number(static_cast<int>(255));
				}
				if (i+4 != size) {
					source_code << ",";
				}
			}
			source_code <<"};\n";
		}
		else {
			 perror(Log::File, this, tr("unsupported gif ") + it.value()->fileName() + " table shit error let's put a breakpoint your party is fucked up");
		}

		if (f->descriptor.left != 0 ||
				f->descriptor.top != 0 ||
				f->descriptor.width != g->screen_descriptor.width ||
				f->descriptor.height != g->screen_descriptor.height)
		{
			perror(Log::File, this, tr("unsupported sub image format ! put a breakpoint you are fucked up :)") );
			return ;
		}

		source_code << " unsigned char " << (it.key() + "_image [" + QString::number(g->screen_descriptor.width * g->screen_descriptor.height)  + "] = {");

		for (int h = 0 ; h<g->screen_descriptor.height ; ++h )
		{
			for (int w =0; w < g->screen_descriptor.width ; ++w)
			{
				source_code << QString::number(static_cast<int>(f->pixels[w + g->screen_descriptor.width * (g->screen_descriptor.height - h - 1)]));
				unsigned int k = w * h;
				if (k + 1 != g->screen_descriptor.width * g->screen_descriptor.height) {
					source_code << ",";
				}
			}
		}
		source_code << "};\n";


		QString listComma = ",";
		if (it + 1 == m_gifs.constEnd()) {
				listComma = "";
		}
		gifs_palettes += it.key() + "_palette" + listComma;
		gifs_names += "\"" + it.key() + "\"" + listComma;
		gifs_images +=  it.key() + "_image" + listComma;
		gifs_images_width += QString::number(g->screen_descriptor.width)+ listComma;
		gifs_images_height += QString::number(g->screen_descriptor.height)+ listComma;

	}

	source_code << gifs_palettes << "};\n";
	source_code << gifs_names << "};\n";
	source_code << gifs_images << "};\n";
	source_code << gifs_images_width << "};\n";
	source_code << gifs_images_height << "};\n";
	source_code << gifs_textures;// << "};\n";

	header_code << "#endif\n";

	header_code.flush();
	source_code.flush();
}


