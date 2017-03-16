#pragma once

class Thread {
public:
	// Constructor
	Thread();

	// Destructor
	~Thread();

	// Operations
	void SetThreadName( const char * _name );
	void Wait();

protected:
	virtual void Run() = 0;

private:
	HANDLE	m_threadHandle;
	DWORD	m_threadID;

	static DWORD WINAPI RunThread( void * _param );
};
