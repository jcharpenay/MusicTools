template< class T > void QuickSort< T >::Sort( Array< T > & _array ) {
	Sort( _array, 0, _array.NumItems() );
}

template< class T > void QuickSort< T >::Sort( Array< T > & _array, unsigned int _p, unsigned int _q ) {
	if ( _p < _q ) {
		const unsigned int r = Partition( _array, _p, _q );
		Sort( _array, _p, r );
		Sort( _array, r + 1, _q );
	}
}

template< class T > unsigned int QuickSort< T >::Partition( Array< T > & _array, unsigned int _p, unsigned int _q ) {
	const T & x = _array[ _p ];
	unsigned int i = _p;
	unsigned int j;

	for ( j = _p + 1; j < _q; j++ ) {
		if ( _array[ j ] <= x ) {
			i++;
			std::swap( _array[ i ], _array[ j ] );
		}
	}

	std::swap( _array[ i ], _array[ _p ] );

	return i;
}

template< class T > void QuickSortWithCallback< T >::Sort( Array< T > & _array ) {
	Sort( _array, 0, _array.NumItems() );
}

template< class T > void QuickSortWithCallback< T >::Sort( Array< T > & _array, unsigned int _p, unsigned int _q ) {
	if ( _p < _q ) {
		const unsigned int r = Partition( _array, _p, _q );
		Sort( _array, _p, r );
		Sort( _array, r + 1, _q );
	}
}

template< class T > unsigned int QuickSortWithCallback< T >::Partition( Array< T > & _array, unsigned int _p, unsigned int _q ) {
	const T & x = _array[ _p ];
	unsigned int i = _p;
	unsigned int j;

	for ( j = _p + 1; j < _q; j++ ) {
		if ( _array[ j ] <= x ) {
			i++;
			std::swap( _array[ i ], _array[ j ] );
			m_swapCallback( m_ptr, i, j );
		}
	}

	std::swap( _array[ i ], _array[ _p ] );
	m_swapCallback( m_ptr, i, _p );

	return i;
}
