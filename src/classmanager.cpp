
// Dependencies
#include <QMetaType>
#include <QVector>
#include "jassert.hpp"
#include "classmanager.hpp"
#include "singleton.h"

// declared classes 
#include "camera.hpp"  // for CameraKeyframe
#include "demotimeline.hpp"
#include "mediafile.hpp"
#include "music.hpp"
#include "oggvorbismusic.hpp"
#include "scene.hpp"   // for RaymarchingScene
#include "sequence.hpp"
#include "timeline.hpp"
#include "timelinetrack.hpp"


#define REGISTER_QOBJECT_CLASS(CLASSNAME) do {classes.insert(#CLASSNAME, &CLASSNAME::staticMetaObject); objectOwnedPointerTypes.insert(qRegisterMetaType<CLASSNAME*>(), &CLASSNAME::staticMetaObject); } while (false)
#define REGISTER_QOBJECT_INT64MAP_TYPE(CLASSNAME) int64MapTypes.insert(qRegisterMetaType<QMap<qint64, CLASSNAME*> >(), &CLASSNAME::staticMetaObject)
#define REGISTER_QOBJECT_STRINGMAP_TYPE(CLASSNAME) stringMapTypes.insert(qRegisterMetaType<StringMap<CLASSNAME*> >(), &CLASSNAME::staticMetaObject)
#define REGISTER_QOBJECT_VECTOR_TYPE(CLASSNAME) vectorTypes.insert(qRegisterMetaType<QVector<CLASSNAME*> >(), &CLASSNAME::staticMetaObject)

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

bool ClassManager::isOwnedPointer(int type) const
  {return objectOwnedPointerTypes.contains(type);}

void ClassManager::initalizeClasses()
{
  initializeQObjectClasses();
  initializeQVectorTypes();
  initializeQMapInt64Types();
  initializeQMapStringTypes();
}

void ClassManager::initializeQObjectClasses()
{
  REGISTER_QOBJECT_CLASS(DemoTimeline);
  REGISTER_QOBJECT_CLASS(Music);
  REGISTER_QOBJECT_CLASS(OggVorbisMusic);
  REGISTER_QOBJECT_CLASS(RaymarchingScene);
  REGISTER_QOBJECT_CLASS(Sequence);
  REGISTER_QOBJECT_CLASS(Timeline);
  REGISTER_QOBJECT_CLASS(TimelineTrack);
  
  logTypes(objectOwnedPointerTypes);
}

void ClassManager::initializeQVectorTypes()
{
  REGISTER_QOBJECT_VECTOR_TYPE(TimelineTrack);

  logTypes(vectorTypes);
}

void ClassManager::initializeQMapInt64Types()
{
  REGISTER_QOBJECT_INT64MAP_TYPE(Keyframe);
  REGISTER_QOBJECT_INT64MAP_TYPE(Sequence);

  logTypes(int64MapTypes);
}

void ClassManager::initializeQMapStringTypes()
{
  REGISTER_QOBJECT_STRINGMAP_TYPE(MediaFile);

  logTypes(stringMapTypes);
} 

void ClassManager::logTypes(const QMap<int, const QMetaObject*>& types)
{
  QMap<int, const QMetaObject*>::const_iterator it;
  for (it = types.constBegin(); it != types.constEnd(); ++it)
  {
    Log::Info("[ClassManager] registered class " + QString(QMetaType::typeName(it.key())) + " with id " + QString::number(it.key()) + " associated metaobject class " + it.value()->className());
  }
}