#ifndef NODEFILE_HPP
#define NODEFILE_HPP

#include <QDomNode>
#include <QFile>

class NodeFile : public QFile
{
	Q_OBJECT

public:
	NodeFile(const QString& _filename, QDomNode _node, QObject* _parent);
	inline QDomNode getNode() const { return m_node; }

protected:
	QDomNode m_node;
};

#endif
