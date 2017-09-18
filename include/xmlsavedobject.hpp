
#ifndef XMLSAVEDOBJECT_HPP
#define XMLSAVEDOBJECT_HPP

#include <QObject>
#include <QString>
#include <QVariant>
#include <QDomElement>
#include <QMap>
#include "jassert.hpp"

/*
** Ugly things
*/

template <class cl>
using Int64Map = QMap<qint64, cl>; // Compiler bug with macros because of ',' ! 

template <class cl>
using StringMap = QMap<QString, cl>; // Compiler bug with macros because of ',' ! 

/*
** Properties
*/

#define SET_PROPERTY(Type, variableName) \
  if (new##variableName != variableName) \
  { \
    Type old##variableName = variableName; \
    variableName = new##variableName; \
    Q_EMIT propertyChanged(this, #variableName, QVariant::fromValue(old##variableName), QVariant::fromValue(new##variableName)); \
  }

#define SET_PROPERTY_NOTIFY(Type, variableName) \
  if (new##variableName != variableName) \
  { \
    Type old##variableName = variableName; \
    variableName = new##variableName; \
    Q_EMIT propertyChanged(this, #variableName, QVariant::fromValue(old##variableName), QVariant::fromValue(new##variableName)); \
    Q_EMIT variableName##Changed(new##variableName); \
  } 

#define GENERATE_PROPERTY_GETTER(Type, variableName, getterName) \
  inline Type getterName() const { return variableName; }

#define GENERATE_PROPERTY_REFERENCE_GETTER(Type, variableName, getterName) \
  inline const Type& getterName() const { return variableName; }

#define DECLARE_PROPERTY_SETTER(Type, variableName, setterName) \
  void setterName(Type new##variableName);

#define DECLARE_PROPERTY_SETTER_REFERENCE(Type, variableName, setterName) \
  void setterName(const Type& new##variableName);


#define GENERATE_PROPERTY_SETTER(Class, Type, variableName, VariableName) \
  void Class::set##VariableName(Type new##variableName) \
  { \
    SET_PROPERTY(Type, variableName, variableName##variableName) \
  }

#define GENERATE_PROPERTY_SETTER_NOTIFY(Class, Type, variableName, VariableName) \
  void Class::set##VariableName(Type new##variableName) \
  { \
    SET_PROPERTY_NOTIFY(Type, variableName) \
  }

#define GENERATE_PROPERTY_SETTER_REFERENCE(Class, Type, variableName, VariableName) \
  void Class::set##VariableName(const Type& new##variableName) \
  { \
    SET_PROPERTY(Type, variableName) \
  }

#define GENERATE_PROPERTY_SETTER_REFERENCE_NOTIFY(Class, Type, variableName, VariableName) \
  void Class::set##VariableName(const Type& new##variableName) \
  { \
    SET_PROPERTY_NOTIFY(Type, variableName) \
  }

#define DECLARE_PROPERTY_GETTER_AND_SETTER(Type, variableName, VariableName) \
  public: \
    GENERATE_PROPERTY_GETTER(Type, variableName, get##VariableName) \
    DECLARE_PROPERTY_SETTER(Type, variableName, set##VariableName)  \
  private: \
    Type variableName;

#define DECLARE_PROPERTY_GETTER_AND_SETTER_REFERENCE(Type, variableName, VariableName) \
  public: \
    GENERATE_PROPERTY_REFERENCE_GETTER(Type, variableName, get##VariableName) \
    DECLARE_PROPERTY_SETTER_REFERENCE(Type, variableName, set##VariableName)  \
  private: \
    Type variableName;

#define DECLARE_PROPERTY(Type, variableName, VariableName) \
  Q_PROPERTY(Type variableName MEMBER variableName READ get##VariableName WRITE set##VariableName) \
  DECLARE_PROPERTY_GETTER_AND_SETTER(Type, variableName, VariableName)

#define DECLARE_PROPERTY_NOTIFY(Type, variableName, VariableName) \
  Q_PROPERTY(Type variableName MEMBER variableName READ get##VariableName WRITE set##VariableName NOTIFY variableName##Changed) \
  DECLARE_PROPERTY_GETTER_AND_SETTER(Type, variableName, VariableName);

#define DECLARE_PROPERTY_CONTAINER(ContainerType, ValueType, valueName, ValueName) \
  Q_PROPERTY(ContainerType<ValueType> valueName##s MEMBER valueName##s READ get##ValueName##s) \
  public: \
    GENERATE_PROPERTY_REFERENCE_GETTER(ContainerType<ValueType>, valueName##s, get##ValueName##s) \
    Q_INVOKABLE void insert##ValueName(ValueType valueName); \
    void remove##ValueName(ValueType valueName); \
  private: \
    ContainerType<ValueType> valueName##s;

#define DECLARE_PROPERTY_REFERENCE(Type, variableName, VariableName) \
  Q_PROPERTY(Type variableName MEMBER variableName READ get##VariableName WRITE set##VariableName) \
  DECLARE_PROPERTY_GETTER_AND_SETTER_REFERENCE(Type, variableName, VariableName)

#define DECLARE_PROPERTY_REFERENCE_NOTIFY(Type, variableName, VariableName) \
  DECLARE_PROPERTY_GETTER_AND_SETTER_REFERENCE(Type, variableName, VariableName) \
  Q_PROPERTY(Type variableName MEMBER variableName READ get##VariableName WRITE set##VariableName NOTIFY variableName##Changed)


/*
** Xml
*/

bool LoadObjectFromXmlNode(QObject& object, const QDomNode& node, QString& failureReason, QStringList& warnings);

#define XML_SAVED_OBJECT \
  public: \
    Q_INVOKABLE void setNode(QDomElement n) { node = n; } \
  private slots: \
   void onPropertyChanged(QObject* owner, QString propertyName, QVariant oldValue, QVariant newValue) \
   { \
     jassert(owner == this); \
     Q_EMIT xmlPropertyChanged(node, propertyName, newValue); \
   } \
  private: \
    QDomElement node; 

#define CONNECT_XML_SAVED_OBJECT(className) connect(this, &className::propertyChanged, this, &className::onPropertyChanged) // do this inside the constructor 

#endif // !XMLSAVEDOBJECT_HPP