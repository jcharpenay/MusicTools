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
