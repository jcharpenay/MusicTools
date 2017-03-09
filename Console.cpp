#include "Precompiled.h"
#include "Console.h"
#include "String.h"

void Console::Init( const wchar_t * _title ) {
	SetConsoleTitle( _title );

	// Setup the console to output UTF16 text
	_setmode( _fileno( stdout ), _O_U16TEXT );
}

void Console::WriteLine( COORD _position, const wchar_t * _format, ... ) {
	const HANDLE stdOutHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;

	if ( GetConsoleScreenBufferInfo ( stdOutHandle, &screenBufferInfo ) ) {
		const int maxLength = screenBufferInfo.dwSize.X - _position.X;
		if ( maxLength > 0 ) {
			wchar_t * buffer = static_cast< wchar_t * >( alloca( ( maxLength + 1 ) * sizeof( wchar_t ) ) );

			COORD cursorPosition = screenBufferInfo.dwCursorPosition;
			SetConsoleCursorPosition( stdOutHandle, _position );

			va_list argPtr;
			va_start( argPtr, _format );
			const int length = _vsnwprintf_s( buffer, maxLength + 1, _TRUNCATE, _format, argPtr );
			va_end( argPtr );

			for ( int i = MAX( 0, length ); i < maxLength; i++ ) {
				buffer[ i ] = TEXT( ' ' );
			}

			buffer[ maxLength ] = '\0';

			DWORD numCharsWritten;
			WriteConsole( stdOutHandle, buffer, maxLength, &numCharsWritten, NULL );

			SetConsoleCursorPosition( stdOutHandle, cursorPosition );
		}
	}
}

bool Console::AskBool( const wchar_t * _format, ... ) {
	va_list argPtr;
    va_start( argPtr, _format );
    vfwprintf( stdout, _format, argPtr );
    va_end( argPtr );

	String answer;
	Gets( answer );
	
	return answer[ 0 ] == TEXT( 'y' ) || answer[ 0 ] == TEXT( 'Y' );
}

void Console::AskString( const wchar_t * _format, String & _answer, ... ) {
	va_list argPtr;
    va_start( argPtr, _format );
    vfwprintf( stdout, _format, argPtr );
    va_end( argPtr );

	Gets( _answer );
}

void Console::SetCursorPosition( COORD _position ) {
	const HANDLE stdOutHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleCursorPosition( stdOutHandle, _position );
}

COORD Console::GetCursorPosition() {
	const HANDLE stdOutHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
	COORD cursorPosition = { 0 };

	if ( GetConsoleScreenBufferInfo ( stdOutHandle, &screenBufferInfo ) ) {
		cursorPosition = screenBufferInfo.dwCursorPosition;
	}

	return cursorPosition;
}

void Printf( const wchar_t * _format, ... ) {
	va_list argPtr;
    va_start( argPtr, _format );
    vfwprintf( stdout, _format, argPtr );
    va_end( argPtr );
}

void Gets( String & _text ) {
	_getws_s( _text, _text.MaxLength() );
	_text.RefreshLength();
}
