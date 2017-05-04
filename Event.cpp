#include "Precompiled.h"
#include "Event.h"

Event::Event( ResetType _resetType, InitialState _initialState ) {
	m_handle = CreateEvent( NULL, _resetType, _initialState, NULL );
}

Event::~Event() {
	CloseHandle( m_handle );
}

void Event::Trigger() {
	SetEvent( m_handle );
}

void Event::Reset() {
	ResetEvent( m_handle );
}

void Event::Wait() {
	WaitForSingleObject( m_handle, INFINITE );
}
