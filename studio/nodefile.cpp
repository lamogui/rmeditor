
#include "nodefile.hpp"

#include "logwidget.hpp"

NodeFile::NodeFile(const QString &filename, QDomNode node, LogWidget &log, QObject *parent):
  QFile(filename,parent),
  m_node(node)
{
  connectLog(log);
}


void NodeFile::connectLog(LogWidget& log)
{
  connect(this,SIGNAL(error(QString)),&log,SLOT(writeError(QString)));
  connect(this,SIGNAL(warning(QString)),&log,SLOT(writeWarning(QString)));
  connect(this,SIGNAL(info(QString)),&log,SLOT(writeInfo(QString)));
}
