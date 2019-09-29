#include "sequence.hpp"
#include "demotimeline.hpp"
#include "logmanager.hpp"
#include "project.hpp"

/*
 *  Sequence
 */

PROUT_DIFFFLAGS_8(Sequence)



Sequence::Sequence( DemoTimeline & _parent ) :
	QObject( &_parent )
{
}

void Sequence::loadSceneFromIndex( qint16 _index )
{
	const Project& project = *static_cast<Project*>(parent()->parent());
	m_scene = project.getRmScene( _index );
	emit sceneChanged(*this);
}

// This one is quite complex and is used to avoid duplicate 2 or 3 time the nearly similar code...
bool Sequence::loadOrControlInsertCameraKeyframe(QDataStream & _stream, quint16 _version, QDataStream * _undoStream, QList<qint64>* _insertedKeyframes )
{
	bool fromFile = !_undoStream && (_version != static_cast<quint16>(-1));
	passert(Log::Code, this, (!_undoStream && !_insertedKeyframes) || (_undoStream && _insertedKeyframes ));

	pReadDiffStreamSize( quint16, keyframeCount )

	bool needSort = (!fromFile && !_undoStream);
	qint64 previousFrame=-1;

	if ( _insertedKeyframes ) {
		_insertedKeyframes->reserve( keyframeCount );
	}

	for (quint16 i = 0; i < keyframeCount; ++i )
	{
		Keyframe* keyframe = new Keyframe( *this );
		if (!keyframe->loadFromFileStream(_version, _stream)) // newly created keyframe doesn't load using diff code path and should by definition not fail...
		{
			delete keyframe;
			return false;
		}

		if ( fromFile ) {
			if ( !controlInsertCameraKeyframe(keyframe->getPosition(), nullptr ) )
			{
				perrorp(Log::File, this, keyframe->getPosition(), tr( "Sequence contains duplicate keyframe at frame " ) + QString::number( keyframe->getPosition() ));
				delete keyframe;
			}
			else
			{
				if (keyframe->getPosition() < previousFrame){
					needSort = true;
				}
				previousFrame = keyframe->getPosition();
				m_cameraKeyframes.push_back( keyframe );
			}
		} else { // from diff
			if (_undoStream) { // control
				*_undoStream << keyframe->getPosition(); // indicate key to destroy in undo
				if ( !controlInsertCameraKeyframe(keyframe->getPosition(), _insertedKeyframes ) )
				delete keyframe;
			} else {
				m_cameraKeyframes.push_back( keyframe );
				emit cameraKeyframeInserted(*keyframe,*this);
			}
		}
	}
	if (needSort)
	{
		if ( fromFile ) {
			pwarning(Log::File, this, tr("Keyframes not sorted in the file !"));
		}
		std::sort( m_cameraKeyframes.begin(),  m_cameraKeyframes.end(), Keyframe::compare );
	}
	if ( fromFile ) {
		emit cameraKeyframesLoaded(*this);
	}
}

bool Sequence::loadFromFileStream( quint16 _version, QDataStream & _stream )
{
	// clear
	emit cameraKeyframesAllDeleted( *this );
	qDeleteAll(m_cameraKeyframes);
	m_cameraKeyframes.clear();

	// load
	pReadFileStream( this, m_sequenceBlock, sequenceBlockChanged );
	pReadFileStreamPointer( this, qint16, m_scene, loadSceneFromIndex );

	if (!loadOrControlInsertCameraKeyframe( _stream, _version )) {
		return false;
	}

	return true;
}

void Sequence::writeToFileStream( QDataStream & _stream ) const
{
	pWriteFileStream( m_sequenceBlock )

	const Project& project = *static_cast<Project*>(parent()->parent());
	int indexOfRmScene = project.indexOfRmScene( m_scene.data() );
	passertmsg( Log::Code, this, indexOfRmScene < 1 << 15, tr("Limit of scenes number reached !") );
	pWriteFileStream( static_cast<qint16>(indexOfRmScene) )

	int cameraKeyframeCount = m_cameraKeyframes.size();
	passertmsg( Log::Code, this, cameraKeyframeCount  < (1 << 16), tr( "Limit of keyframes number reached !" ) );
	pWriteFileStream( static_cast<qint16>(cameraKeyframeCount) )

	for (auto it = m_cameraKeyframes.cbegin(); it != m_cameraKeyframes.cend(); ++it)
	{
		(*it)->writeToFileStream(_stream);
	}
}

bool Sequence::loadOrControlFromDiffStream(QDataStream & _stream, QDataStream * _undoStream, const std::function<bool(Block_t&, Block_t&)>* _controlMoveSequence )
{
	auto undoFlags = [](DiffFlags _f) {
		DiffFlags f = static_cast<DiffFlags>(0);
		if (_f & DiffFlags::SEQUENCE_BLOCK) f = f | DiffFlags::SEQUENCE_BLOCK;
		if (_f & DiffFlags::SCENE) f = f | DiffFlags::SCENE;
		if (_f & DiffFlags::CAMERA_KEYFRAMES_DELETED) f = f | DiffFlags::CAMERA_KEYFRAMES_INSERTED;
		if (_f & DiffFlags::CAMERA_KEYFRAMES_MODIFIED) f = f | DiffFlags::CAMERA_KEYFRAMES_MODIFIED;
		if (_f & DiffFlags::CAMERA_KEYFRAMES_INSERTED) f = f | DiffFlags::CAMERA_KEYFRAMES_DELETED;
		return f;
	};
	pReadDiffStreamFlags( undoFlags )
	if (!pverify(Log::Network, this, !(flags & DiffFlags::CAMERA_KEYFRAMES_DELETED && flags & DiffFlags::CAMERA_KEYFRAMES_INSERTED))) {
		return false;
	}

	pReadDiffStreamControled( DiffFlags::SEQUENCE_BLOCK, Block_t, m_sequenceBlock, sequenceBlockChanged, _controlMoveSequence )
	pReadDiffStreamPointer( DiffFlags::SCENE, qint16, m_scene, loadSceneFromIndex, static_cast<Project*>(parent()->parent())->indexOfRmScene )

	QList<qint64> _insertedKeyframes;
	QList<qint64> _deletedKeyframes;

	if (flags & DiffFlags::CAMERA_KEYFRAMES_DELETED) {
		pReadDiffStreamSize( qint16, keyframeCount )

		for (qint16 i = 0; i < keyframeCount; ++i ) {
			qint64 keyframeFrame = -1;
			pReadStreamKey( keyframeFrame )

			Keyframe* keyframe = findCameraKeyframe( keyframeFrame );
			if ( !keyframe ){
				perror(Log::Network,this, tr("Unable to find the keyframe to delete at frame ") + QString::number(keyframeFrame));
				if ( _undoStream ) { // Control failed !
					return false;
				}
				continue;
			}

			if ( _undoStream ) {
				keyframe->writeToFileStream( *_undoStream ); // undoStream will insert keyframes instead of delete thems...
				if ( !controlDeleteCameraKeyframe( keyframe->getPosition(), &_deletedKeyframes ) ) {
					perror(Log::Network,this, tr("Unable to find the keyframe to delete at frame ") + QString::number(keyframe->getPosition()));
					return false;
				}
			}
			else if ( keyframe ){
				emit cameraKeyframeDeleted(*keyframe,*this);
				m_cameraKeyframes.removeOne( keyframe );
				delete keyframe;
			}
		}
	}
	else if ( flags & DiffFlags::CAMERA_KEYFRAMES_INSERTED ) {
		if (!loadOrControlInsertCameraKeyframe( _stream, static_cast<quint16>(-1), _undoStream, &_insertedKeyframes)) {
			return false;
		}
	}

	if ( flags & DiffFlags::CAMERA_KEYFRAMES_MODIFIED ) {
		pReadDiffStreamSize( qint16, keyframeCount )

		auto controlKeyframeMove = [&](qint64 _before, qint64 _after){
			return controlMoveCameraKeyframe( _before, _after, &_deletedKeyframes, &_insertedKeyframes );
		};
		std::function<bool(qint64, qint64)> controlKeyframeMoveFunction(controlKeyframeMove);

		for (qint16 i = 0; i < keyframeCount; ++i ) {
			qint64 keyframeFrame = -1;
			pReadStreamKey( keyframeFrame )

			Keyframe* keyframe = findCameraKeyframe( keyframeFrame );
			if ( !keyframe ){
				perror(Log::Network,this, tr("Unable to find the keyframe to update at frame ") + QString::number(keyframeFrame));
				return false; // can't read the stream following...
			}

			if (!keyframe->loadOrControlFromDiffStream(_stream, _undoStream, &controlKeyframeMoveFunction) ) { // Delegate the rest to child objects !
				return false;
			}
		}
	}
}

void Sequence::writeLocalDiffToStream( DiffFlags _flags, QDataStream & _stream ) const
{
	_stream << _flags;
	pWriteDiffStreamMember( DiffFlags::SEQUENCE_BLOCK, m_sequenceBlock )

	const Project& project = *static_cast<Project*>(parent()->parent());
	qint16 sceneIndex = static_cast<qint16>( project.indexOfRmScene(m_scene) );
	pWriteDiffStreamMember( DiffFlags::SCENE, sceneIndex )
}


bool Sequence::controlDeleteCameraKeyframe( qint64 _frame, QList<qint64>* _deletedKeyframes ) const
{
	if ( findCameraKeyframe( _frame ) != nullptr && !(_deletedKeyframes && _deletedKeyframes->contains( _frame ) ) ) {
		if ( _deletedKeyframes ) {
			_deletedKeyframes->append( _frame );
		}
		return true;
	}
	return false;
}

bool Sequence::controlInsertCameraKeyframe(qint64 _frame, QList<qint64>* _insertedKeyframes ) const
{
	if ( findCameraKeyframe( _frame ) == nullptr && !( _insertedKeyframes && _insertedKeyframes->contains( _frame ) ) )
	{
		if ( _insertedKeyframes ) {
			_insertedKeyframes->append( _frame );
		}
		return true;
	}
	return false;
}

bool Sequence::controlMoveCameraKeyframe( qint64 _before, qint64 _after, QList<qint64>* _deletedKeyframes, QList<qint64>* _insertedKeyframes ) const
{
	if ( ( findCameraKeyframe( _before ) != nullptr || _insertedKeyframes->contains( _before ) ) &&
			 ( findCameraKeyframe( _after ) == nullptr || _deletedKeyframes->contains( _after ) ) ) {
		_deletedKeyframes->append( _before ); _deletedKeyframes->removeAll( _after );
		_insertedKeyframes->append( _after ); _insertedKeyframes->removeAll( _before );
		return true;
	}
	return false;
}

QList<Keyframe *>::const_iterator Sequence::upperCameraKeyframe( qint64 _value ) const {
	return std::lower_bound(m_cameraKeyframes.cbegin(), m_cameraKeyframes.cend(), _value, Keyframe::compare );
}

Keyframe* Sequence::findCameraKeyframe(qint64 _value) const {
	QList<Keyframe *>::const_iterator it = upperCameraKeyframe( _value );
	if ( it == m_cameraKeyframes.cend() || (*it)->getPosition() != _value) {
		return nullptr;
	}
	return *it;
}

#include "demotimelinewidget.hpp"

/*
 * SequenceWidget
 */

SequenceWidget::SequenceWidget(DemoTimelineWidget & _parent, const Sequence& _target) :
	QWidget( &_parent ), m_target(_target), m_height( 80 ), m_y( 0 )
{

	setSizePolicy( QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed );

	connect( &m_target, &Sequence::sequenceBlockChanged, this, &SequenceWidget::onTargetChanged );
	//connect( &_target, &Sequence::sceneChanged, this, SequenceWidget:: );
	connect( &m_target, &Sequence::cameraKeyframeInserted, this, &SequenceWidget::onInsertCameraKeyframe );
	connect( &m_target, &Sequence::cameraKeyframeDeleted, this, &SequenceWidget::onDeleteCameraKeyframe );
	connect( &m_target, &Sequence::cameraKeyframesLoaded, this, &SequenceWidget::onLoadAllKeyframes );
	connect( &m_target, &Sequence::cameraKeyframesAllDeleted, this, &SequenceWidget::onDeleteAllKeyframes );

	onTargetChanged();
	onLoadAllKeyframes();
}

DemoTimelineWidget& SequenceWidget::getTimelineWidget() const {
	return *static_cast<DemoTimelineWidget *>( parent() );
}

QSize SequenceWidget::sizeHint() const {
	const qreal zoomLevel = getTimelineWidget().getZoomLevel();
	return QSize( static_cast<int>( m_target.getLength() * zoomLevel ), m_height );
}

void SequenceWidget::paintEvent(QPaintEvent *_event) {
	QPainter painter(this);

	QBrush fillBrush(/*isSelected() ? selectedBrush() :*/ idleBrush());
	const qreal zoomLevel = getTimelineWidget().getZoomLevel();
	QRectF rect( QPointF(m_target.getStartFrame() * zoomLevel, m_y ) , sizeHint() );
	painter.fillRect(rect, fillBrush);
	painter.drawRect(rect);

	QRectF previewRect(QPointF(5*zoomLevel,5),QSizeF(m_preview.size()));
	previewRect.setWidth(previewRect.width()*zoomLevel);
	if (previewRect.bottom() > rect.bottom() - 5)
	{
		previewRect.setBottom(rect.bottom() - 5);
		previewRect.setWidth(rect.width() * previewRect.height() / static_cast<qreal>(m_preview.size().height()));
	}
	if (previewRect.width() > (rect.width() - 10*zoomLevel))
	{
		previewRect.setWidth((rect.width() - 10*zoomLevel));
	}
	if (m_target.getLength() * zoomLevel > 10.0 )
	{
		painter.drawImage(previewRect, m_preview);
	}
	QFont textFont;
	painter.setFont(textFont);
	QRectF textRect = QRectF(previewRect.right() + 5*zoomLevel,previewRect.top(), previewRect.right()-(previewRect.right()+5*zoomLevel), rect.height());
	if (!m_target.getScene().isNull())
	{
		painter.drawText(textRect,Qt::AlignLeft|Qt::AlignTop,m_target.getScene().data()->objectName() + " (" + m_target.getScene()->getPath().fileName() + ")");
	}
	else
	{
		painter.drawText(textRect,Qt::AlignLeft|Qt::AlignTop,QString("Missing media"));
	}
}

void 	SequenceWidget::mouseDoubleClickEvent(QMouseEvent* _event) {
	QWidget::mouseDoubleClickEvent(_event);
}

void 	SequenceWidget::mouseMoveEvent(QMouseEvent* _event) {
	QWidget::mouseMoveEvent(_event);
	/*
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
	*/
}

void 	SequenceWidget::mousePressEvent(QMouseEvent* _event) {
	QWidget::mousePressEvent(_event);
	/*
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
	*/
}

void 	SequenceWidget::mouseReleaseEvent(QMouseEvent* _event) {
	QWidget::mouseReleaseEvent(_event);
	/*
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
	*/
}

void SequenceWidget::setPosition(qint64 _relativePosition, qreal _zoomLevel)
{
	setGeometry( QRect( QPoint( static_cast<int>( _relativePosition * _zoomLevel ), m_y ) , sizeHint() ) );
}

void SequenceWidget::onTargetChanged() {
	setPosition( m_target.getStartFrame(), getTimelineWidget().getZoomLevel() );
}

void SequenceWidget::onZoomLevelChanged(qreal _zoomLevel) {
	setPosition( m_target.getStartFrame(), _zoomLevel );
}

void SequenceWidget::onInsertCameraKeyframe(const Keyframe & _keyframe) {
	m_cameraKeyframes.append( new KeyframeWidget( *this, _keyframe, 50 ) );
}

void SequenceWidget::onDeleteCameraKeyframe(const Keyframe & _keyframe) {
	const int numKeyframes = m_cameraKeyframes.size();
	for ( int i = 0; i < numKeyframes; ++i ) {
		if ( &_keyframe == &( m_cameraKeyframes[ i ]->getTarget() ) ) {
			delete m_cameraKeyframes[ i ];
			m_cameraKeyframes.removeAt( i );
			return;
		}
	}
}

void SequenceWidget::onDeleteAllKeyframes() {
	for ( int i = 0; i < m_cameraKeyframes.size(); ++i ) {
		delete m_cameraKeyframes[ i ];
	}
	m_cameraKeyframes.clear();
}

void SequenceWidget::onLoadAllKeyframes() {
	onDeleteAllKeyframes();
	const int numKeyframes = m_target.getNumCameraKeyframes();
	m_cameraKeyframes.reserve( numKeyframes );
	for ( int i = 0; i < numKeyframes; ++i ) {
		onInsertCameraKeyframe( m_target.getCameraKeyframe( i ) );
	}
}

void SequenceWidget::initializeGL(RenderFunctionsCache& _renderCache)
{
	this->m_renderCache = &_renderCache;
	renderImages();
	if ( m_renderer ) {
		m_renderer ->initializeGL(_renderCache);
	}
}

void SequenceWidget::renderImages()
{
	passert( Log::Code, this, m_renderCache );
	if ( m_renderer )
  {
		/*
		int height = static_cast<int>(m_height - 10);
    RenderTexture2D render;
		render.initializeGL(*m_renderCache, QSize(4 * height / 3, height));
		m_preview = render.getImage();
		render.render(*m_renderCache, *m_renderer);
		*/
  }
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

void SequenceWidget::setFramePosition(qint64 framePosition)
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
*/

