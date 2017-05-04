#include "Precompiled.h"
#include "Thread.h"

Thread::Thread() : m_threadHandle( NULL ), m_threadID( 0 ), m_shouldStopThread( false ) {
	m_threadHandle = CreateThread( NULL, 0, RunThread, this, 0, &m_threadID );
}

Thread::~Thread() {
#ifdef _DEBUG
	DWORD exitCode;
	if ( GetExitCodeThread( m_threadHandle, &exitCode ) != 0 ) {
		// Forgot to call StopThread( true ) or WaitThread()?
		DEBUG_ASSERT( exitCode != STILL_ACTIVE );
	}
#endif // _DEBUG
	CloseHandle( m_threadHandle );
}

void Thread::SetThreadName( const char * _name ) {
#ifdef _DEBUG
	const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack( push, 8 )
	struct ThreadNameInfo {
		DWORD	dwType;
		LPCSTR	szName;
		DWORD	dwThreadID;
		DWORD	dwFlags;
	};
#pragma pack( pop )

	ThreadNameInfo info;
	info.dwType = 0x1000;
	info.szName = _name;
	info.dwThreadID = m_threadID;
	info.dwFlags = 0;

	__try {
		RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), reinterpret_cast< const ULONG_PTR * >( &info ) );
	} __except ( EXCEPTION_EXECUTE_HANDLER ) {
	}
#endif // _DEBUG
}

void Thread::StopThread( bool _wait ) {
	m_shouldStopThread = true;
	StopThreadRequested();

	if ( _wait ) {
		WaitThread();
	}
}

void Thread::WaitThread() {
	WaitForSingleObject( m_threadHandle, INFINITE );
}

DWORD WINAPI Thread::RunThread( void * _param ) {
	reinterpret_cast< Thread * >( _param )->Run();
	return 0;
}
