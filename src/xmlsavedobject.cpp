
#include "xmlsavedobject.hpp"
#include "classmanager.hpp"
#include <QMetaProperty>
#include <QVector3D>
#include <QQuaternion>
#include <cassert>


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

static QObject* InstantiateContainedQObjectFromXmlNode(const QMetaObject& targetClass, const QDomNode& node, QString& failureReason, QStringList& warnings)
{
  if (!node.isElement())
  {
    failureReason = "line " + QString::number(node.lineNumber()) +
      " (" + node.nodeName() + ") excepted '" + NodeTypeToString(QDomNode::ElementNode)
      + "'' node but got '" + NodeTypeToString(node.nodeType());
    return nullptr;
  }

  QDomElement element = node.toElement();
  ClassManager* manager = ClassManager::get();
  const QMetaObject* realTargetClass = manager->getClass(element.tagName());
  if (!realTargetClass)
  {
    failureReason = "line " + QString::number(element.lineNumber()) + " (" + node.nodeName() + ") the class " + element.tagName() + " doesn't exists !";
    return nullptr;
  }

  if (!realTargetClass->inherits(&targetClass))
  {
    failureReason = "line " + QString::number(element.lineNumber()) + " (" + node.nodeName() + ") the class " + element.tagName() + " don't inherit from " + targetClass.className();
    return nullptr;
  }

  QObject* newInstance = realTargetClass->newInstance();
  if (!newInstance)
  {
    failureReason = "line " + QString::number(element.lineNumber()) + " (" + node.nodeName() + ") could not instantiate new object of class " + element.tagName();
    assert(false);
    return nullptr;
  }

  if (!LoadObjectFromXmlNode(*newInstance, element, failureReason, warnings))
  {
    delete newInstance;
    return nullptr;
  }
  return newInstance;
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
                      " (" + node.nodeName() + ") excepted '" + NodeTypeToString(QDomNode::ElementNode)
                      + "'' node but got '" + NodeTypeToString(node.nodeType()) + "' ignoring the block");
    }
    else
    {
      QString variableName = element.tagName();
      int variableIndex = cl->indexOfProperty(variableName.toStdString().c_str());
      if (variableIndex == -1)
      {
        warnings.append("line " + QString::number(element.lineNumber()) +
          " (" + element.nodeName() + ") " + cl->className() + " has not property named " + variableName + " it will be skiped");
        continue;
      }
      QMetaProperty variable = cl->property(variableIndex);
      if (variable.type() == QVariant::Invalid)
      {
        failureReason = "line " + QString::number(element.lineNumber()) + " (" + element.nodeName() + ") the property type of " + variableName + " is unknown";
        assert(false); // did you forget to declare QMetaType
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
              failureReason = "line " + QString::number(element.lineNumber()) + " (" + element.nodeName() + ") the type id of container not registered in ClassManager";
              assert(false); // did you forget to register your type in ClassManager ? 
              return false;
            }

            QString containedName = variable.name();
            if (containedName.length())
            {
              containedName[0] = containedName[0].toUpper(); // Camel Case
              if (containedName.length() > 1 && containedName[containedName.length() - 1] == 's')
                containedName.truncate(containedName.length() - 1); // remove plural
            }
            else
              assert(false); // WTF !!!

            const QString insertMethodName = "void insert" + containedName + "(" + targetClass->className() + "*)";
            const int indexOfInsertMethod = cl->indexOfMethod(insertMethodName.toStdString().c_str());
            if (indexOfInsertMethod == -1)
            {
              failureReason = "line " + QString::number(element.lineNumber()) + " (" + element.tagName() + ") could not find required method " + insertMethodName;
              assert(false); // check that the method is slot or Q_INVOKABLE
              return false;
            }

            QMetaMethod insertMethod = cl->method(indexOfInsertMethod);
            assert(insertMethod.isValid()); // wtf ???

            QDomElement childElement = element.firstChildElement();
            while (!childElement.isNull())
            {
              QObject* newInstance = InstantiateContainedQObjectFromXmlNode(*targetClass, childElement, failureReason, warnings);
              if (!newInstance)
                return false;
              
              if (!insertMethod.invoke(&object, Q_ARG(QObject*, newInstance)))
              {
                failureReason = "line " + QString::number(element.lineNumber()) + " (" + element.tagName() + ") invoking " + insertMethodName + " failed !";
                assert(false);
                delete newInstance;
                return false;
              }

              element = childElement.nextSiblingElement();
            }
          }
          else
          {
            assert(false);
          }

        }
        else 
        {
          failureReason = "line " + QString::number(element.lineNumber()) + " (" + element.nodeName() + ") the class type of property is invalid";
          qDebug() << failureReason;
          assert(false); // did you forget to declare QMetaType
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
            failureReason = "line " + QString::number(element.lineNumber()) + " (" + element.nodeName() + ") the property type of " + variableName + " is unsupported yet";
            assert(false); // TODO 
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
  return true;
}