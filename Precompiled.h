#pragma once

#define USE_TAGLIB				// Taglib
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <algorithm>			// For std::swap
#include <fcntl.h>				// For _O_U16TEXT
#include <io.h>					// For _setmode
#include <memory.h>				// For memset
#include <Objbase.h>			// COM
#include <PortableDevice.h>		// WPD
#include <PortableDeviceApi.h>	// WPD
#include <propvarutil.h>		// For InitPropVariantFromString
#include <shellapi.h>			// For SHFileOperation
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include <Windows.h>

#ifdef USE_TAGLIB
	#define TAGLIB_STATIC
	#include <fileref.h>		// Taglib
	#include <tag.h>			// Taglib
#endif // USE_TAGLIB

#define BIT( n )		( 1 << ( n ) )
#define MIN( a, b )		( ( a ) < ( b ) ? ( a ) : ( b ) )
#define MAX( a, b )		( ( a ) > ( b ) ? ( a ) : ( b ) )
