#include "Precompiled.h"
#include "Buffer.h"

Buffer::Buffer() : m_data( NULL ), m_size( 0 ) {
}

Buffer::Buffer( unsigned int _size ) : m_data( NULL ), m_size( 0 ) {
	Resize( _size );
}

Buffer::~Buffer() {
	if ( m_data != NULL ) {
		free( m_data );
	}
}

void Buffer::EnsureAllocated( unsigned int _size ) {
	if ( _size > m_size ) {
		unsigned int newSize = MAX( 8, m_size << 1 );
		while ( newSize < _size ) {
			newSize <<= 1;
		}
		Resize( newSize );
	}
}

void Buffer::Resize( unsigned int _size ) {
#ifdef _DEBUG
	if ( _size > 100 * 1024 * 1024 ) {
		DebugBreak();
	}
#endif // DEBUG
	if ( _size != m_size ) {
		unsigned char * newData = _size > 0 ? static_cast< unsigned char * >( malloc( _size ) ) : NULL;
		if ( m_data != NULL ) {
			const unsigned int sizeToCopy = MIN( m_size, _size );
			if ( sizeToCopy > 0 ) {
				memcpy( newData, m_data, sizeToCopy );
			}
			free( m_data );
		}
		m_data = newData;
		m_size = _size;
	}
}
