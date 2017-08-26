#include "sequence.hpp"

#include <QDebug>

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

#include "render.hpp" // for rendering preview
#include "mediafile.hpp"
#include "renderfunctionscache.hpp"
#include "timelinewidget.hpp" // fucking scale
#include "undocommands.hpp"

Sequence::Sequence(QGraphicsObject* parent) :
  QGraphicsObject(parent),
  startFrame(-1), // invalid position
  length(2), // minimal length is 1 frame else the sequence doesn't "exists", but actual impl only support at least 2 frames
  camera(new Camera), 
  height(60),
  renderCache(nullptr)
{
  CONNECT_XML_SAVED_OBJECT(Sequence);

  setFlags(QGraphicsItem::ItemIsSelectable);
  setAcceptHoverEvents(true);
}

void Sequence::setMedia(const QPointer<MediaFile>& mediaFile)
{
  if (media != mediaFile)
  {
    QPointer<MediaFile> oldMedia;
    media = mediaFile;
    if (!media.isNull())
    {
      Q_ASSERT(media->canBeRendered());
      renderer.reset(media->createRenderer());
      if (renderer->hasDynamicCamera() && cameraKeyframes.size())
      {
        renderer->setCurrentCamera(camera);
      }
      if (renderCache)
      {
        renderer->initializeGL(*renderCache);
        renderImages();
      }
    }
    emit propertyChanged(this, "media", QVariant::fromValue(oldMedia), QVariant::fromValue(media));
  }
}

void Sequence::insertCameraKeyframe(CameraKeyframe* keyframe)
{
  Q_ASSERT(keyframe);
  Q_ASSERT(keyframe->getRelativeFrame() >= 0 && keyframe->getRelativeFrame() < (qint64)getLength()); // not terribble but should work
  Q_ASSERT(cameraKeyframes.constFind(keyframe->getRelativeFrame()) != cameraKeyframes.constEnd()); // Error erase previous keyframe

  keyframe->setParentItem(this);

  connect(keyframe, &Keyframe::propertyChanged, this, &Sequence::keyframePropertyChanged);
  connect(keyframe, &Keyframe::requestFramePosition, this, &Sequence::keyframeRequestFramePosition);

  cameraKeyframes.insert(keyframe->getRelativeFrame(), keyframe);
  
  QVariant oldValue;
  QVariant newValue = QVariant::fromValue(keyframe);
  emit propertyChanged(this, "cameraKeyframes", oldValue, newValue);
}

void Sequence::removeCameraKeyframe(CameraKeyframe* keyframe)
{
  Int64Map<CameraKeyframe*>::iterator it = cameraKeyframes.find(keyframe->getRelativeFrame());
  Q_ASSERT(it != cameraKeyframes.end());
  Q_ASSERT(it.value() == keyframe);

  cameraKeyframes.erase(it);

  disconnect(keyframe, &Keyframe::propertyChanged, this, &Sequence::keyframePropertyChanged);
  disconnect(keyframe, &Keyframe::requestFramePosition, this, &Sequence::keyframeRequestFramePosition);

  setParentItem(nullptr);

  QVariant oldValue = QVariant::fromValue(keyframe);
  QVariant newValue; 
  emit propertyChanged(this, "cameraKeyframes", oldValue, newValue);
}

void Sequence::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{  
  QBrush fillBrush(isSelected() ? selectedBrush() : idleBrush());
  QRectF rect = boundingRect();
  painter->fillRect(rect, fillBrush); 
  painter->drawRect(rect);

  qreal scale = getScaleFromWidget(widget);
  QRectF previewRect(QPointF(5*scale,5),QSizeF(preview.size()));
  previewRect.setWidth(previewRect.width()*scale);
  if (previewRect.bottom() > rect.bottom() - 5)
  {
    previewRect.setBottom(rect.bottom() - 5);
    previewRect.setWidth(rect.width() * previewRect.height() / (float) (preview.size().height()));
  }
  if (previewRect.width() > (rect.width() - 10*scale))
  {
    previewRect.setWidth((rect.width() - 10*scale));
  }
  if (getLength() > 10)
  {
    painter->drawImage(previewRect,preview);
  }
  QFont textFont;
  textFont.setStretch((int)(100*scale));
  painter->setFont(textFont);
  QRectF textRect = QRectF(previewRect.right() + 5*scale,previewRect.top(), previewRect.right()-(previewRect.right()+5*scale), rect.height());
  if (!media.isNull())
  {
    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,media.data()->objectName() + " (" + media->getPath().fileName() + ")");
  }
  else
  {
    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,QString("Missing media"));
  }
}

QRectF Sequence::boundingRect() const
{
  return QRectF(0, 0, getLength(), getHeight());
}

void Sequence::initializeGL(RenderFunctionsCache& renderCache)
{
  this->renderCache = &renderCache;
  renderImages();
  if (!renderer.isNull())
    renderer->initializeGL(renderCache);
}

void Sequence::renderImages()
{
  Q_ASSERT(renderCache);
  if (!renderer.isNull())
  {
    int height = getHeight() - 10;
    RenderTexture2D render;
    render.initializeGL(*renderCache, QSize(4 * height / 3, height));
    preview = render.getImage();
    render.render(*renderCache, *renderer);
  }
}

void Sequence::setStartFrame(qint64 frame)
{
  qint64 previous_frame = getStartFrame();
  if (frame < 0)
  {
    frame = 0;
  }
  if (frame != previous_frame)
  {
    startFrame = frame;
    emit propertyChanged(this, "startFrame", previous_frame, startFrame);
  }
}

void Sequence::setLength(quint64 length)
{
  /*qint64 maxlength = timeline->maxSequenceLengthBeforeOverlap(this);
  if (length > maxlength)
  {
    length = maxlength;
  }*/
  if (length < 2)
  {
    length = 2;
  }
  
  quint64 oldLength = getLength();
  if (oldLength != length)
  {
    this->length = length;
    update(QRect(startFrame, 0, length, height));
    emit propertyChanged(this, "length", oldLength, length);
  }
}

void Sequence::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  BaseClass::mousePressEvent(event);
  mousePressPos = event->scenePos();
  mousePressStartFrame = getStartFrame();
  mousePressLength = getLength();
  qreal scale = getScaleFromWidget(event->widget());

  if (isInsideRightExtend(event->pos(),scale))
  {
    currentAction = RightExtend;
    QApplication::setOverrideCursor(Qt::SizeHorCursor);

  }
  else if (isInsideLeftExtend(event->pos(), scale))
  {
    currentAction = LeftExtend;
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
  }
  else
  {
    currentAction = Move;
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
  }
}

void Sequence::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  BaseClass::mouseMoveEvent(event);
  qint64 delta = event->scenePos().x() - mousePressPos.x();
  if (currentAction==Move)
  {
    setStartFrame(mousePressStartFrame + delta);
  }
  else if (currentAction==RightExtend)
  {
    qint64 v = mousePressLength + delta;
    setLength(v < 0 ? 0 : v);
  }
  else if (currentAction==LeftExtend)
  {
    qint64 v = mousePressStartFrame - delta;
    setStartFrame(mousePressStartFrame + delta);
    setLength(v < 0 ? 0: v);
  }
}

void Sequence::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  BaseClass::mouseReleaseEvent(event);
  qint64 delta = event->scenePos().x() - mousePressPos.x();
  if (currentAction==Move)
  {
    ASSERT_IF_UNIQUE_RECEIVER(sendUndoCommand(QUndoCommand*))
    {
      emit sendUndoCommand(new ModifyPropertyCommand(*this, "startFrame", mousePressStartFrame, mousePressStartFrame + delta));
    }
  }
  else if (currentAction==RightExtend)
  {
    ASSERT_IF_UNIQUE_RECEIVER(sendUndoCommand(QUndoCommand*))
    {
      qint64 v = mousePressLength + delta;
      emit sendUndoCommand(new ModifyPropertyCommand(*this, "length", mousePressLength, v < 0 ? 0 : v));
    }
  }
  else if (currentAction == LeftExtend)
  {
    ASSERT_IF_UNIQUE_RECEIVER(sendUndoCommand(QUndoCommand*))
    {
      qint64 v = mousePressStartFrame - delta;
      QUndoCommand* cmd = new QUndoCommand("Extend sequence");
      QUndoCommand* startFrameCmd = new ModifyPropertyCommand(*this, "startFrame", mousePressStartFrame, mousePressStartFrame + delta, cmd);
      QUndoCommand* lengthCmd = new ModifyPropertyCommand(*this, "length", mousePressLength, v < 0 ? 0 : v, cmd);
      emit sendUndoCommand(cmd);
    }
  }
  QApplication::restoreOverrideCursor();
}

/*
qint64 Sequence::nearestFrameAvailableForKeyframe(qint64 rel_frame) const
{
  if (rel_frame < 0)
  {
    rel_frame = 0;
  }
  else if (rel_frame >= getLength())
  {
    rel_frame = getLength();
  }

  QMap<qint64,CameraKeyframe*>::const_iterator it = cameraKeyframes.find(rel_frame);
  while (it != cameraKeyframes.constEnd() && it.key() == rel_frame)
  {
    ++rel_frame;
    ++it;
  }
  return rel_frame;
}
*/
void Sequence::setFramePosition(qint64 framePosition)
{
  CameraKeyframe* begin = nullptr;
  CameraKeyframe* end = nullptr;

  QMap<qint64,CameraKeyframe*>::ConstIterator it;
  for (it = cameraKeyframes.constBegin(); it != cameraKeyframes.constEnd(); ++it)
  {
    if (it.key() <= framePosition)
    {
      begin = it.value();
    }
    else if (it.key() > framePosition)
    {
      end = it.value();
      break;
    }
  }

  if (begin && end)
  {
    float delta = (qreal)(framePosition - begin->getRelativeFrame())/
                  (qreal)(end->getRelativeFrame() - begin->getRelativeFrame());

    float delta_inv = 1.f-delta;
    QVector3D p = end->getPosition() * delta + begin->getPosition() * delta_inv;
    QQuaternion q = QQuaternion::nlerp(begin->getRotation(),end->getRotation(),delta);
    camera->setPosition(p);
    camera->setRotation(q);

  }
  else if (begin)
  {
    camera->fromKeyframe(*begin);
  }
  else if (end)
  {
    camera->fromKeyframe(*end);
  }
  else
  {
    camera->reset();
  }
}

void Sequence::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  BaseClass::hoverEnterEvent(event);
  qreal scale = getScaleFromWidget(event->widget());
  if (isInsideRightExtend(event->pos(),scale) || isInsideLeftExtend(event->pos(), scale))
  {
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
  }
  else
  {
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
  }
}

void Sequence::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
  BaseClass::hoverMoveEvent(event);
  qreal scale = getScaleFromWidget(event->widget());
  if (isInsideRightExtend(event->pos(),scale) || isInsideLeftExtend(event->pos(), scale))
  {
    QApplication::changeOverrideCursor(Qt::SizeHorCursor);
  }
  else
  {
    QApplication::changeOverrideCursor(Qt::OpenHandCursor);
  }
}

void Sequence::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  BaseClass::hoverLeaveEvent(event);
  QApplication::restoreOverrideCursor();
}

bool Sequence::isInsideRightExtend(QPointF rel_pos, qreal scale) const
{
  return (rel_pos.x()>=getLength()-3.0*scale); //Ugly
}

bool Sequence::isInsideLeftExtend(QPointF rel_pos, qreal scale) const
{
	return (rel_pos.x()<=3.0*scale); //Ugly
}

qreal Sequence::getScaleFromWidget(const QWidget *widget) const
{
  const TimelineWidget* w = qobject_cast<const TimelineWidget*>(widget);
  if (!w)
  {
    w = qobject_cast<const TimelineWidget*>(widget->parent());
  }
  return w ? 1.0 / w->getScale().x() : 1.0;
}

void Sequence::keyframePropertyChanged(QObject* owner, const QString& propertyName, const QVariant& oldValue, const QVariant& newValue)
{
  if (propertyName == "relativeFrame")
  {
    CameraKeyframe* keyframe = qobject_cast<CameraKeyframe*>(owner);
    qint64 oldFrame = oldValue.toInt();
    qint64 newFrame = newValue.toInt();
    if (cameraKeyframes.contains(oldFrame) && cameraKeyframes[oldFrame] == keyframe)
      cameraKeyframes.remove(oldFrame); 

    if (cameraKeyframes.contains(newFrame)) // fuck !
    {
      // search around the keyframe a free space avaiable
      qint64 newFrameLeft = newFrame;
      while (cameraKeyframes.contains(newFrameLeft))
        newFrameLeft--;

      qint64 newFrameRight;
      while (cameraKeyframes.contains(newFrameRight))
        newFrameRight--;

      if (newFrameLeft >= 0 && newFrameRight <= (qint64)getLength())
      {
        if (newFrame - newFrameLeft < newFrameRight - newFrame)
          newFrame = newFrameLeft;
        else
          newFrame = newFrameRight;
      }
      else if (newFrameLeft >= 0)
        newFrame = newFrameLeft;
      else if (newFrameRight <= (qint64)getLength())
        newFrame = newFrameRight;
      else
        Q_ASSERT(false);

      keyframe->setRelativeFrame(newFrame);
    }
    else
      cameraKeyframes.insert(newFrame, keyframe); // avoid re-entrancy 
  }
}

void Sequence::keyframeRequestFramePosition(qint64 position)
{
  emit requestFramePosition(this, position);
}