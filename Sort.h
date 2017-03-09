#pragma once

template< class T > class Array;

template< class T > class SortBase {
public:
	virtual void Sort( Array< T > & _array ) = 0;
};

template< class T > class QuickSort : public SortBase< T > {
public:
	virtual void Sort( Array< T > & _array ) override;

private:
	void Sort( Array< T > & _array, unsigned int _p, unsigned int _q );
	unsigned int Partition( Array< T > & _array, unsigned int _p, unsigned int _q );
};

template< class T > class QuickSortWithCallback : public SortBase< T > {
public:
	typedef void ( * SwapCallback )( void * _ptr, unsigned int _firstIndex, unsigned int _secondIndex );

	QuickSortWithCallback( void * _ptr, SwapCallback _swapCallback ) : m_ptr( _ptr ), m_swapCallback( _swapCallback ) {}
	virtual void Sort( Array< T > & _array ) override;

private:
	void Sort( Array< T > & _array, unsigned int _p, unsigned int _q );
	unsigned int Partition( Array< T > & _array, unsigned int _p, unsigned int _q );

	void *			m_ptr;
	SwapCallback	m_swapCallback;
};
