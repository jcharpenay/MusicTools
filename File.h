#pragma once

#include "Buffer.h"
#include "RefCounted.h"

class String;

class File : public RefCounted {
public:
	File();
	File( Buffer & _buffer );

	const Buffer & GetBuffer() const { return *m_buffer; }
	Buffer & GetBuffer() { return *m_buffer; }

protected:
	RefCountedPtr< Buffer > m_buffer;
};

class TextFile : public File {
public:
	enum Encoding {
		ANSI = 0,
		UTF_8,
		UTF_16_BE,
		UTF_16_LE
	};

	TextFile();
	TextFile( File & _file );

	void Set( Encoding _encoding, const String & _string );
	bool ToString( String & _string ) const;

private:
	Encoding		m_encoding;
	union {
		char *		m_str;
		wchar_t *	m_wstr;
	};
};
