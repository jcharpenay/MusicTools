#pragma once

#include "RefCounted.h"

class Buffer : public RefCounted {
public:
	// Constructor
	Buffer();
	Buffer( unsigned int _size );

	// Destructor
	virtual ~Buffer() override;

	// Allocation
	void EnsureAllocated( unsigned int _size );
	void Resize( unsigned int _size );

	// Accessors
	bool IsEmpty() const				{ return m_size == 0; }
	unsigned int Size() const			{ return m_size; }
	const unsigned char * Data() const	{ return m_data; }
	unsigned char * Data()				{ return m_data; }

private:
	unsigned char *		m_data;
	unsigned int		m_size;
};
