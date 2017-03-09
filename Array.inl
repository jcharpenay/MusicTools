template< class T > Array< T >::Array() : m_data( NULL ), m_num( 0 ), m_size( 0 ) {
}

template< class T > Array< T >::Array( const Array< T > & _other ) : m_data( NULL ), m_num( 0 ), m_size( 0 ) {
	operator+=( _other );
}

template< class T > Array< T >::~Array() {
	if ( m_data != NULL ) {
		delete[] m_data;
	}
}

template< class T > void Array< T >::operator=( const Array< T > & _other ) {
	Clear();
	operator+=( _other );
}

template< class T > void Array< T >::operator+=( const Array< T > & _other ) {
	EnsureAllocated( m_num + _other.m_num );
	for ( unsigned int index = 0; index < _other.m_num; index++ ) {
		Add( _other[ index ] );
	}
}

template< class T > T & Array< T >::Add() {
	EnsureAllocated( m_num + 1 );
	return m_data[ m_num++ ];
}

template< class T > T & Array< T >::Add( const T & _item ) {
	EnsureAllocated( m_num + 1 );
	m_data[ m_num ] = _item;
	return m_data[ m_num++ ];
}

template< class T > bool Array< T >::Remove( const T & _item ) {
	for ( unsigned int index = 0; index < m_num; index++ ) {
		if ( m_data[ index ] == _item ) {
			for ( index++; index < m_num; index++ ) {
				m_data[ index - 1 ] = m_data[ index ];
			}
			m_num--;
			return true;
		}
	}
	return false;
}

template< class T > void Array< T >::Clear() {
	m_num = 0;
}

template< class T > void Array< T >::Sort( SortBase< T > & _sort ) {
	_sort.Sort( *this );
}

template< class T > void Array< T >::EnsureAllocated( unsigned int _size ) {
	if ( _size > m_size ) {
		unsigned int newSize = MAX( 8, m_size << 1 );
		while ( newSize < _size ) {
			newSize <<= 1;
		}
		Resize( newSize );
	}
}

template< class T > void Array< T >::Resize( unsigned int _size ) {
#ifdef _DEBUG
	if ( _size > 1000 ) {
		DebugBreak();
	}
#endif // DEBUG
	if ( _size != m_size ) {
		T * newData = _size > 0 ? new T[ _size ] : NULL;
		const unsigned int newNum = MIN( m_num, _size );
		if ( m_data != NULL ) {
			for ( unsigned int index = 0; index < newNum; index++ ) {
				newData[ index ] = m_data[ index ];
			}
			delete[] m_data;
		}
		m_data = newData;
		m_num = newNum;
		m_size = _size;
	}
}
