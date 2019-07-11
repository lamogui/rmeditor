#include "music.hpp"
#include "demotimeline.hpp"
#include "fast2dquad.hpp"

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


DemoTimeline::DemoTimeline(QDomElement &node, Project &project, double fps, LogWidget &log):
  Timeline(*(project.getMusic()),2*60.0,fps,log),
  node(node),
  trackHeight(60.0),
  project(&project),
  renderer(new DemoRenderer(*this,1280,720))
{
  renderer->setCamera(&camera);
  load();
}

DemoTimeline::~DemoTimeline()
{
  QMap<qint64, Sequence*>::iterator it;
  for (it = sequences.begin(); it != sequences.end(); it++) //TODO use dichotomie
  {
    delete it.value(); //Maybe useless because of QGraphicsScene parenting
  }
}

void DemoTimeline::load()
{
  QDomElement element = node.firstChildElement();
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
      emit warning(QString("[") + project->fileName() + "]" + " <timeline> warning at line " + QString::number(element.lineNumber()) +
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
      Sequence* seq = new Sequence(*project,*this,element,trackHeight);
      addSequence(seq);

    }
    else
    {
        emit warning(QString(QString("[") + project->fileName() + "] <timeline> warning at line " + QString::number(element.lineNumber()) +
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
  for (it = sequences.constBegin(); it != sequences.constEnd(); it++) //TODO use dichotomie
  {
    if (it.value()->isInside(frame))
    {
      return it.value();
    }
  }
  return nullptr;
}


bool DemoTimeline::correctStartFrame(Sequence* seq)
{
  QMap<qint64, Sequence*>::iterator it;
  bool corrected = false;
  seq->setLength(seq->getOrginalLength());
  for (it = sequences.begin(); it != sequences.end(); it++) //TODO use dichotomie
  {
    if (it.value()->isInside(seq->startFrame()))
    {
      corrected = true;
      seq->forceSetStartFrame(it.value()->endFrame());
      if ((it + 1) != sequences.end() && (it + 1).value()->overlap(*seq))
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
  sequences[seq->startFrame()] = seq;
  return corrected;
}

qint64 DemoTimeline::sequenceStartFrameChanged(qint64 previous_frame, Sequence *seq)
{
  sequences.remove(previous_frame);
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
    insertCameraKeyframe(currentFrame(),cam->getPosition(),cam->getRotation());
  }
}

void DemoTimeline::insertCameraKeyframe(qint64 frame, Camera *cam)
{
  if (cam)
  {
    insertCameraKeyframe(frame,cam->getPosition(),cam->getRotation());
  }
}

void DemoTimeline::insertCameraKeyframe(const QVector3D& pos, const QQuaternion& rot)
{
  insertCameraKeyframe(currentFrame(),pos,rot);
}


void DemoTimeline::insertCameraKeyframe(qint64 frame, const QVector3D &pos, const QQuaternion &rot)
{
  Sequence* seq = isInsideSequence(frame);
  if (seq)
  {
    seq->insertCameraKeyframe(frame-seq->startFrame(),pos,rot);
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
    CameraKeyframe* cakey = dynamic_cast<CameraKeyframe*>(item);

    if (seq)
    {
      deleteSequence(seq);
    }
    else if (cakey)
    {
      cakey->getSequence()->deleteCameraKeyframe(cakey);
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
  QMap<qint64,Sequence*>::iterator it = sequences.find(seq->startFrame());
  node.firstChildElement("track").removeChild(seq->getNode()).clear();
  delete it.value();
  sequences.erase(it);
  project->notifyDocumentChanged();
}

void DemoTimeline::addSequenceAction(QAction *action)
{
  QDomElement e = project->getDocument().createElement("sequence");
  e = node.firstChildElement("track").appendChild(e).toElement();
  Sequence* seq = new Sequence(*project,*this,e,*(project->getRayMarchScene(action->text())),mousePressPos.x(),600,trackHeight);
  addSequence(seq);
}

void DemoTimeline::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  mousePressPos = event->scenePos();

  QMenu contextMenu;
  QMenu addSequenceMenu;
  addSequenceMenu.setTitle(tr("Add sequence"));

  QMap<QString,Scene*> scenes = project->rayMarchScenes();
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
  QMap<qint64, Sequence*>::const_iterator it = sequences.constFind(seq->startFrame());
  if (it != sequences.constEnd() && (it+1) != sequences.constEnd())
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
  ((Renderer*)renderer)->glRender((size_t)resolution.width(),(size_t)resolution.height());
  *target = renderer->getImage();
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
  qint64 cakeyframe_count = 0;
  for (it = sequences.constBegin(); it != sequences.constEnd(); it++)
  {
     cakeyframe_count += it.value()->getCameraKeyframes().size();
  }
  QString cakeyframe_count_str =  "CAMERA_KEYFRAMES_COUNT";


  //Camera preprocessor
  cameras_source_code << ShaderMinifier::generatedSourceString();
  cameras_source_code << "#include \"cameras.hpp\"\n\n";

  cameras_header_code << ShaderMinifier::generatedHeaderString();
  cameras_header_code << "#ifndef CAMERAS_H\n";
  cameras_header_code << "#define CAMERAS_H\n\n";
  cameras_header_code << "#define " << cakeyframe_count_str  <<"  " << QString::number(getFramerate()) + "\n\n";

  //sequences preprocessor
  sequences_source_code << ShaderMinifier::generatedSourceString();
  sequences_source_code << "#include \"sequences.hpp\"\n\n";

  sequences_header_code << ShaderMinifier::generatedHeaderString();
  sequences_header_code << "#ifndef SEQUENCES_H\n";
  sequences_header_code << "#define SEQUENCES_H\n\n";
  sequences_header_code << "#include \"cameras.hpp\"\n";
  sequences_header_code << "#include \"scenes.hpp\"\n\n";
  sequences_header_code << "#define " << seq_count_str + " " << QString::number(sequences.size()) << "\n";
  sequences_header_code << "#define FRAMERATE " << QString::number(getFramerate()) << "\n\n";


  //Sequences variables
  QString seq_start_frame = "const " + unsigned_int_type + " seq_start_frame[" + seq_count_str + "]";
  QString seq_shader = "const GLuint* seq_shader[" + seq_count_str + "]";
  QString seq_start_cakeyframe = "const " + unsigned_int_type + " seq_start_cakeyframe[" + seq_count_str + "]";
  QString seq_cakeyframe_count = "const " + unsigned_int_type + " seq_cakeyframe_count[" + seq_count_str + "]";

  QList<QString*> seq_vars;
  seq_vars << &seq_start_frame << &seq_shader << &seq_start_cakeyframe << &seq_cakeyframe_count;
  foreach (QString* var, seq_vars)
  {
    sequences_header_code << "extern " << *var << ";\n";
    *var += " = {\n";
  }


  //Camera variables
  QString caframe = "const " + unsigned_int_type + " caframe[" + cakeyframe_count_str + "]";
  QString capos_x = "const float capos_x[" + cakeyframe_count_str + "]";
  QString capos_y = "const float capos_y[" + cakeyframe_count_str + "]";
  QString capos_z = "const float capos_z[" + cakeyframe_count_str + "]";
  QString carot_x = "const float carot_x[" + cakeyframe_count_str + "]";
  QString carot_y = "const float carot_y[" + cakeyframe_count_str + "]";
  QString carot_z = "const float carot_z[" + cakeyframe_count_str + "]";
  QString carot_w = "const float carot_w[" + cakeyframe_count_str + "]";

  QList<QString*> cavars;
  cavars << &caframe << &capos_x << &capos_y << &capos_z
           << &carot_x << &carot_y << &carot_z << &carot_w;
  foreach (QString* var, cavars)
  {
    cameras_header_code << "extern " << *var << ";\n";
    *var += " = {\n";
  }

  //Finalize the headers
  sequences_header_code << "#endif\n";
  cameras_header_code << "#endif\n";

  qint64 current_keyframe = 0;

  //OK now fill our tables
  for (it = sequences.constBegin(); it != sequences.constEnd(); it++)
  {
    QString comma = ",";
    if (it == sequences.constEnd() - 1)
    {
      comma = "";
    }

    QMap<qint64, CameraKeyframe*> cakeyframes = it.value()->getCameraKeyframes();

    seq_start_frame += "   " + QString::number(it.value()->startFrame()) + comma + "\n";
    seq_shader += "   &fs_" + it.value()->glScene()->objectName() + "_program" + comma + "\n";
    seq_start_cakeyframe += "   " + QString::number(current_keyframe) + comma + "\n";
    seq_cakeyframe_count += "   " + QString::number(cakeyframes.size()) + comma + "\n";

    QMap<qint64, CameraKeyframe*>::const_iterator key_it;
    for (key_it = cakeyframes.constBegin(); key_it != cakeyframes.constEnd(); key_it++)
    {
      caframe += "   " + QString::number(key_it.value()->relativeFrame()) + comma + "\n";
      capos_x += "   " + QString::number(key_it.value()->getPosition().x()) + comma + "\n";
      capos_y += "   " + QString::number(key_it.value()->getPosition().y()) + comma + "\n";
      capos_z += "   " + QString::number(key_it.value()->getPosition().z()) + comma + "\n";

      carot_x += "   " + QString::number(key_it.value()->getRotation().x()) + comma + "\n";
      carot_y += "   " + QString::number(key_it.value()->getRotation().y()) + comma + "\n";
      carot_z += "   " + QString::number(key_it.value()->getRotation().z()) + comma + "\n";
      carot_w += "   " + QString::number(key_it.value()->getRotation().scalar()) + comma + "\n";
    }
    current_keyframe += cakeyframes.size();
  }

  foreach (QString* var, seq_vars)
  {
    sequences_source_code << *var << "};\n";
  }
  foreach (QString* var, cavars)
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


DemoRenderer::DemoRenderer(DemoTimeline &timeline, size_t w, size_t h):
  Renderer(w,h,&timeline),
  timeline(&timeline)
{

}

void DemoRenderer::glRender()
{
  fbo.enable();

  qint64 frame = timeline->currentFrame();
  Sequence* current_sequence = timeline->isInsideSequence(frame);

  if (current_sequence && current_sequence->glScene())
  {
    Scene* scene = current_sequence->glScene();
    scene->getShader().enable();


    timeline->getMusic()->updateTextures();

    glActiveTexture(GL_TEXTURE0 + 0);
    timeline->getMusic()->getNoteVelocityTex().bind();
    scene->getShader().sendi("notes_velocity",0);

    if (camera)
    {
      scene->getShader().sendf("cam_position",camera->getPosition().x(),camera->getPosition().y(), camera->getPosition().z());
      scene->getShader().sendf("cam_rotation",camera->getRotation().x(),camera->getRotation().y(), camera->getRotation().z(), camera->getRotation().scalar());
    }
    else
    {
      scene->getShader().sendf("cam_rotation",0.f,0.f,0.f,1.f);
    }

    scene->getShader().sendf("resolution",(float)fbo.getSizeX(), (float)fbo.getSizeY());
    scene->getShader().sendf("sequence_time",(float)timeline->getMusic()->getTime() - (float)current_sequence->startFrame()/timeline->getFramerate());
    scene->getShader().sendf("track_time",(float) timeline->getMusic()->getTime());
    Fast2DQuadDraw();

    glActiveTexture(GL_TEXTURE0+0);
    glBindTexture(GL_TEXTURE_2D, 0);

    scene->getShader().disable();
  }
  else
  {
    glClear(GL_COLOR_BUFFER_BIT);
  }
  fbo.disable();
}




