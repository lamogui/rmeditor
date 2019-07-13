#include "demotimeline.hpp"
#include "fast2dquad.hpp"
#include "music.hpp"
#include "sequence.hpp"
#include "scene.hpp"
#include "shaderminifier.hpp"
#include "project.hpp"


#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QTextStream>
#include <QTimer>
#include <QtMath>


DemoTimeline::DemoTimeline(QDomElement &node, Project &project, double fps, LogWidget &log):
  Timeline(*(project.music()),2*60.0,fps,log),
  m_node(node),
  m_trackHeight(60.0),
  m_project(project),
  m_renderer(new DemoRenderer(project, *this,1280,720))
{
  m_renderer->setCamera(&m_camera);
  load();
}

DemoTimeline::~DemoTimeline()
{
  QMap<qint64, Sequence*>::iterator it;
  for (it = m_sequences.begin(); it != m_sequences.end(); it++) //TODO use dichotomie
  {
    delete it.value(); //Maybe useless because of QGraphicsScene parenting
  }
}

void DemoTimeline::load()
{
  QDomElement element = m_node.firstChildElement();
  while (!element.isNull())
  {
    if (element.tagName() == "track")
    {
      if (!parseTrackNode(element))
      {
       // buildSuccess = false;
      }
    }
    else
    {
      emit warning(QString("[") + m_project.fileName() + "]" + " <timeline> warning at line " + QString::number(element.lineNumber()) +
                   + " unknown tag name '" + element.tagName() + "' ignoring the block");
    }
    element = element.nextSiblingElement();
  }
}


bool DemoTimeline::parseTrackNode(QDomElement& node)
{
  QDomElement element = node.firstChildElement();
  while (!element.isNull())
  {
    if (element.tagName() == "sequence")
    {
      Sequence* seq = new Sequence(m_project,*this,element,m_trackHeight);
      addSequence(seq);

    }
    else
    {
        emit warning(QString(QString("[") + m_project.fileName() + "] <timeline> warning at line " + QString::number(element.lineNumber()) +
                     + " unknown tag name '" + element.tagName() + "' ignoring the block"));
    }
    element = element.nextSiblingElement();
  }
  return true;
}

qint64 DemoTimeline::addSequence(Sequence *seq)
{
  correctStartFrame(seq);
  seq->setPos(seq->pos().x(),0);
  this->addItem(seq);
  return seq->startFrame();
}

void DemoTimeline::updateCamera(qint64 frame, Camera &cam)
{
  Sequence* seq = isInsideSequence(frame);
  if (seq)
  {
    seq->setCamera(frame-seq->startFrame(),cam);
  }
  else
  {
    cam.reset();
  }
}

Sequence* DemoTimeline::isInsideSequence(qint64 frame) const
{
  QMap<qint64, Sequence*>::const_iterator it;
  for (it = m_sequences.constBegin(); it != m_sequences.constEnd(); it++) //TODO use dichotomie
  {
    if (it.value()->isInside(frame))
    {
      return it.value();
    }
  }
  return nullptr;
}
unsigned int DemoTimeline::sequenceID(qint64 frame) const
{
  QMap<qint64, Sequence*>::const_iterator it;
  unsigned int id =0;
  for (it = m_sequences.constBegin(); it != m_sequences.constEnd(); it++) //TODO use dichotomie
  {
    if (it.value()->isInside(frame))
    {
      break;
    }
    ++id;
  }
  return id;
}



bool DemoTimeline::correctStartFrame(Sequence* seq)
{
  QMap<qint64, Sequence*>::iterator it;
  bool corrected = false;
  seq->setLength(seq->orginalLength());
  for (it = m_sequences.begin(); it != m_sequences.end(); it++) //TODO use dichotomie
  {
    if (it.value()->isInside(seq->startFrame()))
    {
      corrected = true;
      seq->forceSetStartFrame(it.value()->endFrame());
      if ((it + 1) != m_sequences.end() && (it + 1).value()->overlap(*seq))
      {
        Sequence* overlap = (it+1).value();
        if (overlap->startFrame() == seq->startFrame())
        {
          continue;
        }
        else
        {
          seq->setLength(overlap->startFrame() - seq->startFrame());
        }
      }
      else
      {
        break;
      }
    }
    else if (seq->isInside(it.value()->startFrame()))
    {
      corrected = true;
      seq->setLength(it.value()->startFrame() - seq->startFrame());
    }
  }
  m_sequences[seq->startFrame()] = seq;
  return corrected;
}

qint64 DemoTimeline::sequenceStartFrameChanged(qint64 previous_frame, Sequence *seq)
{
  m_sequences.remove(previous_frame);
  correctStartFrame(seq);
  return seq->startFrame();
}

void DemoTimeline::updateTime()
{
  updateCamera();
}


void DemoTimeline::insertCameraKeyframe(Camera *cam)
{
  if (cam)
  {
    insertCameraKeyframe(currentFrame(),cam->position(),cam->rotation(), cam->fov());
  }
}

void DemoTimeline::insertCameraKeyframe(qint64 frame, Camera *cam)
{
  if (cam)
  {
    insertCameraKeyframe(frame,cam->position(),cam->rotation(), cam->fov());
  }
}

void DemoTimeline::insertCameraKeyframe(const QVector3D& pos, const QQuaternion& rot, float fov)
{
  insertCameraKeyframe(currentFrame(),pos,rot, fov);
}


void DemoTimeline::insertCameraKeyframe(qint64 frame, const QVector3D &pos, const QQuaternion &rot, float fov)
{
  Sequence* seq = isInsideSequence(frame);
  if (seq)
  {
    seq->insertCameraKeyframe(frame-seq->startFrame(),pos,rot, fov);
  }
  else
  {
    emit warning("Warning cannot insert camera keyframes outside a sequence");
  }
}


void DemoTimeline::keyReleaseEvent(QKeyEvent *keyEvent)
{
  if (keyEvent->key() == Qt::Key_Delete)
  {
    deleteSelectedItems();
  }
  else
  {
    return Timeline::keyReleaseEvent(keyEvent);
  }
}

void DemoTimeline::deleteSelectedItems()
{
  deleteItems(selectedItems());
}

void DemoTimeline::deleteItems(const QList<QGraphicsItem *> items)
{
  foreach (QGraphicsItem* item, items)
  {
    Sequence* seq = dynamic_cast<Sequence*>(item);
    CameraKeyframe* cam_key = dynamic_cast<CameraKeyframe*>(item);

    if (seq)
    {
      deleteSequence(seq);
    }
    else if (cam_key)
    {
      cam_key->sequence()->deleteCameraKeyframe(cam_key);
    }
    else
    {
      removeItem(item);
      delete item;
    }
  }
}

void DemoTimeline::deleteSequence(Sequence* seq)
{
  QMap<qint64,Sequence*>::iterator it = m_sequences.find(seq->startFrame());
  m_node.firstChildElement("track").removeChild(seq->node()).clear();
  delete it.value();
  m_sequences.erase(it);
  m_project.notifyDocumentChanged();
}

void DemoTimeline::addSequenceAction(QAction *action)
{
  QDomElement e = m_project.document().createElement("sequence");
  e = m_node.firstChildElement("track").appendChild(e).toElement();
  Sequence* seq = new Sequence(m_project,*this,e,*(m_project.getRayMarchScene(action->text())),m_mousePressPos.x(),600,m_trackHeight);
  addSequence(seq);
}

void DemoTimeline::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  m_mousePressPos = event->scenePos();

  QMenu contextMenu;
  QMenu addSequenceMenu;
  addSequenceMenu.setTitle(tr("Add sequence"));

  QMap<QString,Scene*> scenes = m_project.rayMarchScenes();
  QMap<QString,Scene*>::const_iterator it = scenes.constBegin();
  for (;it != scenes.constEnd(); it++)
  {
    QAction* action = new QAction(it.key(),&addSequenceMenu);
    addSequenceMenu.addAction(action);
  }

  connect(&addSequenceMenu,SIGNAL(triggered(QAction*)),this,SLOT(addSequenceAction(QAction*)));
  contextMenu.addMenu(&addSequenceMenu);
  contextMenu.exec(event->screenPos());
}

qint64 DemoTimeline::maxSequenceLengthBeforeOverlap(Sequence* seq) const
{
  QMap<qint64, Sequence*>::const_iterator it = m_sequences.constFind(seq->startFrame());
  if (it != m_sequences.constEnd() && (it+1) != m_sequences.constEnd())
  {
    return (it+1).value()->startFrame()-seq->startFrame();
  }
  else if (length() > seq->startFrame())
  {
    return length() - seq->startFrame();
  }
  else
  {
    //Should not happen
    return seq->length() + 1;
  }
}


void DemoTimeline::renderImage(const QSize& resolution, QImage* target)
{
  Q_ASSERT(target);

  updateTime();
  //Cast because of compiler bug in debug...
  ((Renderer*)m_renderer)->glRender((size_t)resolution.width(),(size_t)resolution.height());
  *target = m_renderer->getImage();
}


void DemoTimeline::exportSources(const QDir &dir) const
{
  QFile cameras_header(dir.absoluteFilePath("cameras.hpp"));
  QFile cameras_source(dir.absoluteFilePath("cameras.cpp"));

  QFile sequences_header(dir.absoluteFilePath("sequences.hpp"));
  QFile sequences_source(dir.absoluteFilePath("sequences.cpp"));

  if (!cameras_header.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) ||
      !cameras_source.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) ||
      !sequences_header.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) ||
      !sequences_source.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
  {
    emit error(tr("error: cannot open files for writing"));
    return;
  }

  QTextStream cameras_header_code(&cameras_header);
  QTextStream cameras_source_code(&cameras_source);
  QTextStream sequences_header_code(&sequences_header);
  QTextStream sequences_source_code(&sequences_source);

  QMap<qint64,Sequence*>::const_iterator it;

  QString unsigned_int_type = "unsigned int";
  if (length() < (qint64)(1<<(sizeof(uint16_t)*8)))
  {
    unsigned_int_type = "unsigned short";
  }

  QString seq_count_str = "SEQUENCES_COUNT";
  qint64 cam_keyframe_count = 0;
  for (it = m_sequences.constBegin(); it != m_sequences.constEnd(); it++)
  {
     cam_keyframe_count += it.value()->cameraKeyframes().size();
  }
  QString cam_keyframe_count_str =  "CAMERA_KEYFRAMES_COUNT";


  //Camera preprocessor
  cameras_source_code << ShaderMinifier::generatedSourceString();
  cameras_source_code << "#include \"cameras.hpp\"\n\n";

  cameras_header_code << ShaderMinifier::generatedHeaderString();
  cameras_header_code << "#ifndef CAMERAS_H\n";
  cameras_header_code << "#define CAMERAS_H\n\n";
  cameras_header_code << "#define " << cam_keyframe_count_str  <<"  " << QString::number(framerate()) + "\n\n";

  //sequences preprocessor
  sequences_source_code << ShaderMinifier::generatedSourceString();
  sequences_source_code << "#include \"sequences.hpp\"\n\n";

  sequences_header_code << ShaderMinifier::generatedHeaderString();
  sequences_header_code << "#ifndef SEQUENCES_H\n";
  sequences_header_code << "#define SEQUENCES_H\n\n";
  sequences_header_code << "#include \"cameras.hpp\"\n";
  sequences_header_code << "#include \"scenes.hpp\"\n\n";
  sequences_header_code << "#define " << seq_count_str + " " << QString::number(m_sequences.size()) << "\n";
  sequences_header_code << "#define FRAMERATE " << QString::number(framerate()) << "\n\n";


  //Sequences variables
  QString seq_start_frame = "const " + unsigned_int_type + " seq_start_frame[" + seq_count_str + "]";
  QString seq_shader = "const GLuint* seq_shader[" + seq_count_str + "]";
  QString seq_start_cam_keyframe = "const " + unsigned_int_type + " seq_start_cam_keyframe[" + seq_count_str + "]";
  QString seq_cam_keyframe_count = "const " + unsigned_int_type + " seq_cam_keyframe_count[" + seq_count_str + "]";

  QList<QString*> seq_vars;
  seq_vars << &seq_start_frame << &seq_shader << &seq_start_cam_keyframe << &seq_cam_keyframe_count;
  foreach (QString* var, seq_vars)
  {
    sequences_header_code << "extern " << *var << ";\n";
    *var += " = {\n";
  }


  //Camera variables
  QString cam_frame = "const " + unsigned_int_type + " cam_frame[" + cam_keyframe_count_str + "]";
  QString cam_pos_x = "const float cam_pos_x[" + cam_keyframe_count_str + "]";
  QString cam_pos_y = "const float cam_pos_y[" + cam_keyframe_count_str + "]";
  QString cam_pos_z = "const float cam_pos_z[" + cam_keyframe_count_str + "]";
  QString cam_rot_x = "const float cam_rot_x[" + cam_keyframe_count_str + "]";
  QString cam_rot_y = "const float cam_rot_y[" + cam_keyframe_count_str + "]";
  QString cam_rot_z = "const float cam_rot_z[" + cam_keyframe_count_str + "]";
  QString cam_rot_w = "const float cam_rot_w[" + cam_keyframe_count_str + "]";
  QString cam_fov = "const float cam_fov[" + cam_keyframe_count_str + "]";

  QList<QString*> cam_vars;
  cam_vars << &cam_frame << &cam_pos_x << &cam_pos_y << &cam_pos_z
           << &cam_rot_x << &cam_rot_y << &cam_rot_z << &cam_rot_w << &cam_fov;
  foreach (QString* var, cam_vars)
  {
    cameras_header_code << "extern " << *var << ";\n";
    *var += " = {\n";
  }

  //Finalize the headers
  sequences_header_code << "#endif\n";
  cameras_header_code << "#endif\n";

  qint64 current_keyframe = 0;

  //OK now fill our tables
  for (it = m_sequences.constBegin(); it != m_sequences.constEnd(); it++)
  {
    QString comma = ",";
    if (it+1 == m_sequences.constEnd())
    {
      comma = "";
    }

    QMap<qint64, CameraKeyframe*> cam_keyframes = it.value()->cameraKeyframes();

    seq_start_frame += "   " + QString::number(it.value()->startFrame()) + comma + "\n";
    seq_shader += "   &fs_" + it.value()->glScene()->objectName() + "_program" + comma + "\n";
    seq_start_cam_keyframe += "   " + QString::number(current_keyframe) + comma + "\n";
    seq_cam_keyframe_count += "   " + QString::number(cam_keyframes.size()) + comma + "\n";

    QMap<qint64, CameraKeyframe*>::const_iterator key_it;
    for (key_it = cam_keyframes.constBegin(); key_it != cam_keyframes.constEnd(); key_it++)
    {
      cam_frame += "   " + QString::number(key_it.value()->relativeFrame()) + comma + "\n";
      cam_pos_x += "   " + QString::number(key_it.value()->position().x()) + comma + "\n";
      cam_pos_y += "   " + QString::number(key_it.value()->position().y()) + comma + "\n";
      cam_pos_z += "   " + QString::number(key_it.value()->position().z()) + comma + "\n";

      cam_rot_x += "   " + QString::number(key_it.value()->rotation().x()) + comma + "\n";
      cam_rot_y += "   " + QString::number(key_it.value()->rotation().y()) + comma + "\n";
      cam_rot_z += "   " + QString::number(key_it.value()->rotation().z()) + comma + "\n";
      cam_rot_w += "   " + QString::number(key_it.value()->rotation().scalar()) + comma + "\n";

      cam_fov += "   " + QString::number(qDegreesToRadians(key_it.value()->fov())) + comma + "\n";
    }
    current_keyframe += cam_keyframes.size();
  }

  foreach (QString* var, seq_vars)
  {
    sequences_source_code << *var << "};\n";
  }
  foreach (QString* var, cam_vars)
  {
    cameras_source_code << *var << "};\n";
  }


  cameras_header_code.flush();
  cameras_source_code.flush();
  sequences_header_code.flush();
  sequences_source_code.flush();
}




/*
  RENDERER
*/


DemoRenderer::DemoRenderer(Project& project, DemoTimeline &timeline, size_t w, size_t h):
  Renderer(w,h,&timeline),
  m_project(project),
  m_timeline(timeline)
{

}

void DemoRenderer::glRender()
{
  m_fbo.enable();

  qint64 frame = m_timeline.currentFrame();
  Sequence* current_sequence = m_timeline.isInsideSequence(frame);

  if (current_sequence && current_sequence->glScene())
  {
      float track_time = m_timeline.music() ? m_timeline.music()->getTime() : 0.0f;
    SceneRenderer::glRenderScene(*this, m_project, *(current_sequence->glScene()), m_camera, track_time, track_time - (static_cast<float>(current_sequence->startFrame())/ m_timeline.framerate()));
  }
  else
  {
    glClear(GL_COLOR_BUFFER_BIT);
  }
  m_fbo.disable();
}




