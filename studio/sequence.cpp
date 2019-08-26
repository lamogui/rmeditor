#include "sequence.hpp"

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

#include "render.hpp" // for rendering preview
#include "mediafile.hpp"
#include "renderfunctionscache.hpp"
#include "demotimelinewidget.hpp" // fucking scale
#include "undocommands.hpp"

Sequence::Sequence(QGraphicsObject* _parent) :
	QGraphicsObject(_parent),
	m_startFrame(-1), // invalid position
	m_length(2), // minimal length is 1 frame else the sequence doesn't "exists", but actual impl only support at least 2 frames
	m_height(60),
	m_renderCache(nullptr)
{
	setFlags(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(true);
}

void Sequence::paint(QPainter *painter, const QStyleOptionGraphicsItem* /*_option*/, QWidget* _widget)
{  
	QBrush fillBrush(isSelected() ? selectedBrush() : idleBrush());
	QRectF rect = boundingRect();
	painter->fillRect(rect, fillBrush);
	painter->drawRect(rect);

	qreal scale = getScaleFromWidget(_widget);
	QRectF previewRect(QPointF(5*scale,5),QSizeF(m_preview.size()));
	previewRect.setWidth(previewRect.width()*scale);
	if (previewRect.bottom() > rect.bottom() - 5)
	{
		previewRect.setBottom(rect.bottom() - 5);
		previewRect.setWidth(rect.width() * previewRect.height() / static_cast<qreal>(m_preview.size().height()));
	}
	if (previewRect.width() > (rect.width() - 10*scale))
	{
		previewRect.setWidth((rect.width() - 10*scale));
	}
	if (m_length > 10)
	{
		painter->drawImage(previewRect, m_preview);
	}
	QFont textFont;
	textFont.setStretch(static_cast<int>(100*scale));
	painter->setFont(textFont);
	QRectF textRect = QRectF(previewRect.right() + 5*scale,previewRect.top(), previewRect.right()-(previewRect.right()+5*scale), rect.height());
	if (!m_media.isNull())
	{
		painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,m_media.data()->objectName() + " (" + m_media->getPath().fileName() + ")");
	}
	else
	{
		painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,QString("Missing media"));
	}
}

QRectF Sequence::boundingRect() const
{
	return QRectF(0, 0, m_length, m_height);
}

void Sequence::initializeGL(RenderFunctionsCache& _renderCache)
{
	this->m_renderCache = &_renderCache;
	renderImages();
	if ( m_renderer ) {
		m_renderer ->initializeGL(_renderCache);
	}
}

void Sequence::renderImages()
{
	jassert(m_renderCache);
	if ( m_renderer )
  {
		int height = static_cast<int>(m_height - 10);
    RenderTexture2D render;
		render.initializeGL(*m_renderCache, QSize(4 * height / 3, height));
		m_preview = render.getImage();
		render.render(*m_renderCache, *m_renderer);
  }
}

void Sequence::setStartFrame(qint64 _frame)
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
    m_currentAction = RightExtend;
    QApplication::setOverrideCursor(Qt::SizeHorCursor);

  }
  else if (isInsideLeftExtend(event->pos(), scale))
  {
    currentAction = LeftExtend;
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
  }
  else
  {
    m_currentAction = Move;
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
  }
}

void Sequence::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  BaseClass::mouseMoveEvent(event);
  qint64 delta = event->scenePos().x() - mousePressPos.x();
  if (m_currentAction==Move)
  {
    setStartFrame(mousePressStartFrame + delta);
  }
  else if (m_currentAction==RightExtend)
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
  if (m_currentAction==Move)
  {
    UndoStack::assertSendUndoCommand(*this, new ModifyPropertyCommand(*this, "startFrame", mousePressStartFrame, mousePressStartFrame + delta));
  }
  else if (m_currentAction==RightExtend)
  {
    qint64 v = mousePressStartFrame - delta;
    UndoStack::assertSendUndoCommand(*this, new ModifyPropertyCommand(*this, "length", mousePressLength, v < 0 ? 0 : v));
  }
  else if (currentAction == LeftExtend)
  {
    {
      qint64 v = mousePressStartFrame - delta;
      QUndoCommand* cmd = new QUndoCommand("Extend sequence");
      QUndoCommand* startFrameCmd = new ModifyPropertyCommand(*this, "startFrame", mousePressStartFrame, mousePressStartFrame + delta, cmd);
      QUndoCommand* lengthCmd = new ModifyPropertyCommand(*this, "length", mousePressLength, v < 0 ? 0 : v, cmd);
      UndoStack::assertSendUndoCommand(*this, cmd);
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

  QMap<qint64,CameraKeyframe*>::const_iterator it = m_cameraKeyframes.find(rel_frame);
  while (it != m_cameraKeyframes.constEnd() && it.key() == rel_frame)
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
  for (it = m_cameraKeyframes.constBegin(); it != m_cameraKeyframes.constEnd(); ++it)
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
    QVector3D p = end->position() * delta + begin->position() * delta_inv;
    QQuaternion q = QQuaternion::nlerp(begin->rotation(),end->rotation(),delta);
    camera->setPosition(p);
    camera->setRotation(q);
    cam.setFov(begin->fov() * delta_inv + end->fov() * delta);

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

void Sequence::insertCameraKeyframe(qint64 rel_frame, const QVector3D &pos, const QQuaternion &rot, float fov)
{
  QMap<qint64,CameraKeyframe*>::iterator it = m_cameraKeyframes.find(rel_frame);
  CameraKeyframe* keyframe = (it != m_cameraKeyframes.end()) ? it.value() : nullptr;
  if (keyframe)
  {
    keyframe->setPosition(pos);
    keyframe->setRotation(rot);
    keyframe->setFov(fov);
  }
  else
  {
    QDomElement e = m_project->document().createElement("keyframe");
    e = m_cameraNode.appendChild(e).toElement();
    keyframe = new CameraKeyframe(rel_frame,*m_project,this,e,pos,rot, fov);
    QObject::connect(keyframe,SIGNAL(requestFramePosition(qint64)),m_timeline,SLOT(requestFramePosition(qint64)));
    keyframe->setPos(keyframe->pos().x(), this->rect().height()-5);
    m_cameraKeyframes[keyframe->relativeFrame()] = keyframe;
  }

  if (rel_frame == 0)
  {
    renderImages();
  }
}

void Sequence::deleteCameraKeyframe(CameraKeyframe *key)
{
  QMap<qint64,CameraKeyframe*>::iterator it = m_cameraKeyframes.find(key->relativeFrame());
  m_cameraNode.removeChild(it.value()->node()).clear();
  delete it.value();
  m_cameraKeyframes.erase(it);
  m_project->notifyDocumentChanged();
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
  return w ? 1.0 / w->scale().x() : 1.0;
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
        jassertfalse;

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
