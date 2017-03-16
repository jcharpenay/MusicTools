#include "Precompiled.h"
#include "Lock.h"

SpinLock::SpinLock() : m_locked( 0 ) {
}

void SpinLock::Lock() {
	while ( InterlockedCompareExchange( &m_locked, 1, 0 ) != 0 ) {
		_mm_pause();
	}
}

void SpinLock::Unlock() {
	m_locked = 0;
}
