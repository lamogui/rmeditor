#ifndef TIMELINETRACK_HPP
#define TIMELINETRACK_HPP

#include "xmlsavedobject.hpp"

#include <QObject>

class Renderer;
class TimelineTrack : public QObject
{
  Q_OBJECT
  PROPERTY_CALLBACK_OBJECT
  XML_SAVED_OBJECT

public:
  TimelineTrack();

  


};

#endif // !TIMELINETRACK_HPP