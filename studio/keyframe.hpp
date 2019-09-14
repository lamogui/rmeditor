#ifndef KEYFRAME_HPP
#define KEYFRAME_HPP

#include <QObject>
#include <QVariant>

#include "diffflags.hpp"

class Sequence;
class Keyframe : public QObject
{
	Q_OBJECT

public:
	Keyframe(Sequence & _parent);

	enum class DiffFlags
	{
		POSITION = 1 << 0,
		CONTENT = 1 << 1
	};

	// Data
	bool loadFromFileStream( quint16 _version, QDataStream & _stream );
	void writeToFileStream( QDataStream & _stream ) const;
	bool loadFromDiffStream( QDataStream & _stream );
	void writeLocalDiffToStream( DiffFlags _flags, QDataStream & _stream ) const;

	// Accessors
	qint64 getPosition() const { return m_position; }
	const QVariant & getContent() const { return m_content; }

	// Utils
	static bool compare(QList<Keyframe *>::const_iterator & _a, qint64 _b) {
		return (*_a)->getPosition() < _b;
	}

signals:
	// update the view
	void positionChanged(const Keyframe&);
	void contentChanged(const Keyframe&);

protected:
	// property
	qint64 m_position; // In frames, relative to parent
	QVariant m_content;
};

PROUT_DECLARE_DIFFFLAGS(Keyframe);

#include <QWidget>
class SequenceWidget;
class KeyframeWidget final : public QWidget
{
	Q_OBJECT

public:
	KeyframeWidget( SequenceWidget & _parent, const Keyframe & _target, int _y );

	static const int c_width = 10;
	static const int c_height = 15;
	static const size_t c_numVertices = 6;
	static constexpr QPointF c_vertices[ c_numVertices ]  = {
		QPointF( c_width * 0.5,  0.0 ),
		QPointF( c_width , c_height / 3.0 ),
		QPointF( c_width , c_height * 2.0 / 3.0 ),
		QPointF( c_width * 0.5, c_height ),
		QPointF( 0.0,  c_height * 2.0 / 3.0 ),
		QPointF( 0.0,  c_height / 3.0 ),
	} ;

protected slots:
	void onTargetChanged();
	void onZoomLevelChanged(qreal _zoomLevel);

protected:
	void setPosition(qint64 _relativePosition, qreal _zoomLevel);

	// QWidget
	QSize sizeHint() const override;
	void paintEvent(QPaintEvent *event) override;

private:
	const Keyframe & m_target;
	const int m_y;
};

#endif
