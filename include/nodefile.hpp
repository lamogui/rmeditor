#ifndef NODEFILE_HPP
#define NODEFILE_HPP

#include <QDomNode>
#include <QFile>

class LogWidget;

class NodeFile : public QFile
{

  Q_OBJECT

public:
  NodeFile(const QString& filename, QDomNode node, LogWidget& log,QObject* parent);
  inline QDomNode getNode() const { return m_node; }

  //Do the connections to log
  virtual void connectLog(LogWidget& log);


signals:
  void error(QString err) const;
  void warning(QString warn) const;
  void info(QString txt) const;

protected:
  QDomNode m_node;
};

#endif
