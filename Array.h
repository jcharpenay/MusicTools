#pragma once

#include "Sort.h"

template< class T > class Array {
public:
	// Constructor
	Array();
	Array( const Array< T > & _other );

	// Destructor
	~Array();

	// Operations
	void operator=( const Array< T > & _other );
	void operator+=( const Array< T > & _other );
	T & Add();
	T & Add( const T & _item );
	bool Remove( const T & _item );
	bool RemoveIndex( unsigned int _index );
	void Clear();
	void Sort( SortBase< T > & _sort = QuickSort< T >() );

	// Allocation
	void EnsureAllocated( unsigned int _size );
	void Resize( unsigned int _size );

	// Accessors
	bool IsEmpty() const								{ return m_num == 0; }
	bool IsFull() const									{ return m_num == m_size; }
	unsigned int NumItems() const						{ return m_num; }
	unsigned int MaxSize() const						{ return m_size; }
	const T & operator[]( unsigned int _index ) const	{ return m_data[ _index ]; }
	T & operator[]( unsigned int _index )				{ return m_data[ _index ]; }
	const T & First() const								{ return *m_data; }
	T & First()											{ return *m_data; }
	const T & Last() const								{ return m_data[ m_num - 1 ]; }
	T & Last()											{ return m_data[ m_num - 1 ]; }

private:
	T *				m_data;
	unsigned int	m_num;
	unsigned int	m_size;
};

#include "Array.inl"
#include "Sort.inl"
