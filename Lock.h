#pragma once

class SpinLock {
public:
	// Constructor
	SpinLock();

	// Operations
	void Lock();
	void Unlock();

private:
	LONG m_locked;
};

template< class T > class ScopedLock {
public:
	// Constructor
	ScopedLock( T & _lock ) : m_lock( &_lock ) { _lock.Lock(); }

	// Destructor
	~ScopedLock() { m_lock->Unlock(); }

private:
	T * m_lock;
};
