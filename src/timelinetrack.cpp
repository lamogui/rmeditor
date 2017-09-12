#include "timelinetrack.hpp"
#include "sequence.hpp"
#include "timeline.hpp"

TimelineTrack::TimelineTrack(QGraphicsObject* parent) :
  QGraphicsObject(parent),
  height(60),
  renderCache(nullptr)
{
  CONNECT_XML_SAVED_OBJECT(TimelineTrack);
}

void TimelineTrack::initializeGL(RenderFunctionsCache& cache)
{
  renderCache = &cache;
  Int64Map<Sequence*>::iterator it;
  for (it = sequences.begin(); it != sequences.end(); ++it)
  {
    it.value()->initializeGL(cache);
  }
}

Sequence* TimelineTrack::isInsideSequence(qint64 frame) const
{
  Int64Map<Sequence*>::const_iterator it;
  for (it = sequences.constBegin(); it != sequences.constEnd(); it++) //TODO use dichotomie
  {
    if (it.value()->isInside(frame))
    {
      return it.value();
    }
  }
  return nullptr;
}

quint64 TimelineTrack::getLength() const
{
  Timeline* timeline = dynamic_cast<Timeline*>(scene());
  assert(timeline);
  return timeline->getLength();
}

QRectF TimelineTrack::boundingRect() const
{
  return QRectF(pos(), QSizeF(getHeight(), getLength()));
}

bool TimelineTrack::makeSequenceFit(Sequence& sequence)
{
  bool shouldModifyProperties = false;
  Sequence* overlapedSequence = isInsideSequence(sequence.getStartFrame());
  while (overlapedSequence) // push the sequence to the end of overlapped sequence
  {
    sequence.setStartFrame(overlapedSequence->getEndFrame());
    shouldModifyProperties = true;
    overlapedSequence = isInsideSequence(sequence.getStartFrame());
  }

  overlapedSequence = isInsideSequence(sequence.getEndFrame() - 1);
  while (overlapedSequence) // reduce the size to not overlap existing sequence
  {
    sequence.setLength(overlapedSequence->getStartFrame() - overlapedSequence->getStartFrame());
    shouldModifyProperties = true;
    overlapedSequence = isInsideSequence(sequence.getEndFrame() - 1);
  }
  return shouldModifyProperties;
}

void TimelineTrack::insertSequence(Sequence* sequence)
{
  assert(sequence);
  assert(sequence->getStartFrame() >= 0); // not terrible but should work
  
  makeSequenceFit(*sequence); // be sure not erasing anything

  sequence->setParentItem(this);
  if (renderCache)
    sequence->initializeGL(*renderCache);

  connect(sequence, &Sequence::propertyChanged, this, &TimelineTrack::sequencePropertyChanged);
  connect(sequence, &Sequence::requestFramePosition, this, &TimelineTrack::sequenceRequestFramePosition);

  sequences.insert(sequence->getStartFrame(), sequence);

  QVariant oldValue;
  QVariant newValue = QVariant::fromValue(sequence);
  emit propertyChanged(this, "sequences", oldValue, newValue);
}

void TimelineTrack::removeSequence(Sequence* sequence)
{
  assert(sequence);

  Int64Map<Sequence*>::iterator it = sequences.find(sequence->getStartFrame());
  assert(it != sequences.end());
  assert(it.value() == sequence);

  sequences.erase(it);

  disconnect(sequence, &Sequence::propertyChanged, this, &TimelineTrack::sequencePropertyChanged);
  disconnect(sequence, &Sequence::requestFramePosition, this, &TimelineTrack::sequenceRequestFramePosition);

  sequence->setParentItem(nullptr);

  QVariant oldValue = QVariant::fromValue(sequence);
  QVariant newValue;
  emit propertyChanged(this, "sequences", oldValue, newValue);
}

void TimelineTrack::sequencePropertyChanged(QObject* owner, const QString& propertyName, const QVariant& oldValue, const QVariant& newValue)
{
  Sequence* sequence = qobject_cast<Sequence*>(owner);
  if (propertyName == "startFrame")
  {
    qint64 oldStartFrame = oldValue.toInt();
    if (sequences.contains(oldStartFrame) && sequences[oldStartFrame] == sequence)
      sequences.remove(oldStartFrame); // be sure we won't overlap self...

    if (!makeSequenceFit(*sequence))
      sequences.insert(sequence->getStartFrame(), sequence); // avoid re-entrancy 
  }
  else if (propertyName == "length")
  {
    makeSequenceFit(*sequence);
  }
}

void TimelineTrack::sequenceRequestFramePosition(const Sequence* source, qint64 position)
{
  emit requestFramePosition(position + source->getStartFrame());
}