#ifndef TIMELINETRACK_HPP
#define TIMELINETRACK_HPP

#include "xmlsavedobject.hpp"

#include <QMap>
#include <QObject>

class Renderer;
class Sequence; 
class DemoTimeline;

/*
** TimelineTrack : a timeline track of a DemoTimeline
*/
class TimelineTrack : public QObject
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT

public:
  TimelineTrack(DemoTimeline& parent); 
  
  

private:
  DECLARE_PROPERTY_REFERENCE(Int64Map<Sequence*>, sequences, Sequences)
};

Q_DECLARE_METATYPE(Int64Map<Sequence*>);

#endif // !TIMELINETRACK_HPP