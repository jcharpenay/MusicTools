#pragma once

class String;

namespace Console {
	void Init( const wchar_t * _title );
	void WriteLine( COORD _position, const wchar_t * _format, ... );

	bool AskBool( const wchar_t * _format, ... );
	void AskString( const wchar_t * _format, String & _answer, ... );

	void SetCursorPosition( COORD _position );
	COORD GetCursorPosition();
}

void Printf( const wchar_t * _format, ... );
void Gets( String & _text );
