
#include "timelinetrack.hpp"
#include "project.hpp"
#include "sequence.hpp"
#include "timeline.hpp"

#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

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
  jassert(timeline);
  return timeline->getLength();
}

QRectF TimelineTrack::boundingRect() const
{
  return QRectF(pos(), QSizeF(getLength(), getHeight()));
}

void TimelineTrack::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  //nothing to do 
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

void TimelineTrack::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  mousePressPos = event->scenePos();

  QMenu contextMenu;
  QMenu addSequenceMenu;
  addSequenceMenu.setTitle(tr("Add sequence"));
  
  Project* p = Project::get(*this);

  /*QMap<QString, Scene*> scenes = project->rayMarchScenes();
  QMap<QString, Scene*>::const_iterator it = scenes.constBegin();
  for (; it != scenes.constEnd(); it++)
  {
    QAction* action = new QAction(it.key(), &addSequenceMenu);
    addSequenceMenu.addAction(action);
  }

  connect(&addSequenceMenu, SIGNAL(triggered(QAction*)), this, SLOT(addSequenceAction(QAction*)));
  contextMenu.addMenu(&addSequenceMenu);
  contextMenu.exec(event->screenPos()); */
}

void TimelineTrack::insertSequence(Sequence* sequence)
{
  jassert(sequence);
  jassert(sequence->getStartFrame() >= 0); // not terrible but should work
  
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
  jassert(sequence);

  Int64Map<Sequence*>::iterator it = sequences.find(sequence->getStartFrame());
  jassert(it != sequences.end());
  jassert(it.value() == sequence);

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