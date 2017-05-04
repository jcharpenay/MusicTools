#pragma once

class Thread {
public:
	// Constructor
	Thread();

	// Destructor
	~Thread();

	// Operations
	void SetThreadName( const char * _name );
	void StopThread( bool _wait );
	void WaitThread();

	// Accessors
	bool ShouldStopThread() const { return m_shouldStopThread; }

protected:
	virtual void Run() = 0;
	virtual void StopThreadRequested() {}

private:
	HANDLE	m_threadHandle;
	DWORD	m_threadID;
	bool	m_shouldStopThread;

	static DWORD WINAPI RunThread( void * _param );
};
