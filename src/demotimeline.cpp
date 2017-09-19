
#include "demotimeline.hpp"
#include "timelinetrack.hpp"
#include "jassert.hpp"

/*
** DemoTimelineRenderer
*/

/*
DemoTimelineRenderer::DemoTimelineRenderer(DemoTimeline&  timeline, const QSize& initialSize)
  : Renderer(),  timeline(timeline)
{

}

DemoTimeline::DemoTimeline(QDomElement &node, Project &project, double fps, LogWidget &log):
  Timeline(*(project.getMusic()),2*60.0,fps,log),
  camera(new Camera),
  node(node),
  trackHeight(60.0),
  project(&project),
  render(new DemoRender(*this,QSize(1280,720)))
{
  render->getCamera() = camera;
  load();
}
*/

DemoTimeline::DemoTimeline(QObject* parentMusic) :
  Timeline(parentMusic),
  renderCache(nullptr)
{
}

DemoTimeline::~DemoTimeline()
{
}

void DemoTimeline::initializeGL(RenderFunctionsCache& cache)
{
  renderCache = &cache;
  for (int i = 0; i < tracks.size(); i++)
  {
    tracks[i]->initializeGL(cache);
  }
}

void DemoTimeline::insertTrack(TimelineTrack* track)
{
  addItem(track); // parenting

  connect(track, &TimelineTrack::requestFramePosition, this, &DemoTimeline::trackRequestFramePosition);

  tracks.push_back(track);

  if (renderCache)
    track->initializeGL(*renderCache);

  QVariant oldValue;
  QVariant newValue = QVariant::fromValue(track);
  emit propertyChanged(this, "tracks", oldValue, newValue);
}

void DemoTimeline::removeTrack(TimelineTrack* track)
{
  for (int i = 0; i < tracks.size(); ++i)
  {
    if (tracks[i] == track)
    {
      tracks.remove(i);
      break;
    }
    jassert(i != tracks.size() - 1); // track wasn't inside this timeline
  }

  disconnect(track, &TimelineTrack::requestFramePosition, this, &DemoTimeline::trackRequestFramePosition);

  removeItem(track); // parenting

  QVariant oldValue = QVariant::fromValue(track);
  QVariant newValue;
  emit propertyChanged(this, "tracks", oldValue, newValue);
}

void DemoTimeline::trackRequestFramePosition(qint64 position)
{
  if (getFramerate())
    emit requestPosition((double)position / (double)getFramerate());
}

/*
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
  jassert(target);

  updateTime();
  //Cast because of compiler bug in debug...
  
  render->render();
  *target = render->getFBO().toImage(true, 0);
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

*/


/*
  RENDERER
*/

/*
DemoRender::DemoRender(DemoTimeline &timeline, const QSize& initialSize):
  Render(initialSize,&timeline),
  timeline(&timeline)
{

}

void DemoRender::renderChildrens() override
{

}

void DemoRender::glRender()
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

    scene->getShader().sendf("xy_scale_factor",(float)fbo.getSizeX()/(float)fbo.getSizeY());
    scene->getShader().sendf("sequence_time",(float)timeline->getMusic()->getPosition() - (float)current_sequence->startFrame()/timeline->getFramerate());
    scene->getShader().sendf("track_time",(float) timeline->getMusic()->getPosition());
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

*/



