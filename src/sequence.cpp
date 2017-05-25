#include "sequence.hpp"


#include <QDebug>

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

#include "demotimeline.hpp"
#include "fbo.hpp"
#include "fast2dquad.hpp"
#include "camera.hpp"
#include "keyframe.hpp"
#include "project.hpp"
#include "scene.hpp"
#include "timelinewidget.hpp"

Sequence::Sequence(Project &project, DemoTimeline &timeline, QDomElement &node, qreal height):
  QGraphicsRectItem(NULL),
  node(node),
  project(&project),
  timeline(&timeline),
  scene(NULL)
{
  setRect(QRectF(10,10,10,height)); //ugly
  load();
}

Sequence::Sequence(Project& project, DemoTimeline &timeline, QDomElement node, Scene &scene, int start, int length, qreal height):
  QGraphicsRectItem(NULL),
  node(node),
  project(&project),
  timeline(&timeline),
  scene(&scene)
{
  setRect(QRectF(10,10,10,height)); //ugly
  node.setTagName("sequence");
  node.setAttribute("start",QString::number(start));
  node.setAttribute("length",QString::number(length));
  node.setAttribute("scene",scene.objectName());

  cameraNode = this->project->getDocument().createElement("camera");
  cameraNode = node.appendChild(cameraNode).toElement();

  this->project->notifyDocumentChanged();

  create(start,length);
}

Sequence::~Sequence()
{
  QApplication::restoreOverrideCursor();
  QMap<qint64,CameraKeyframe*>::iterator it;
  for (it = cameraKeyframes.begin(); it != cameraKeyframes.end(); it++)
  {
    delete it.value(); //Maybe useless because of QGraphicsItem parenting
  }
}

void Sequence::load()
{
  QString start_str = node.attribute("start","0");
  QString length_str = node.attribute("length","600");
  QString sceneName = node.attribute("scene","");

  qint64 start = start_str.toInt();
  qint64 length = length_str.toInt();
  scene = project->getRayMarchScene(sceneName);

  create(start,length); //prevent a bug with CameraKeyframe

  cameraNode = node.firstChildElement("camera");
  if (!cameraNode.isNull())
  {
    QDomElement e = cameraNode.firstChildElement("keyframe");
    while (!e.isNull())
    {
      CameraKeyframe* keyframe = new CameraKeyframe(*project,this,e);
      Q_ASSERT(!cameraKeyframes.contains(keyframe->relativeFrame()));
      keyframe->setPos(keyframe->pos().x(), this->rect().height()-5);
      QObject::connect(keyframe,SIGNAL(requestFramePosition(qint64)),timeline,SLOT(requestFramePosition(qint64)));
      cameraKeyframes[keyframe->relativeFrame()] = keyframe;
      e = e.nextSiblingElement("keyframe");
    }
  }
  else
  {
    cameraNode = project->getDocument().createElement("camera");
    cameraNode = node.appendChild(cameraNode).toElement();
  }
  renderImages();

}

void Sequence::create(int start, int length)
{
  setRect(0,0,length,rect().height());
  setPos(start,pos().y());

  orginalLength = length;
  setBrush(idleBrush());
  setFlags(QGraphicsItem::ItemIsSelectable);

  setAcceptHoverEvents(true);

  renderImages();
}


void Sequence::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

  if (this->isSelected())
  {
    setBrush(selectedBrush());
  }
  else
  {
    setBrush(idleBrush());
  }

  QGraphicsRectItem::paint(painter,option,widget);
  qreal scale = getScaleFromWidget(widget);


  QRectF r(QPointF(5*scale,5),QSizeF(preview.size()));
  r.setWidth(r.width()*scale);
  if (r.bottom() > this->rect().bottom() - 5)
  {
    r.setBottom(this->rect().bottom() - 5);
    r.setWidth(r.width() * r.height() / (float) (preview.size().height()));
  }
  if (r.width() > (this->rect().width() - 10*scale))
  {
    r.setWidth((this->rect().width() - 10*scale));
  }
  if (length() > 10)
  {
    painter->drawImage(r,preview);
  }
  QFont textFont;
  textFont.setStretch((int)(100*scale));
  painter->setFont(textFont);
  QRectF textRect = QRectF(r.right() + 5*scale,r.top(),this->rect().right()-(r.right()+5*scale),this->rect().height());
  if (scene)
  {
    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,scene->objectName() + " (" + scene->fileName() + ")");
  }
  else
  {
    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,QString("Invalid scene: ") + node.attribute("scene"));
  }
}

void Sequence::renderImages()
{
  if (scene)
  {
    size_t height = rect().height() - 10;
    FBO fbo(4*height/3,height);
    fbo.enable();

    scene->getShader().enable();

    Camera cam;
    this->setCamera(0,cam);

    scene->getShader().sendf("carotation",cam.getRotation().x(),cam.getRotation().y(),cam.getRotation().z(),cam.getRotation().scalar());
    scene->getShader().sendf("caposition",cam.getPosition().x(),cam.getPosition().y(),cam.getPosition().z());

    scene->getShader().sendf("xy_scale_factor",(float)fbo.getSizeX()/(float)fbo.getSizeY());
    scene->getShader().sendf("sequence_time",0.f);
    scene->getShader().sendf("track_time",(float)this->pos().x()/timeline->getFramerate());

    Fast2DQuadDraw();
    scene->getShader().disable();

    preview = fbo.getImage();

    fbo.disable();

  }
}


void Sequence::setStartFrame(qint64 frame, bool notify)
{
  qint64 previous_frame = startFrame();
  if (frame < 0)
  {
    frame = 0;
  }
  setPos((qreal)frame,pos().y());
  timeline->sequenceStartFrameChanged(previous_frame,this);
  node.setAttribute("start",QString::number(startFrame()));
  if (notify)
  {
    project->notifyDocumentChanged();
  }
  renderImages();
}

void Sequence::setLength(qint64 length, bool notify)
{
  qint64 maxlength = timeline->maxSequenceLengthBeforeOverlap(this);
  if (length > maxlength)
  {
    length = maxlength;
  }
  if (length < 0)
  {
    length = 1;
  }
  QRectF r = rect();
  r.setWidth(length);
  setRect(r);
  node.setAttribute("length",QString::number(length));
  if (notify)
  {
    project->notifyDocumentChanged();
  }
}



void Sequence::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsRectItem::mousePressEvent(event);
  mousePressPos = event->scenePos();
  mousePressStartFrame = startFrame();
  orginalLength = length();
  qreal scale = getScaleFromWidget(event->widget());

  if (isInsideRightExtend(event->pos(),scale))
  {
    currentAction = RightExtend;
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
  QGraphicsRectItem::mouseMoveEvent(event);
  if (currentAction==Move)
  {
    qint64 delta = event->scenePos().x() - mousePressPos.x();
    setStartFrame(mousePressStartFrame + delta,false);

  }
  else if (currentAction==RightExtend)
  {
    qint64 delta = event->scenePos().x() - mousePressPos.x();
    setLength(orginalLength + delta,false);
  }

}

void Sequence::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsRectItem::mouseReleaseEvent(event);
  if (currentAction==Move)
  {
    qint64 delta = event->scenePos().x() - mousePressPos.x();
    setStartFrame(mousePressStartFrame+delta,true);
  }
  else if (currentAction==RightExtend)
  {
    qint64 delta = event->scenePos().x() - mousePressPos.x();
    setLength(orginalLength + delta,true);
  }
  QApplication::restoreOverrideCursor();
  orginalLength = length();
}

qint64 Sequence::nearestFrameAvailableForKeyframe(qint64 rel_frame) const
{
  if (rel_frame < 0)
  {
    rel_frame = 0;
  }
  else if (rel_frame >= length())
  {
    rel_frame = length();
  }

  QMap<qint64,CameraKeyframe*>::const_iterator it = cameraKeyframes.find(rel_frame);
  while (it != cameraKeyframes.constEnd() && it.key() == rel_frame)
  {
    ++rel_frame;
    ++it;
  }
  return rel_frame;
}

void Sequence::keyframePositionChanged(qint64 previous_frame, Keyframe* keyframe)
{
  //Q_ASSERT(cameraKeyframes.contains(previous_frame));
  Q_ASSERT(!cameraKeyframes.contains(keyframe->relativeFrame()));
  cameraKeyframes.remove(previous_frame);
  cameraKeyframes[keyframe->relativeFrame()] = (CameraKeyframe*)keyframe;
}

void Sequence::setCamera(qint64 relative_frame, Camera &cam) const
{
  CameraKeyframe* begin = NULL;
  CameraKeyframe* end = NULL;

  QMap<qint64,CameraKeyframe*>::ConstIterator it;
  for (it = cameraKeyframes.constBegin(); it != cameraKeyframes.constEnd(); ++it)
  {
    if (it.key() <= relative_frame)
    {
      begin = it.value();
    }
    else if (it.key() > relative_frame)
    {
      end = it.value();
      break;
    }
  }

  if (begin && end)
  {
    float delta = (qreal)(relative_frame - begin->relativeFrame())/
                  (qreal)(end->relativeFrame() - begin->relativeFrame());

    float delta_inv = 1.f-delta;
    QVector3D p = end->getPosition() * delta + begin->getPosition() * delta_inv;
    QQuaternion q = QQuaternion::nlerp(begin->getRotation(),end->getRotation(),delta);
    cam.setPosition(p);
    cam.setRotation(q);

  }
  else if (begin)
  {
    cam.fromKeyframe(*begin);
  }
  else if (end)
  {
    cam.fromKeyframe(*end);
  }
  else
  {
    cam.reset();
  }
}

void Sequence::insertCameraKeyframe(qint64 rel_frame, const QVector3D &pos, const QQuaternion &rot)
{
  QMap<qint64,CameraKeyframe*>::iterator it = cameraKeyframes.find(rel_frame);
  CameraKeyframe* keyframe = (it != cameraKeyframes.end()) ? it.value() : NULL;
  if (keyframe)
  {
    keyframe->setPosition(pos);
    keyframe->setRotation(rot);
  }
  else
  {
    QDomElement e = project->getDocument().createElement("keyframe");
    e = cameraNode.appendChild(e).toElement();
    keyframe = new CameraKeyframe(rel_frame,*project,this,e,pos,rot);
    QObject::connect(keyframe,SIGNAL(requestFramePosition(qint64)),timeline,SLOT(requestFramePosition(qint64)));
    keyframe->setPos(keyframe->pos().x(), this->rect().height()-5);
    cameraKeyframes[keyframe->relativeFrame()] = keyframe;
  }

  if (rel_frame == 0)
  {
    renderImages();
  }
}

void Sequence::deleteCameraKeyframe(CameraKeyframe *key)
{
  QMap<qint64,CameraKeyframe*>::iterator it = cameraKeyframes.find(key->relativeFrame());
  cameraNode.removeChild(it.value()->getNode()).clear();
  delete it.value();
  cameraKeyframes.erase(it);
  project->notifyDocumentChanged();
}


void Sequence::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  QGraphicsRectItem::hoverEnterEvent(event);
  qreal scale = getScaleFromWidget(event->widget());
  if (isInsideRightExtend(event->pos(),scale))
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
  QGraphicsRectItem::hoverMoveEvent(event);
  qreal scale = getScaleFromWidget(event->widget());
  if (isInsideRightExtend(event->pos(),scale))
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
  QGraphicsRectItem::hoverLeaveEvent(event);
  QApplication::restoreOverrideCursor();
}

bool Sequence::isInsideRightExtend(QPointF rel_pos, qreal scale) const
{
  return (rel_pos.x()>=length()-3.0*scale); //Ugly
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
