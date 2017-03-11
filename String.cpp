#include "Precompiled.h"
#include "String.h"

String::String() : m_data( m_static ), m_length( 0 ), m_size( STATIC_SIZE ) {
	*m_static = TEXT( '\0' );
}

String::String( const wchar_t * _text ) : m_data( m_static ), m_length( 0 ), m_size( STATIC_SIZE ) {
	*m_static = TEXT( '\0' );
	operator+=( _text );
}

String::String( const String & _other ) : m_data( m_static ), m_length( 0 ), m_size( STATIC_SIZE ) {
	*m_static = TEXT( '\0' );
	operator+=( _other );
}

String::~String() {
	if ( m_data != m_static ) {
		delete m_data;
	}
}

void String::operator=( wchar_t _char ) {
	Clear();
	operator+=( _char );
}

void String::operator=( const wchar_t * _text ) {
	Clear();
	operator+=( _text );
}

void String::operator=( const String & _other ) {
	Clear();
	operator+=( _other );
}

void String::operator+=( wchar_t _char ) {
	const unsigned int newLength = m_length + 1;
	EnsureAllocated( newLength + 1 );
	m_data[ m_length ] = _char;
	m_data[ newLength ] = TEXT( '\0' );
	m_length = newLength;
}

void String::operator+=( const wchar_t * _text ) {
	const unsigned int additionalLength = Length( _text );
	if ( additionalLength > 0 ) {
		const unsigned int newLength = m_length + additionalLength;
		EnsureAllocated( newLength + 1 );
		memcpy( m_data + m_length, _text, additionalLength * sizeof( wchar_t ) );
		m_data[ newLength ] = TEXT( '\0' );
		m_length = newLength;
	}
}

void String::operator+=( const String & _other ) {
	if ( _other.m_length > 0 ) {
		const unsigned int newLength = m_length + _other.m_length;
		EnsureAllocated( newLength + 1 );
		memcpy( m_data + m_length, _other.m_data, _other.m_length * sizeof( wchar_t ) );
		m_data[ newLength ] = TEXT( '\0' );
		m_length = newLength;
	}
}

void String::Clear() {
	*m_data = TEXT( '\0' );
	m_length = 0;
}

void String::Add( const wchar_t * _text, unsigned int _length ) {
	const unsigned int newLength = m_length + _length;
	EnsureAllocated( newLength + 1 );
	unsigned int textIndex = 0;
	unsigned int dataIndex = m_length;
	while ( textIndex < _length && _text[ textIndex ] != TEXT( '\0' ) ) {
		m_data[ dataIndex++ ] = _text[ textIndex++ ];
	}
	m_data[ dataIndex ] = TEXT( '\0' );
	m_length = dataIndex;
}

void String::AddPath( const String & _other ) {
	if ( _other.m_length > 0 ) {
		if ( m_length == 0 ) {
			operator+=( _other );
		} else {
			const unsigned int newLength = m_length + 1 + _other.m_length;
			EnsureAllocated( newLength + 1 );
			m_data[ m_length ] = TEXT( '\\' );
			memcpy( m_data + m_length + 1, _other.m_data, _other.m_length * sizeof( wchar_t ) );
			m_data[ newLength ] = TEXT( '\0' );
			m_length = newLength;
		}
	}
}

void String::StripPath() {
	const wchar_t * fileName = GetFileName();
	if ( fileName != m_data ) {
		const unsigned int fileNameIndex = static_cast< unsigned int >( fileName - m_data );
		for ( unsigned int index = 0; index < fileNameIndex; index++ ) {
			m_data[ index ] = m_data[ fileNameIndex + index ];
		}
		m_length -= fileNameIndex;
	}
}

void String::StripFileExtension() {
	const wchar_t * fileExtension = GetFileExtension();
	if ( fileExtension != TEXT( "" ) ) {
		const unsigned int dotIndex = static_cast< unsigned int >( fileExtension - m_data ) - 1;
		m_data[ dotIndex ] = '\0';
		m_length = dotIndex;
	}
}

const wchar_t * String::GetFileName() const {
	for ( int index = m_length - 1; index >= 0; index-- ) {
		const wchar_t character = m_data[ index ];
		if ( character == TEXT( '\\' ) ) {
			return m_data + index + 1;
		}
	}

	return m_data;
}

const wchar_t * String::GetFileExtension() const {
	for ( int index = m_length - 1; index >= 0; index-- ) {
		const wchar_t character = m_data[ index ];
		if ( character == TEXT( '.' ) ) {
			return m_data + index + 1;
		} else if ( character == TEXT( '\\' ) ) {
			break;
		}
	}

	return TEXT( "" );
}

void String::EnsureAllocated( unsigned int _size ) {
	if ( _size > m_size ) {
		unsigned int newSize = ( m_size << 1 );
		while ( newSize < _size ) {
			newSize <<= 1;
		}
		Resize( newSize );
	}
}

void String::Resize( unsigned int _size ) {
#ifdef _DEBUG
	if ( _size > 1000000 ) {
		DebugBreak();
	}
#endif // DEBUG
	if ( _size != m_size ) {
		wchar_t * newData;
		if ( _size > STATIC_SIZE ) {
			newData = new wchar_t[ _size ];
		} else {
			newData = m_static;
			_size = STATIC_SIZE;
		}
		if ( newData != m_data ) {
			if ( _size > m_length ) {
				memcpy( newData, m_data, ( m_length + 1 ) * sizeof( wchar_t ) );
			} else {
				m_length = _size - 1;
				memcpy( newData, m_data, m_length * sizeof( wchar_t ) );
				newData[ m_length ] = TEXT( '\0' );
			}
			if ( m_data != m_static ) {
				delete[] m_data;
			}
		}
		m_data = newData;
		m_size = _size;
	}
}

void String::SetLength( unsigned int _length ) {
	m_length = _length;
}

void String::RefreshLength() {
	m_length = Length( m_data );
}

unsigned int String::Length( const wchar_t * _text ) {
	int index = 0;
	while ( _text[ index ] != TEXT( '\0' ) ) {
		index++;
	}
	return index;
}

bool String::operator==( const wchar_t * _text ) const {
	return Compare( _text ) == 0;
}

bool String::operator==( const String & _other ) const {
	if ( m_length == _other.m_length ) {
		return Compare( _other.m_data ) == 0;
	} else {
		return false;
	}
}

bool String::operator!=( const wchar_t * _text ) const {
	return Compare( _text ) != 0;
}

bool String::operator!=( const String & _other ) const {
	if ( m_length == _other.m_length ) {
		return Compare( _other.m_data ) != 0;
	} else {
		return true;
	}
}

bool String::operator<( const wchar_t * _text ) const {
	return Compare( _text ) < 0;
}

bool String::operator<( const String & _other ) const {
	return Compare( _other ) < 0;
}

bool String::operator>( const wchar_t * _text ) const {
	return Compare( _text ) > 0;
}

bool String::operator>( const String & _other ) const {
	return Compare( _other ) > 0;
}

bool String::operator<=( const wchar_t * _text ) const {
	return Compare( _text ) <= 0;
}

bool String::operator<=( const String & _other ) const {
	return Compare( _other ) <= 0;
}

bool String::operator>=( const wchar_t * _text ) const {
	return Compare( _text ) >= 0;
}

bool String::operator>=( const String & _other ) const {
	return Compare( _other ) >= 0;
}

int String::Compare( const wchar_t * _text ) const {
	const int result = CompareStringEx( LOCALE_NAME_USER_DEFAULT, 0, m_data, m_length, _text, -1, NULL, NULL, 0 );
	if ( result != 0 ) {
		return result - 2;
	} else {
		return wcscmp( m_data, _text );
	}
}

int String::Compare( const String & _other ) const {
	const int result = CompareStringEx( LOCALE_NAME_USER_DEFAULT, 0, m_data, m_length, _other.m_data, _other.m_length, NULL, NULL, 0 );
	if ( result != 0 ) {
		return result - 2;
	} else {
		return wcscmp( m_data, _other.m_data );
	}
}

int String::Compare( const wchar_t * _text, unsigned int _length ) const {
	const unsigned int myLength = MIN( m_length, _length );
	const unsigned int textLength = MIN( Length( _text ), _length );
	const int result = CompareStringEx( LOCALE_NAME_USER_DEFAULT, 0, m_data, myLength, _text, textLength, NULL, NULL, 0 );
	if ( result != 0 ) {
		return result - 2;
	} else {
		return wcsncmp( m_data, _text, _length );
	}
}

int String::Compare( const String & _other, unsigned int _length ) const {
	const unsigned int myLength = MIN( m_length, _length );
	const unsigned int otherLength = MIN( _other.m_length, _length );
	const int result = CompareStringEx( LOCALE_NAME_USER_DEFAULT, 0, m_data, myLength, _other.m_data, otherLength, NULL, NULL, 0 );
	if ( result != 0 ) {
		return result - 2;
	} else {
		return wcsncmp( m_data, _other.m_data, _length );
	}
}

int String::Compare( const wchar_t * _a, const wchar_t * _b ) {
	const int result = CompareStringEx( LOCALE_NAME_USER_DEFAULT, 0, _a, -1, _b, -1, NULL, NULL, 0 );
	if ( result != 0 ) {
		return result - 2;
	} else {
		return wcscmp( _a, _b );
	}
}

int String::Compare( const wchar_t * _a, const wchar_t * _b, unsigned int _length ) {
	const unsigned int aLength = MIN( Length( _a ), _length );
	const unsigned int bLength = MIN( Length( _b ), _length );
	const int result = CompareStringEx( LOCALE_NAME_USER_DEFAULT, 0, _a, aLength, _b, bLength, NULL, NULL, 0 );
	if ( result != 0 ) {
		return result - 2;
	} else {
		return wcsncmp( _a, _b, _length );
	}
}
