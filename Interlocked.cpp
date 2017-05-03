#include "Precompiled.h"
#include "Interlocked.h"

InterlockedInt::InterlockedInt() :
	m_value( 0 ) {
	// Check 4-byte alignment
	DEBUG_ASSERT( ( reinterpret_cast< uintptr_t >( &m_value ) & 0x3 ) == 0 );
}

int InterlockedInt::Add( int _value ) {
	return InterlockedAdd( &m_value, _value );
}

int InterlockedInt::Increment() {
	return InterlockedIncrement( &m_value );
}

int InterlockedInt::Decrement() {
	return InterlockedDecrement( &m_value );
}

InterlockedInt64::InterlockedInt64() :
	m_value( 0 ) {
	// Check 8-byte alignment
	DEBUG_ASSERT( ( reinterpret_cast< uintptr_t >( &m_value ) & 0x7 ) == 0 );
}

int64_t InterlockedInt64::Add( int64_t _value ) {
	return InterlockedAdd64( &m_value, _value );
}

int64_t InterlockedInt64::Increment() {
	return InterlockedIncrement64( &m_value );
}

int64_t InterlockedInt64::Decrement() {
	return InterlockedDecrement64( &m_value );
}
