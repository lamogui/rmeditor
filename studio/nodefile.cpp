
#include "nodefile.hpp"

#include "logwidget.hpp"

NodeFile::NodeFile(const QString &_filename, QDomNode _node, QObject *_parent):
	QFile(_filename,_parent),
	m_node(_node)
{
}
