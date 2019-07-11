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
  QGraphicsRectItem(nullptr),
  m_node(node),
  m_project(&project),
  m_timeline(&timeline),
  m_scene(nullptr)
{
  setRect(QRectF(10,10,10,height)); //ugly
  load();
}

Sequence::Sequence(Project& project, DemoTimeline &timeline, QDomElement node, Scene &scene, int start, int length, qreal height):
  QGraphicsRectItem(nullptr),
  m_node(node),
  m_project(&project),
  m_timeline(&timeline),
  m_scene(&scene)
{
  setRect(QRectF(10,10,10,height)); //ugly
  m_node.setTagName("sequence");
  m_node.setAttribute("start",QString::number(start));
  m_node.setAttribute("length",QString::number(length));
  m_node.setAttribute("scene",scene.objectName());

  m_cameraNode = m_project->document().createElement("camera");
  m_cameraNode = m_node.appendChild(m_cameraNode).toElement();

  m_project->notifyDocumentChanged();

  create(start,length);
}

Sequence::~Sequence()
{
  QApplication::restoreOverrideCursor();
  QMap<qint64,CameraKeyframe*>::iterator it;
  for (it = m_cameraKeyframes.begin(); it != m_cameraKeyframes.end(); it++)
  {
    delete it.value(); //Maybe useless because of QGraphicsItem parenting
  }
}

void Sequence::load()
{
  QString start_str = m_node.attribute("start","0");
  QString length_str = m_node.attribute("length","600");
  QString sceneName = m_node.attribute("scene","");

  qint64 start = start_str.toInt();
  qint64 length = length_str.toInt();
  m_scene = m_project->getRayMarchScene(sceneName);

  create(start,length); //prevent a bug with CameraKeyframe

  m_cameraNode = m_node.firstChildElement("camera");
  if (!m_cameraNode.isNull())
  {
    QDomElement e = m_cameraNode.firstChildElement("keyframe");
    while (!e.isNull())
    {
      CameraKeyframe* keyframe = new CameraKeyframe(*m_project,this,e);
      Q_ASSERT(!m_cameraKeyframes.contains(keyframe->relativeFrame()));
      keyframe->setPos(keyframe->pos().x(), this->rect().height()-5);
      QObject::connect(keyframe,SIGNAL(requestFramePosition(qint64)),m_timeline,SLOT(requestFramePosition(qint64)));
      m_cameraKeyframes[keyframe->relativeFrame()] = keyframe;
      e = e.nextSiblingElement("keyframe");
    }
  }
  else
  {
    m_cameraNode = m_project->document().createElement("camera");
    m_cameraNode = m_node.appendChild(m_cameraNode).toElement();
  }
  renderImages();

}

void Sequence::create(int start, int length)
{
  setRect(0,0,length,rect().height());
  setPos(start,pos().y());

  m_orginalLength = length;
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


  QRectF r(QPointF(5*scale,5),QSizeF(m_preview.size()));
  r.setWidth(r.width()*scale);
  if (r.bottom() > this->rect().bottom() - 5)
  {
    r.setBottom(this->rect().bottom() - 5);
    r.setWidth(r.width() * r.height() / (float) (m_preview.size().height()));
  }
  if (r.width() > (this->rect().width() - 10*scale))
  {
    r.setWidth((this->rect().width() - 10*scale));
  }
  if (length() > 10)
  {
    painter->drawImage(r,m_preview);
  }
  QFont textFont;
  textFont.setStretch((int)(100*scale));
  painter->setFont(textFont);
  QRectF textRect = QRectF(r.right() + 5*scale,r.top(),this->rect().right()-(r.right()+5*scale),this->rect().height());
  if (m_scene)
  {
    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,m_scene->objectName() + " (" + m_scene->fileName() + ")");
  }
  else
  {
    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignTop,QString("Invalid scene: ") + m_node.attribute("scene"));
  }
}

void Sequence::renderImages()
{
  if (m_scene)
  {
    size_t height = rect().height() - 10;
    FBO fbo(4*height/3,height);
    fbo.enable();

    m_scene->getShader().enable();

    Camera cam;
    this->setCamera(0,cam);

    m_scene->getShader().sendf("cam_rotation",cam.rotation().x(),cam.rotation().y(),cam.rotation().z(),cam.rotation().scalar());
    m_scene->getShader().sendf("cam_position",cam.position().x(),cam.position().y(),cam.position().z());

    m_scene->getShader().sendf("xy_scale_factor",(float)fbo.getSizeX()/(float)fbo.getSizeY());
    m_scene->getShader().sendf("sequence_time",0.f);
    m_scene->getShader().sendf("track_time",(float)this->pos().x()/m_timeline->framerate());

    Fast2DQuadDraw();
    m_scene->getShader().disable();

    m_preview = fbo.getImage();

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
  m_timeline->sequenceStartFrameChanged(previous_frame,this);
  m_node.setAttribute("start",QString::number(startFrame()));
  if (notify)
  {
    m_project->notifyDocumentChanged();
  }
  renderImages();
}

void Sequence::setLength(qint64 length, bool notify)
{
  qint64 maxlength = m_timeline->maxSequenceLengthBeforeOverlap(this);
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
  m_node.setAttribute("length",QString::number(length));
  if (notify)
  {
    m_project->notifyDocumentChanged();
  }
}



void Sequence::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsRectItem::mousePressEvent(event);
  m_mousePressPos = event->scenePos();
  m_mousePressStartFrame = startFrame();
  m_orginalLength = length();
  qreal scale = getScaleFromWidget(event->widget());

  if (isInsideRightExtend(event->pos(),scale))
  {
    m_currentAction = RightExtend;
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
  QGraphicsRectItem::mouseMoveEvent(event);
  if (m_currentAction==Move)
  {
    qint64 delta = event->scenePos().x() - m_mousePressPos.x();
    setStartFrame(m_mousePressStartFrame + delta,false);

  }
  else if (m_currentAction==RightExtend)
  {
    qint64 delta = event->scenePos().x() - m_mousePressPos.x();
    setLength(m_orginalLength + delta,false);
  }

}

void Sequence::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsRectItem::mouseReleaseEvent(event);
  if (m_currentAction==Move)
  {
    qint64 delta = event->scenePos().x() - m_mousePressPos.x();
    setStartFrame(m_mousePressStartFrame+delta,true);
  }
  else if (m_currentAction==RightExtend)
  {
    qint64 delta = event->scenePos().x() - m_mousePressPos.x();
    setLength(m_orginalLength + delta,true);
  }
  QApplication::restoreOverrideCursor();
  m_orginalLength = length();
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

  QMap<qint64,CameraKeyframe*>::const_iterator it = m_cameraKeyframes.find(rel_frame);
  while (it != m_cameraKeyframes.constEnd() && it.key() == rel_frame)
  {
    ++rel_frame;
    ++it;
  }
  return rel_frame;
}

void Sequence::keyframePositionChanged(qint64 previous_frame, Keyframe* keyframe)
{
  //Q_ASSERT(m_cameraKeyframes.contains(previous_frame));
  Q_ASSERT(!m_cameraKeyframes.contains(keyframe->relativeFrame()));
  m_cameraKeyframes.remove(previous_frame);
  m_cameraKeyframes[keyframe->relativeFrame()] = (CameraKeyframe*)keyframe;
}

void Sequence::setCamera(qint64 relative_frame, Camera &cam) const
{
  CameraKeyframe* begin = nullptr;
  CameraKeyframe* end = nullptr;

  QMap<qint64,CameraKeyframe*>::ConstIterator it;
  for (it = m_cameraKeyframes.constBegin(); it != m_cameraKeyframes.constEnd(); ++it)
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
    QVector3D p = end->position() * delta + begin->position() * delta_inv;
    QQuaternion q = QQuaternion::nlerp(begin->rotation(),end->rotation(),delta);
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
  QMap<qint64,CameraKeyframe*>::iterator it = m_cameraKeyframes.find(rel_frame);
  CameraKeyframe* keyframe = (it != m_cameraKeyframes.end()) ? it.value() : nullptr;
  if (keyframe)
  {
    keyframe->setPosition(pos);
    keyframe->setRotation(rot);
  }
  else
  {
    QDomElement e = m_project->document().createElement("keyframe");
    e = m_cameraNode.appendChild(e).toElement();
    keyframe = new CameraKeyframe(rel_frame,*m_project,this,e,pos,rot);
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
  return w ? 1.0 / w->scale().x() : 1.0;
}
