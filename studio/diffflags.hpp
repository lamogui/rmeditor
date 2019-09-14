#ifndef DIFFFLAGS_HPP
#define DIFFFLAGS_HPP

#define PROUT_DECLARE_DIFFFLAGS( _class ) \
	_class::DiffFlags operator|( _class::DiffFlags _lhs, _class::DiffFlags _rhs ); \
	bool operator&( _class::DiffFlags _lhs, _class::DiffFlags _rhs ); \
	QDataStream& operator<<( QDataStream & _stream, _class::DiffFlags _flags ); \
	QDataStream& operator>>( QDataStream & _stream, _class::DiffFlags & _flags );

#define PROUT_DIFFFLAGS_8( _class )
	_class::DiffFlags operator|( _class::DiffFlags _lhs, _class::DiffFlags _rhs ) {return static_cast<_class::DiffFlags>(static_cast<quint8>(_lhs) | static_cast<quint8>(_rhs));} \
	bool operator&( _class::DiffFlags _lhs, _class::DiffFlags _rhs ) {return static_cast<quint8>(_lhs) & static_cast<quint8>(_rhs) ? true : false;} \
	QDataStream& operator<<( QDataStream & _stream, _class::DiffFlags _flags ) { _stream << static_cast<qint8>( _flags ) ; return _stream; } \
	QDataStream& operator>>( QDataStream & _stream, _class::DiffFlags & _flags ) { qint8 value = 0; _stream >> value; _flags = static_cast<_class::DiffFlags>( value ); return _stream; }

#define pWriteDiffStreamMember( _diffflag, _member ) \
	if ( _flags & _diffflag ) _stream << _member;

#endif // DIFFFLAGS_HPP
