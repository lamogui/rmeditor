
#include "xmlsavedobject.hpp"
#include "classmanager.hpp"
#include <QMetaProperty>
#include <QVector3D>
#include <QQuaternion>


static QString NodeTypeToString(QDomNode::NodeType type)
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

static bool LoadDoubleFromXmlNode(double& target, const QDomElement& node, QString& failureReason)
{
  QDomNodeList childs = node.childNodes();
  if (childs.size() != 1 || !childs.at(0).isText())
  {
    failureReason = "line " + QString::number(node.lineNumber()) + "(" + node.nodeName() + ") has an invalid value";
    return false;
  }
  bool ok = false;
  target = childs.at(0).nodeValue().toDouble(&ok);
  if (!ok)
  {
    failureReason = "line " + QString::number(node.lineNumber()) + "(" + node.nodeName() + ") is not a number (got " + childs.at(0).nodeValue() + ")";
    return false;
  }
  return true;
}

static bool LoadQVector3DFromXmlNode(QVector3D& target, const QDomElement& node, QString& failureReason)
{
  double x = 0.0, y = 0.0, z = 0.0;
  bool ok = LoadDoubleFromXmlNode(x, node.firstChildElement("x"), failureReason);
  if (ok) ok = LoadDoubleFromXmlNode(y, node.firstChildElement("y"), failureReason);
  if (ok) ok = LoadDoubleFromXmlNode(z, node.firstChildElement("z"), failureReason);
  target.setX(x);
  target.setY(y);
  target.setZ(z);
  return ok;
}

static bool LoadQVector3DFromXmlNode(QVariant& target, const QDomElement& node, QString& failureReason)
{
  QVector3D vector;
  bool success = LoadQVector3DFromXmlNode(vector, node, failureReason);
  target = vector;
  return success;
}

static bool LoadQQuaternionFromXmlNode(QQuaternion& target, const QDomElement& node, QString& failureReason)
{
  double x = 0.0, y = 0.0, z = 0.0, w = 1.0;
  bool ok = LoadDoubleFromXmlNode(x, node.firstChildElement("x"), failureReason);
  if (ok) ok = LoadDoubleFromXmlNode(y, node.firstChildElement("y"), failureReason);
  if (ok) ok = LoadDoubleFromXmlNode(z, node.firstChildElement("z"), failureReason);
  if (ok) ok = LoadDoubleFromXmlNode(w, node.firstChildElement("w"), failureReason);
  target.setX(x);
  target.setY(y);
  target.setZ(z);
  target.setZ(w);
  return ok;
}

static bool LoadQQuaternionFromXmlNode(QVariant& target, const QDomElement& node, QString& failureReason)
{
  QQuaternion quaternion;
  bool success = LoadQQuaternionFromXmlNode(quaternion, node, failureReason);
  target = quaternion;
  return success;
}

static QObject* InstantiateContainedQObjectFromXmlNode(const QMetaObject* targetClass, const QDomNode& node, QString& failureReason, QStringList& warnings)
{

}

bool LoadObjectFromXmlNode(QObject& object, const QDomNode& node, QString& failureReason, QStringList& warnings)
{
  const QMetaObject* cl = object.metaObject();
  QDomNode variableNode = node.firstChild();
  while (!variableNode.isNull())
  {
    QDomElement element = node.toElement();
    if (element.isNull())
    {
      warnings.append("line " + QString::number(node.lineNumber()) +
                      "(" + node.nodeName() + ") excepted '" + NodeTypeToString(QDomNode::ElementNode)
                      + "'' node but got '" + NodeTypeToString(node.nodeType()) + "' ignoring the block");
    }
    else
    {
      QString variableName = element.tagName();
      int variableIndex = cl->indexOfProperty(variableName.toStdString().c_str());
      if (variableIndex == -1)
      {
        warnings.append("line " + QString::number(element.lineNumber()) +
          "(" + element.nodeName() + ") " + cl->className() + " has not property named " + variableName + " it will be skiped");
        continue;
      }
      QMetaProperty variable = cl->property(variableIndex);
      if (variable.type() == QVariant::Invalid)
      {
        failureReason = "line " + QString::number(element.lineNumber()) + "(" + element.nodeName() + ") the property type of " + variableName + " is unknown";
        Q_ASSERT(false); // did you forget to declare QMetaType
        return false;
      }

      if (variable.type() == QVariant::UserType)
      {
        QMetaType type(variable.userType());
        if (type.isValid())
        {
          ClassManager* manager = ClassManager::get();
          if (manager->isContainer(variable.userType()))
          {
            const QMetaObject* targetClass = manager->getContainerTargetClass(variable.userType());
            if (!targetClass)
            {
              failureReason = "line " + QString::number(element.lineNumber()) + "(" + element.nodeName() + ") the type id of container not registered in ClassManager";
              Q_ASSERT(false); // did you forget to register your type in ClassManager ? 
              return false;
            }

            variable.

            QDomElement childElement = element.firstChildElement();
            while (!childElement.isNull())
            {
              QObject* newInstance = InstantiateContainedQObjectFromXmlNode(targetClass, childElement, failureReason, warnings);
              if (!newInstance)
                return false;
              

              element = childElement.nextSiblingElement();
            }
          }
          else
          {
            
          }

        }
        else 
        {
          failureReason = "line " + QString::number(element.lineNumber()) + "(" + element.nodeName() + ") the class type of property is invalid";
          Q_ASSERT(false); // did you forget to declare QMetaType
          return false;
        }
        //const QMetaObject* constructedClass = QMetaType::metaObjectForType(variable.userType());
        
      }
      else 
      {
        QVariant value;
        bool success = false;
        switch (variable.type())
        {
          case QVariant::Vector3D:
            success = LoadQVector3DFromXmlNode(value, element, failureReason);
            break;
          case QVariant::Quaternion:
            success = LoadQQuaternionFromXmlNode(value, element, failureReason);
            break;
          default:
            failureReason = "line " + QString::number(element.lineNumber()) + "(" + element.nodeName() + ") the property type of " + variableName + " is unsupported yet";
            Q_ASSERT(false); // TODO 
            return false;
        }
        if (success)
          variable.write(&object, value);
        else
          return false;
      }
    }
    variableNode.nextSibling();
  }

}