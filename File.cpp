#include "Precompiled.h"
#include "Console.h"
#include "File.h"
#include "String.h"

File::File() : m_buffer( new Buffer() ) {
}

File::File( Buffer & _buffer ) : m_buffer( &_buffer ) {
}

TextFile::TextFile() : m_encoding( ANSI ) {
	m_str = NULL;
}

TextFile::TextFile( File & _file ) : File( _file.GetBuffer() ), m_encoding( ANSI ) {
	m_buffer->AddRef();

	// Read BOM
	unsigned char * data = m_buffer->Data();
	m_str = reinterpret_cast< char * >( data );

	const unsigned int size = m_buffer->Size();
	if ( size >= 3 && data[ 0 ] == 0xEF && data[ 1 ] == 0xBB && data[ 2 ] == 0xBF ) {
		m_encoding = UTF_8;
		m_str += 3;
	} else if ( size >= 2 ) {
		if ( data[ 0 ] == 0xFE && data[ 1 ] == 0xFF ) {
			m_encoding = UTF_16_BE;
			m_wstr = reinterpret_cast< wchar_t * >( data + 2 );
		} else if ( data[ 0 ] == 0xFF && data[ 1 ] == 0xFE ) {
			m_encoding = UTF_16_LE;
			m_wstr = reinterpret_cast< wchar_t * >( data + 2 );
		}
	}
}

void TextFile::Set( Encoding _encoding, const String & _string ) {
	m_encoding = _encoding;
	m_str = NULL;

	// Write BOM
	switch ( _encoding ) {
		case ANSI:
			break;
		case UTF_8: {
			m_buffer->EnsureAllocated( 3 );
			unsigned char * data = m_buffer->Data();
			data[ 0 ] = 0xEF;
			data[ 1 ] = 0xBB;
			data[ 2 ] = 0xBF;
			break;
		}
		case UTF_16_BE:
		case UTF_16_LE: {
			m_buffer->EnsureAllocated( 2 );
			unsigned char * data = m_buffer->Data();
			if ( _encoding == UTF_16_BE ) {
				data[ 0 ] = 0xFE;
				data[ 1 ] = 0xFF;
			} else {
				data[ 0 ] = 0xFF;
				data[ 1 ] = 0xFE;
			}
			break;
		}
		default:
			break;
	}

	// Write string
	const unsigned int length = _string.Length();
	switch ( _encoding ) {
		case ANSI: {
			m_buffer->Resize( length );
			char * str = reinterpret_cast< char * >( m_buffer->Data() );
			const int returnedLength = WideCharToMultiByte( CP_ACP, 0, _string, length, str, length, NULL, NULL );
			if ( returnedLength > 0 ) {
				m_str = str;
			} else {
				Printf( TEXT( "WideCharToMultiByte failed: %u\n" ), GetLastError() );
			}
		}
		case UTF_8: {
			int returnedLength = WideCharToMultiByte( CP_UTF8, 0, _string, length, NULL, 0, NULL, NULL );
			if ( returnedLength > 0 ) {
				const unsigned int size = 3 + returnedLength;
				m_buffer->Resize( size );
				char * str = reinterpret_cast< char * >( m_buffer->Data() + 3 );
				returnedLength = WideCharToMultiByte( CP_UTF8, 0, _string, length, str, returnedLength, NULL, NULL );
				if ( returnedLength > 0 ) {
					m_str = str;
				}
			}
			if ( returnedLength <= 0 ) {
				Printf( TEXT( "WideCharToMultiByte failed: %u\n" ), GetLastError() );
			}
			break;
		}
		case UTF_16_BE:
		case UTF_16_LE: {
			const unsigned int size = 2 + length * sizeof( wchar_t );
			m_buffer->Resize( size );
			wchar_t * wstr = reinterpret_cast< wchar_t * >( m_buffer->Data() + 2 );
			memcpy( wstr, _string, length * sizeof( wchar_t ) );
			if ( m_encoding == UTF_16_BE ) {
				for ( unsigned i = 0; i < length; i++ ) {
					wstr[ i ] = _byteswap_ushort( wstr[ i ] );
				}
			}
			m_wstr = wstr;
			break;
		}
		default:
			break;
	}
}

bool TextFile::ToString( String & _string ) const {
	_string.Clear();

	switch ( m_encoding ) {
		case ANSI:
		case UTF_8: {
			const unsigned int length = m_buffer->Size() - 3;
			if ( length > 0 ) {
				_string.EnsureAllocated( length + 1 );
				const int returnedLength = MultiByteToWideChar( m_encoding == ANSI ? CP_ACP : CP_UTF8, 0, m_str, length, _string, length );
				if ( returnedLength > 0 ) {
					_string[ returnedLength ] = TEXT( '\0' );
					_string.SetLength( returnedLength );
				} else {
					Printf( TEXT( "MultiByteToWideChar failed: %u\n" ), GetLastError() );
				}
			}
			break;
		}
		case UTF_16_BE:
		case UTF_16_LE: {
			const unsigned int length = ( m_buffer->Size() - 2 ) / sizeof( wchar_t );
			if ( length > 0 ) {
				_string.EnsureAllocated( length + 1 );
				memcpy( _string, m_wstr, length * sizeof( wchar_t ) );
				if ( m_encoding == UTF_16_BE ) {
					for ( unsigned i = 0; i < length; i++ ) {
						_string[ i ] = _byteswap_ushort( _string[ i ] );
					}
				}
				_string[ length ] = TEXT( '\0' );
				_string.SetLength( length );
			}
			break;
		}
		default:
			break;
	}

	return false;
}
