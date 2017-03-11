#pragma once

class String {
public:
	// Constructor
	String();
	String( const wchar_t * _text );
	String( const String & _other );

	// Destructor
	~String();

	// Operations
	void operator=( wchar_t _char );
	void operator=( const wchar_t * _text );
	void operator=( const String & _other );
	void operator+=( wchar_t _char );
	void operator+=( const wchar_t * _text );
	void operator+=( const String & _other );
	void Clear();

	void Add( const wchar_t * _text, unsigned int _length );
	void AddPath( const String & _other );
	void StripPath();
	void StripFileExtension();
	const wchar_t * GetFileName() const;
	const wchar_t * GetFileExtension() const;

	// Allocation
	void EnsureAllocated( unsigned int _size );
	void Resize( unsigned int _size );

	// Length
	void SetLength( unsigned int _length );
	void RefreshLength();
	static unsigned int Length( const wchar_t * _text );

	// Comparison
	bool operator==( const wchar_t * _text ) const;
	bool operator==( const String & _other ) const;
	bool operator!=( const wchar_t * _text ) const;
	bool operator!=( const String & _other ) const;
	bool operator<( const wchar_t * _text ) const;
	bool operator<( const String & _other ) const;
	bool operator>( const wchar_t * _text ) const;
	bool operator>( const String & _other ) const;
	bool operator<=( const wchar_t * _text ) const;
	bool operator<=( const String & _other ) const;
	bool operator>=( const wchar_t * _text ) const;
	bool operator>=( const String & _other ) const;
	int Compare( const wchar_t * _text ) const;
	int Compare( const String & _other ) const;
	int Compare( const wchar_t * _text, unsigned int _length ) const;
	int Compare( const String & _other, unsigned int _length ) const;
	static int Compare( const wchar_t * _a, const wchar_t * _b );
	static int Compare( const wchar_t * _a, const wchar_t * _b, unsigned int _length );

	// Accessors
	bool IsEmpty() const								{ return m_length == 0; }
	unsigned int Length() const							{ return m_length; }
	unsigned int MaxLength() const						{ return m_size - 1; }
	unsigned int MaxSize() const						{ return m_size; }
	const wchar_t * AsChar() const						{ return m_data; }
	operator const wchar_t *() const					{ return m_data; }
	operator wchar_t *()								{ return m_data; }

private:
	// Windows max path is defined as 260 characters: <Drive>:\<256-character path><\0>
	static const int STATIC_SIZE = MAX_PATH;

	wchar_t *		m_data;
	unsigned int	m_length;
	unsigned int	m_size;
	wchar_t			m_static[ STATIC_SIZE ];
};
