#pragma once

class InterlockedInt {
public:
	// Constructor
	InterlockedInt();

	// Operations
	int Add( int _value );
	int Increment();
	int Decrement();

	// Accessors
	operator int() const { return m_value; }

private:
	LONG m_value;
};

class InterlockedInt64 {
public:
	// Constructor
	InterlockedInt64();

	// Operations
	int64_t Add( int64_t _value );
	int64_t Increment();
	int64_t Decrement();

	// Accessors
	operator int64_t() const { return m_value; }

private:
	LONGLONG m_value;
};