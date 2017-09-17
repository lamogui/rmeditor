
// Dependencies
#include <QMetaType>
#include <QVector>
#include "jassert.hpp"
#include "classmanager.hpp"
#include "singleton.h"

// declared classes 
#include "camera.hpp"  // for CameraKeyframe
#include "mediafile.hpp"
#include "scene.hpp"   // for RaymarchingScene
#include "sequence.hpp"
#include "timelinetrack.hpp"


#define REGISTER_QOBJECT_CLASS(CLASSNAME) classes.insert(#CLASSNAME, &CLASSNAME::staticMetaObject) 
#define REGISTER_QOBJECT_INT64MAP_TYPE(CLASSNAME) int64MapTypes.insert(qRegisterMetaType<QMap<qint64, CLASSNAME*> >(), &CLASSNAME::staticMetaObject)
#define REGISTER_QOBJECT_STRINGMAP_TYPE(CLASSNAME) stringMapTypes.insert(qRegisterMetaType<StringMap<CLASSNAME*> >(), &CLASSNAME::staticMetaObject)

/*
** Declare MetaTypes
*/

Q_DECLARE_METATYPE(QVector<TimelineTrack*>)
Q_DECLARE_METATYPE(StringMap<MediaFile*>)
Q_DECLARE_METATYPE(Int64Map<CameraKeyframe*>)
Q_DECLARE_METATYPE(Int64Map<Sequence*>)

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
  jassertfalse;
  return nullptr;
}

const QMetaObject* ClassManager::getContainerTargetClass(int type) const
{
  QMap<int, const QMetaObject*>::const_iterator it;
  it = vectorTypes.constFind(type);
  if (it != vectorTypes.constEnd())
    return it.value();
  it = int64MapTypes.constFind(type);
  if (it != int64MapTypes.constEnd())
    return it.value();
  it = stringMapTypes.constFind(type);
  if (it != stringMapTypes.constEnd())
    return it.value();
  jassertfalse;
  return nullptr;
}

bool ClassManager::isVector(int type) const
  {return vectorTypes.contains(type);}

bool ClassManager::isInt64Map(int type) const
  {return int64MapTypes.contains(type);}

bool ClassManager::isStringMap(int type) const
  {return stringMapTypes.contains(type);}

bool ClassManager::isContainer(int type) const
  {return isVector(type) || isInt64Map(type) || isStringMap(type);}

void ClassManager::initalizeClasses()
{
  initializeQObjectClasses();
  initializeQVectorTypes();
  initializeQMapInt64Types();
  initializeQMapStringTypes();
}

void ClassManager::initializeQObjectClasses()
{
  REGISTER_QOBJECT_CLASS(TimelineTrack);
  REGISTER_QOBJECT_CLASS(Sequence);
  REGISTER_QOBJECT_CLASS(RaymarchingScene);
}

void ClassManager::initializeQVectorTypes()
{
  
}

void ClassManager::initializeQMapInt64Types()
{
  REGISTER_QOBJECT_INT64MAP_TYPE(Keyframe);
  REGISTER_QOBJECT_INT64MAP_TYPE(Sequence);
}

void ClassManager::initializeQMapStringTypes()
{
  REGISTER_QOBJECT_STRINGMAP_TYPE(MediaFile);
} 