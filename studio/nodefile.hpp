#ifndef NODEFILE_HPP
#define NODEFILE_HPP

#include <QDomNode>
#include <QFile>

class LogWidget;

class NodeFile : public QFile
{
	Q_OBJECT

public:
	NodeFile(const QString& _filename, QDomNode _node, QObject* _parent);
	inline QDomNode getNode() const { return m_node; }

	//Do the connections to log
	virtual void connectLog(LogWidget& log);

protected:
	QDomNode m_node;
};

#endif
