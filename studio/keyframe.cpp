﻿
#include "keyframe.hpp"
#include "logmanager.hpp"
#include "sequence.hpp"


/*
** Keyframe
*/

PROUT_DIFFFLAGS_8(Keyframe)

Keyframe::Keyframe(Sequence & _parent) :
	QObject( &_parent )
{

}

bool Keyframe::loadFromFileStream(quint16 _version, QDataStream & _stream)
{
	(void) _version;
	preadstream( Log::File, this, m_position )
	emit positionChanged( *this );

	preadstream( Log::File, this, m_content )
	emit contentChanged( *this );
	return true;
}

void Keyframe::writeToFileStream( QDataStream & _stream ) const
{
	_stream << m_position;
	_stream << m_content;
}

bool Keyframe::loadFromDiffStream( QDataStream &_stream )
{
	DiffFlags flags;
	preadstream( Log::Network, this, flags )

	if ( flags & DiffFlags::POSITION )
	{
		preadstream( Log::Network, this, m_position )
		emit positionChanged( *this );
	}

	if ( flags & DiffFlags::CONTENT )
	{
		preadstream( Log::Network, this, m_content )
		emit contentChanged( *this );
	}
}

void Keyframe::writeLocalDiffToStream( DiffFlags _flags, QDataStream & _stream ) const
{
	_stream << _flags;

	pWriteDiffStreamMember( DiffFlags::POSITION, m_position )
	pWriteDiffStreamMember( DiffFlags::CONTENT, m_content )
}

#include <QPainter>

/*
 * KeyframeWidget
 */

KeyframeWidget::KeyframeWidget( SequenceWidget & _parent, const Keyframe & _target, int _y ) :
	QWidget( &_parent ),
	m_target( _target ),
	m_y( _y )
{
	setSizePolicy( QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed );

	connect(&m_target, &Keyframe::positionChanged, this, &KeyframeWidget::onTargetChanged );
	onTargetChanged();
}

void KeyframeWidget::onTargetChanged()
{
	setPosition( m_target.getPosition(), static_cast<SequenceWidget*>(parent())->getZoomLevel() );
}

void KeyframeWidget::onZoomLevelChanged(qreal _zoomLevel)
{
	setPosition( m_target.getPosition(), _zoomLevel );
}

void KeyframeWidget::setPosition( qint64 _relativePosition, qreal _zoomLevel )
{
	setGeometry( QRect( QPoint( static_cast<int>( _relativePosition * _zoomLevel ) - c_width / 2 , m_y ) , sizeHint() ) );
}

QSize KeyframeWidget::sizeHint() const
{
	return QSize( c_width, c_height );
}

void KeyframeWidget::paintEvent(QPaintEvent* /*_event*/)
{
	QPainter painter( this );
	painter.setPen(QColor(0,0,0));
	painter.setBrush(QBrush(QColor(42,166,40)));
	painter.drawPolygon( c_vertices, c_numVertices );
}

