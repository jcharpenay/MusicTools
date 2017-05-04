#pragma once

class Event {
public:
	enum ResetType {
		AUTO_RESET = FALSE,
		MANUAL_RESET = TRUE
	};

	enum InitialState {
		NOT_SET = FALSE,
		SET = TRUE
	};

	// Constructor
	Event( ResetType _resetType, InitialState _initialState );

	// Destructor
	~Event();

	// Operations
	void Trigger();
	void Reset();
	void Wait();

private:
	HANDLE m_handle;
};
