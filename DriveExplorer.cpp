#include "Precompiled.h"
#include "Buffer.h"
#include "Console.h"
#include "DriveExplorer.h"
#include "File.h"

FileSystem::DriveExplorer::DriveExplorer( const Drive & _drive ) {
	m_path = _drive.m_path;
	ExplorePath( m_path, m_folderNames, m_fileNames );
}

void FileSystem::DriveExplorer::GetID( FolderID & _folderID ) const {
	_folderID.Set( m_path );
}

void FileSystem::DriveExplorer::GetFolderID( unsigned int _index, FolderID & _folderID ) const {
	if ( _index < m_folderNames.NumItems() ) {
		String path = m_path;
		path.AddPath( m_folderNames[ _index ] );
		_folderID.Set( path );
	}
}

void FileSystem::DriveExplorer::GetFileID( unsigned int _index, FileID & _fileID ) const {
	if ( _index < m_fileNames.NumItems() ) {
		String path = m_path;
		path.AddPath( m_fileNames[ _index ] );
		_fileID.Set( path );
	}
}

FileSystem::Explorer * FileSystem::DriveExplorer::ExploreFolder( const FolderID & _folderID ) const {
	if ( _folderID.IsValid() ) {
		DriveExplorer * explorer = new DriveExplorer();
		explorer->m_path = _folderID.GetPath();
		ExplorePath( explorer->m_path, explorer->m_folderNames, explorer->m_fileNames );

		return explorer;
	} else {
		return NULL;
	}
}

bool FileSystem::DriveExplorer::CreateFolder( const FolderID & _parentFolderID, const String & _folderName, FolderID & _folderID ) const {
	bool succeeded = false;
	if ( _parentFolderID.IsValid() && !_folderName.IsEmpty() ) {
		String path = _parentFolderID.GetPath();
		path.AddPath( _folderName );

		if ( CreateDirectory( path, NULL ) != 0 ) {
			_folderID.Set( path );
			succeeded = true;
		} else {
			Printf( TEXT( "CreateDirectory failed: %u\n" ), GetLastError() );
		}
	}

	return succeeded;
}

bool FileSystem::DriveExplorer::DeleteFolder( const FolderID & _folderID ) const {
	bool succeeded = false;
	if ( _folderID.IsValid() ) {
		const String & path = _folderID.GetPath();
		String doubleNullTerminatedPath;

		doubleNullTerminatedPath.EnsureAllocated( path.Length() + 2 );
		doubleNullTerminatedPath = path;
		doubleNullTerminatedPath[ path.Length() + 1 ] = TEXT( '\0' );

		SHFILEOPSTRUCT fileOp = {
			NULL,
			FO_DELETE,
			doubleNullTerminatedPath,
			TEXT( "" ),
			FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
			false,
			NULL,
			TEXT( "" )
		};

		succeeded = ( SHFileOperation( &fileOp ) == 0 );
	}

	return succeeded;
}

File * FileSystem::DriveExplorer::ReadFile( const FileID & _fileID ) const {
	File * file = NULL;
	if ( _fileID.IsValid() ) {
		HANDLE hFile = ::CreateFile( _fileID.GetPath(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE ) {
			const DWORD size = GetFileSize( hFile, NULL );
			if ( size != INVALID_FILE_SIZE ) {
				RefCountedPtr< Buffer > buffer = new Buffer( size );
				DWORD sizeRead = 0;
				if ( ::ReadFile( hFile, buffer->Data(), size, &sizeRead, NULL ) != 0 ) {
					if ( sizeRead == size ) {
						file = new File( *buffer );
					} else {
						Printf( TEXT( "ReadFile read %u byte(s) but %u was expected\n" ), sizeRead, size );
					}
				} else {
					Printf( TEXT( "ReadFile failed: %u\n" ), GetLastError() );
				}
			}
			CloseHandle( hFile );
		} else {
			Printf( TEXT( "CreateFile failed: %u\n" ), GetLastError() );
		}
	}

	return file;
}

bool FileSystem::DriveExplorer::CreateFile( const FolderID & _parentFolderID, const String & _fileName, const File & _file, FileID & _fileID ) const {
	String path = _parentFolderID.GetPath();
	path.AddPath( _fileName );

	FileID fileID;
	fileID.Set( path );

	if ( WriteFile( fileID, _file ) ) {
		_fileID = fileID;
		return true;
	} else {
		return false;
	}
}

bool FileSystem::DriveExplorer::WriteFile( const FileID & _fileID, const File & _file ) const {
	bool succeeded = false;
	if ( _fileID.IsValid() ) {
		HANDLE hFile = ::CreateFile( _fileID.GetPath(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE ) {
			const Buffer & buffer = _file.GetBuffer();
			DWORD sizeWritten = 0;
			if ( ::WriteFile( hFile, buffer.Data(), buffer.Size(), &sizeWritten, NULL ) != 0 ) {
				succeeded = true;
			} else {
				Printf( TEXT( "WriteFile failed: %u\n" ), GetLastError() );
			}
			CloseHandle( hFile );
		} else {
			Printf( TEXT( "CreateFile failed: %u\n" ), GetLastError() );
		}
	}

	return succeeded;
}

bool FileSystem::DriveExplorer::DeleteFile( const FileID & _fileID ) const {
	bool succeeded = false;
	if ( _fileID.IsValid() ) {
		if ( ::DeleteFile( _fileID.GetPath() ) != 0 ) {
			succeeded = true;
		} else {
			Printf( TEXT( "DeleteFile failed: %u\n" ), GetLastError() );
		}
	}

	return succeeded;
}

bool FileSystem::DriveExplorer::IsFileReadOnly( const FileID & _fileID ) const {
	const bool isReadOnly = ( ( GetFileAttributes( _fileID.GetPath() ) & FILE_ATTRIBUTE_READONLY ) != 0 );

	return isReadOnly;
}

bool FileSystem::DriveExplorer::SetFileReadOnly( const FileID & _fileID, bool _readOnly ) const {
	DWORD attributes = GetFileAttributes( _fileID.GetPath() );
	bool succeeded = false;

	if ( _readOnly ) {
		attributes |= FILE_ATTRIBUTE_READONLY;
	} else {
		attributes &= ~FILE_ATTRIBUTE_READONLY;
	}

	if ( SetFileAttributes( _fileID.GetPath(), attributes ) != 0 ) {
		succeeded = true;
	} else {
		Printf( TEXT( "SetFileAttributes failed: %u\n" ), GetLastError() );
	}

	return succeeded;
}

bool FileSystem::DriveExplorer::FetchAudioFileTags( const FileID & _fileID, AudioFileTags & _tags ) const {
#ifdef USE_TAGLIB
	if ( _fileID.IsValid() ) {
		TagLib::FileRef fileRef( TagLib::FileName( _fileID.GetPath() ), false, TagLib::AudioProperties::Fast );
		TagLib::Tag * fileTag = fileRef.tag();

		if ( fileTag != NULL ) {
			_tags.m_title = fileTag->title().toCWString();
			_tags.m_artist = fileTag->artist().toCWString();
			_tags.m_album = fileTag->album().toCWString();
			_tags.m_genre = fileTag->genre().toCWString();
			_tags.m_year = fileTag->year();
			_tags.m_track = fileTag->track();
			_tags.m_relevantTagsMask = AudioFileTags::ALL;

			return true;
		}
	}
#endif // USE_TAGLIB
	return false;
}

bool FileSystem::DriveExplorer::ExplorePath( const String & _path, Array< String > & _folderNames, Array< String > & _fileNames ) {
	String formattedPath = _path;
	formattedPath += TEXT( "\\*" );

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile( formattedPath, &findData );

	if ( hFind == INVALID_HANDLE_VALUE ) {
		return false;
	}

	do {
		String name = findData.cFileName;
		if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if ( name != TEXT( "." ) && name != TEXT( ".." ) ) {
				_folderNames.Add( name );
			}
		} else {
			_fileNames.Add( name );
		}
	} while ( FindNextFile( hFind, &findData ) );

	_folderNames.Sort();
	_fileNames.Sort();

	FindClose( hFind );

	return true;
}
