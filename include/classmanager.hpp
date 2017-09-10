#ifndef CLASSMANAGER_HPP
#define CLASSMANAGER_HPP

#include <QMap>
#include <QString>

class ClassManager final
{
public:
  ~ClassManager();

  // Statics
  static ClassManager* get();

  // Methods
  void initalizeClasses();
  const QMetaObject* getClass(const QString& className) const;
  const QMetaObject* getContainerTargetClass(int type) const;
  bool isVector(int type) const;
  bool isInt64Map(int type) const;
  bool isStringMap(int type) const;
  bool isContainer(int type) const;

private:
  // Singleton
  ClassManager();
  static ClassManager* createInstance();

  void initializeQObjectClasses();
  void initializeQVectorTypes();
  void initializeQMapInt64Types();
  void initializeQMapStringTypes();

  QMap<int, const QMetaObject*> objectPointerTypes;
  QMap<QString, const QMetaObject*> classes;
  QMap<int, const QMetaObject*> vectorTypes;
  QMap<int, const QMetaObject*> int64MapTypes;
  QMap<int, const QMetaObject*> stringMapTypes;
};


#endif // !CLASSMANAGER_HPP
