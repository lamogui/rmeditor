
#ifndef XMLSAVEDOBJECT_HPP
#define XMLSAVEDOBJECT_HPP

// TODO Delete this once we are done with the XML :)

class QObject;
class QString;
class QDomNode;
class QStringList;

/*
** Xml
*/

bool LoadObjectFromXmlNode(QObject& object, const QDomNode& node, QString& failureReason, QStringList& warnings);


#endif // !XMLSAVEDOBJECT_HPP
