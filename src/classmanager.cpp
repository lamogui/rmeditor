
// Dependencies
#include <QMetaType>
#include <QVector>
#include "classmanager.hpp"
#include "singleton.h"

// declared classes 
#include "camera.hpp"  // for CameraKeyframe
#include "sequence.hpp"
#include "timelinetrack.hpp"


#define DECLARE_QOBJECT_CLASS(CLASSNAME) classes.insert(#CLASSNAME, &CLASSNAME::staticMetaObject) 
#define DECLARE_QOBJECT_INT64MAP_TYPE(CLASSNAME) int64MapTypes.insert(qRegisterMetaType<QMap<qint64, CLASSNAME*> >(), &CLASSNAME::staticMetaObject)

/*
** Class manager
*/

ClassManager::ClassManager()
{
}

ClassManager::~ClassManager()
{
}

ClassManager* ClassManager::createInstance()
{
  return new ClassManager();
}

ClassManager* ClassManager::get()
{
  return Singleton<ClassManager>::instance(ClassManager::createInstance);
}

const QMetaObject* ClassManager::getClass(const QString& className) const
{
  QMap<QString, const QMetaObject*>::const_iterator it = classes.constFind(className);
  if (it != classes.constEnd())
  {
    return it.value();
  }
  return nullptr;
}

bool ClassManager::isVector(int type) const
  {return vectorTypes.contains(type);}

bool ClassManager::isInt64Map(int type) const
  {return int64MapTypes.contains(type);}

void ClassManager::initalizeClasses()
{
  initializeQObjectClasses();
  initializeQVectorTypes();
  initializeQMapInt64Types();
}

void ClassManager::initializeQObjectClasses()
{
  DECLARE_QOBJECT_CLASS(TimelineTrack);
  DECLARE_QOBJECT_CLASS(Sequence);
}

void ClassManager::initializeQVectorTypes()
{

}

void ClassManager::initializeQMapInt64Types()
{
  DECLARE_QOBJECT_INT64MAP_TYPE(Sequence);
}