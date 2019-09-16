#ifndef DIFFFLAGS_HPP
#define DIFFFLAGS_HPP

#define PROUT_DECLARE_DIFFFLAGS( _class ) \
	_class::DiffFlags operator|( _class::DiffFlags _lhs, _class::DiffFlags _rhs ); \
	bool operator&( _class::DiffFlags _lhs, _class::DiffFlags _rhs ); \
	QDataStream& operator<<( QDataStream & _stream, _class::DiffFlags _flags ); \
	QDataStream& operator>>( QDataStream & _stream, _class::DiffFlags & _flags );

#define PROUT_DIFFFLAGS_8( _class ) \
	_class::DiffFlags operator|( _class::DiffFlags _lhs, _class::DiffFlags _rhs ) {return static_cast<_class::DiffFlags>(static_cast<quint8>(_lhs) | static_cast<quint8>(_rhs));} \
	bool operator&( _class::DiffFlags _lhs, _class::DiffFlags _rhs ) {return static_cast<quint8>(_lhs) & static_cast<quint8>(_rhs) ? true : false;} \
	QDataStream& operator<<( QDataStream & _stream, _class::DiffFlags _flags ) { _stream << static_cast<qint8>( _flags ) ; return _stream; } \
	QDataStream& operator>>( QDataStream & _stream, _class::DiffFlags & _flags ) { qint8 value = 0; _stream >> value; _flags = static_cast<_class::DiffFlags>( value ); return _stream; }

#define pReadStreamKey( _key ) \
	_stream >> _key;

// File
#define pWriteFileStream( _member ) \
	_stream << _member;

#define pReadFileStream( _sender, _property, _signal )  PROUT_BLOCK_WITH_FORCED_SEMICOLON( \
	_stream >> _property; \
	if ( _stream.status() != QDataStream::Status::Ok ) { \
		perror( Log::File, _sender, tr("Failed to read ") + _sender->objectName() + " " + QString(_sender->metaObject()->className()) + "::"#_property + tr("(corrupted file ?)"));  \
		return false; \
	}  \
	emit _signal(*this); )

#define pReadFileStreamPointer( _sender, _idType, _property, _load ) PROUT_BLOCK_WITH_FORCED_SEMICOLON( \
	_idType id_##_property = static_cast< _idType >(-1); \
	_stream >> id_##_property; \
	if ( _stream.status() != QDataStream::Status::Ok ) { \
		perror( Log::File, _sender, tr("Failed to read ") + _sender->objectName() + " " + QString(_sender->metaObject()->className()) + "::"#_property + tr("(corrupted file ?)"));  \
		return false; \
	}  \
	_load( id_##_property ); )

// Diff
#define pWriteDiffStreamMember( _diffflag, _member ) \
	if ( _flags & _diffflag ) _stream << _member;

#define pReadDiffStream( _flag, _propertyType, _property, _signal ) \
	if ( flags & _flag ) { \
		if ( _undoStream ) { \
			_propertyType _##_property; \
			_stream >> _##_property; \
			*_undoStream << _property; \
		} else { \
			_stream >> _property; \
			emit _signal( *this ); \
		} \
	} // TODO check ! _stream and _undoStream status !

#define pReadDiffStreamControled( _flag, _propertyType, _property, _signal, _control ) \
	if ( flags & _flag ) { \
		if ( _undoStream ) { \
			_propertyType new_##_property; \
			_stream >> new_##_property; \
			*_undoStream << _property; \
			if ( (*_control)( _property, new_##_property ) ) { \
				perror(Log::Network, this, tr( "Could not resolve diff for ") +  objectName() +  " member " + metaObject()->className() + "::" + #_property ); \
				return false; \
			} \
		} else { \
			_stream >> _property; \
			emit _signal( *this ); \
		}  \
	}// TODO check ! _stream and _undoStream status !

#define pReadDiffStreamControledWithDependencies( _propertyType, _property ) \
	if ( _undoStream ) { \
		_stream >> new_##_property; \
		*_undoStream << _property; \
	} else { \
		_stream >> _property; \
	}  // TODO check ! _stream and _undoStream status !

#define pReadDiffStreamSize( _sizeType, _size ) \
	_sizeType _size = 0; \
	_stream >> _size; \
	if ( _undoStream ) { \
		*_undoStream << _size; \
	} // TODO check ! _stream and _undoStream status !

#define pReadDiffStreamFlags( _undoFlags ) \
	DiffFlags flags = static_cast<DiffFlags>(0); \
	_stream >> flags; \
	if ( _undoStream ) { \
		*_undoStream << _undoFlags( flags ); \
	} // TODO check ! _stream and _undoStream status !

#define pReadDiffStreamPointer( _flag, _idType, _ptr, _load, _get ) \
	if ( flags & _flag ) { \
		_idType id_##_ptr; \
		_stream >> id_##_ptr; \
		if ( _undoStream ) { \
			*_undoStream << _get( _ptr ); \
		} else { \
			_load( id_##_ptr ); \
		} \
	} // TODO check ! _stream and _undoStream status !


#endif // DIFFFLAGS_HPP
