#include "Precompiled.h"
#include "Buffer.h"
#include "Console.h"
#include "File.h"
#include "PortableDeviceExplorer.h"

FileSystem::PortableDeviceExplorer::PortableDeviceExplorer( const PortableDevice & _device ) {
	ComPtr< IPortableDeviceManager > deviceManager;
	ComPtr< IPortableDevice > device;
	ComPtr< IPortableDeviceValues > clientInfo;
	ComPtr< IPortableDeviceContent > content;
	ComPtr< IPortableDeviceProperties > properties;
	ComPtr< IPortableDeviceKeyCollection > genericProperties;
	ComPtr< IPortableDeviceKeyCollection > audioProperties;
	ComPtr< IPortableDeviceKeyCollection > sizeProperty;

	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceManager ) ) )
		&& SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceFTM, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &device ) ) )
		&& GetClientInfo( *&clientInfo )
		&& SUCCEEDED( device->Open( _device.m_id, clientInfo ) )
		&& SUCCEEDED( device->Content( &content ) )
		&& SUCCEEDED( content->Properties( &properties ) )
		&& GetGenericProperties( *&genericProperties )
		&& GetAudioProperties( *&audioProperties )
		&& GetSizeProperty( *&sizeProperty ) ) {
		m_content = content;
		m_properties = properties;
		m_genericProperties = genericProperties;
		m_audioProperties = audioProperties;
		m_sizeProperty = sizeProperty;

		m_path = _device.m_name;

		ExplorePath( WPD_DEVICE_OBJECT_ID );
	}
}

void FileSystem::PortableDeviceExplorer::GetID( FolderID & _folderID ) const {
	_folderID.Set( m_ID, m_path );
}

void FileSystem::PortableDeviceExplorer::GetFolderID( unsigned int _index, FolderID & _folderID ) const {
	if ( _index < m_folderNames.NumItems() ) {
		String path = m_path;
		path.AddPath( m_folderNames[ _index ] );
		_folderID.Set( m_folderIDs[ _index ], path );
	}
}

void FileSystem::PortableDeviceExplorer::GetFileID( unsigned int _index, FileID & _fileID ) const {
	if ( _index < m_fileNames.NumItems() ) {
		String path = m_path;
		path.AddPath( m_fileNames[ _index ] );
		_fileID.Set( m_fileIDs[ _index ], path );
	}
}

FileSystem::Explorer * FileSystem::PortableDeviceExplorer::ExploreFolder( const FolderID & _folderID ) const {
	if ( _folderID.IsValid() ) {
		PortableDeviceExplorer * explorer = new PortableDeviceExplorer();
		explorer->m_path = _folderID.GetPath();

		explorer->m_content = m_content;
		explorer->m_properties = m_properties;
		explorer->m_genericProperties = m_genericProperties;
		explorer->m_audioProperties = m_audioProperties;
		explorer->m_sizeProperty = m_sizeProperty;

		explorer->ExplorePath( _folderID.GetID() );

		return explorer;
	} else {
		return NULL;
	}
}

bool FileSystem::PortableDeviceExplorer::CreateFolder( const FolderID & _parentFolderID, const String & _folderName, FolderID & _folderID ) const {
	bool succeeded = false;
	if ( _parentFolderID.IsValid() && !_folderName.IsEmpty() ) {
		ComPtr< IPortableDeviceValues > folderProperties;
		if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &folderProperties ) ) )
			&& SUCCEEDED( folderProperties->SetStringValue( WPD_OBJECT_PARENT_ID, _parentFolderID.GetID() ) )
			&& SUCCEEDED( folderProperties->SetStringValue( WPD_OBJECT_NAME, _folderName ) )
			&& SUCCEEDED( folderProperties->SetGuidValue( WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_FOLDER ) ) ) {
			wchar_t * folderID = NULL;
			HRESULT result = S_OK;

			if ( SUCCEEDED( result = m_content->CreateObjectWithPropertiesOnly( folderProperties, &folderID ) ) ) {
				String path = m_path;
				path.AddPath( _folderName );

				_folderID.Set( folderID, path );
				succeeded = true;
			} else {
				Printf( TEXT( "Error %x\n" ), result );
			}

			CoTaskMemFree( folderID );
		}
	}

	return succeeded;
}

bool FileSystem::PortableDeviceExplorer::DeleteFolder( const FolderID & _folderID ) const {
	bool succeeded = false;
	if ( _folderID.IsValid() ) {
		ComPtr< IPortableDevicePropVariantCollection > objectIDsToDelete;
		PROPVARIANT objectID;

		if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDevicePropVariantCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &objectIDsToDelete ) ) )
			&& SUCCEEDED( InitPropVariantFromString( _folderID.GetID(), &objectID ) )
			&& SUCCEEDED( objectIDsToDelete->Add( &objectID ) )
			&& m_content->Delete( PORTABLE_DEVICE_DELETE_WITH_RECURSION, objectIDsToDelete, NULL ) == S_OK ) {
			succeeded = true;
		}
	}

	return succeeded;
}

File * FileSystem::PortableDeviceExplorer::ReadFile( const FileID & _fileID ) const {
	File * file = NULL;
	if ( _fileID.IsValid() ) {
		ComPtr< IPortableDeviceValues > fileProperties;
		ULONGLONG fileSize = 0;

		if ( SUCCEEDED( m_properties->GetValues( _fileID.GetID(), m_sizeProperty, &fileProperties ) )
			&& SUCCEEDED( fileProperties->GetUnsignedLargeIntegerValue( WPD_OBJECT_SIZE, &fileSize ) )
			&& fileSize > 0 ) {
			ComPtr< IPortableDeviceResources > resources;
			DWORD optimalBufferSize = 0;
			ComPtr< IStream > stream;
			HRESULT result = S_OK;

			if ( SUCCEEDED( result = m_content->Transfer( &resources ) )
				&& SUCCEEDED( result = resources->GetStream( _fileID.GetID(), WPD_RESOURCE_DEFAULT, STGM_READ, &optimalBufferSize, &stream ) ) ) {
				file = ReadStream( *stream, optimalBufferSize, static_cast< DWORD >( fileSize ) );
			}

			if ( FAILED( result ) ) {
				Printf( TEXT( "Error %x\n" ), result );
			}
		}
	}

	return file;
}

bool FileSystem::PortableDeviceExplorer::CreateFile( const FolderID & _parentFolderID, const String & _fileName, const File & _file, FileID & _fileID ) const {
	bool succeeded = false;
	if ( _parentFolderID.IsValid() && !_fileName.IsEmpty() ) {
		const Buffer & buffer = _file.GetBuffer();
		const unsigned int fileSize = buffer.Size();
		ComPtr< IPortableDeviceValues > fileProperties;
		String fileNameWithoutExtension = _fileName;
		fileNameWithoutExtension.StripFileExtension();

		if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &fileProperties ) ) )
			&& SUCCEEDED( fileProperties->SetStringValue( WPD_OBJECT_PARENT_ID, _parentFolderID.GetID() ) )
			&& SUCCEEDED( fileProperties->SetStringValue( WPD_OBJECT_NAME, fileNameWithoutExtension ) )
			&& SUCCEEDED( fileProperties->SetStringValue( WPD_OBJECT_ORIGINAL_FILE_NAME, _fileName ) )
			&& SUCCEEDED( fileProperties->SetUnsignedLargeIntegerValue( WPD_OBJECT_SIZE, fileSize ) ) ) {
			const wchar_t * fileExtension = _fileName.GetFileExtension();
			if ( IsAudioFile( fileExtension ) ) {
				fileProperties->SetGuidValue( WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_AUDIO );
				fileProperties->SetGuidValue( WPD_OBJECT_FORMAT, GetAudioFileFormat( fileExtension ) );
			} else if ( IsPlaylistFile( fileExtension ) ) {
				fileProperties->SetGuidValue( WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_PLAYLIST );
				fileProperties->SetGuidValue( WPD_OBJECT_FORMAT, GetPlaylistFileFormat( fileExtension ) );
			}

			ComPtr< IStream > stream;
			DWORD optimalBufferSize = 0;
			wchar_t * fileID = NULL;
			HRESULT result = S_OK;

			if ( SUCCEEDED( result = m_content->CreateObjectWithPropertiesAndData( fileProperties, &stream, &optimalBufferSize, &fileID ) )
				&& WriteStream( *stream, optimalBufferSize, _file ) ) {
				String path = m_path;
				path.AddPath( _fileName );

				_fileID.Set( fileID, path );
				succeeded = true;
			}

			if ( FAILED( result ) ) {
				Printf( TEXT( "Error %x\n" ), result );
			}

			CoTaskMemFree( fileID );
		}
	}

	return succeeded;
}

bool FileSystem::PortableDeviceExplorer::WriteFile( const FileID & _fileID, const File & _file ) const {
	bool succeeded = false;
	if ( _fileID.IsValid() ) {
		const wchar_t * fileName = _fileID.GetPath().GetFileName();
		const Buffer & buffer = _file.GetBuffer();
		const unsigned int fileSize = buffer.Size();
		ComPtr< IPortableDeviceValues > fileProperties;
		String fileNameWithoutExtension = fileName;
		fileNameWithoutExtension.StripFileExtension();

		if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &fileProperties ) ) )
			&& SUCCEEDED( fileProperties->SetStringValue( WPD_OBJECT_NAME, fileNameWithoutExtension ) )
			&& SUCCEEDED( fileProperties->SetStringValue( WPD_OBJECT_ORIGINAL_FILE_NAME, fileName ) )
			&& SUCCEEDED( fileProperties->SetUnsignedLargeIntegerValue( WPD_OBJECT_SIZE, fileSize ) ) ) {
			ComPtr< IPortableDeviceContent2 > content2;
			ComPtr< IStream > stream;
			DWORD optimalBufferSize = 0;
			HRESULT result = S_OK;

			if ( SUCCEEDED( result = m_content.As( content2 ) )
				&& SUCCEEDED( result = content2->UpdateObjectWithPropertiesAndData( _fileID.GetID(), fileProperties, &stream, &optimalBufferSize ) )
				&& WriteStream( *stream, optimalBufferSize, _file ) ) {
				succeeded = true;
			}

			if ( FAILED( result ) ) {
				Printf( TEXT( "Error %x\n" ), result );
			}
		}
	}

	return succeeded;
}

bool FileSystem::PortableDeviceExplorer::DeleteFile( const FileID & _fileID ) const {
	bool succeeded = false;
	if ( _fileID.IsValid() ) {
		ComPtr< IPortableDevicePropVariantCollection > objectIDsToDelete;
		PROPVARIANT objectID;

		if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDevicePropVariantCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &objectIDsToDelete ) ) )
			&& SUCCEEDED( InitPropVariantFromString( _fileID.GetID(), &objectID ) )
			&& SUCCEEDED( objectIDsToDelete->Add( &objectID ) )
			&& m_content->Delete( PORTABLE_DEVICE_DELETE_NO_RECURSION, objectIDsToDelete, NULL ) == S_OK ) {
			succeeded = true;
		}
	}

	return succeeded;
}

bool FileSystem::PortableDeviceExplorer::FetchAudioFileTags( const FileID & _fileID, AudioFileTags & _tags ) const {
	if ( _fileID.IsValid() ) {
		ComPtr< IPortableDeviceValues > fileProperties;
		if ( SUCCEEDED( m_properties->GetValues( _fileID.GetID(), m_audioProperties, &fileProperties ) ) ) {
			wchar_t * title = NULL;
			wchar_t * artist = NULL;
			wchar_t * album = NULL;
			wchar_t * genre = NULL;
			PROPVARIANT date;
			ULONG track;

			_tags.m_relevantTagsMask = 0;

			if ( SUCCEEDED( fileProperties->GetStringValue( WPD_MEDIA_TITLE, &title ) )
				|| SUCCEEDED( fileProperties->GetStringValue( WPD_OBJECT_NAME, &title ) ) ) {
				_tags.m_title = title;
				_tags.m_relevantTagsMask |= AudioFileTags::TITLE;
			}

			if ( SUCCEEDED( fileProperties->GetStringValue( WPD_MEDIA_ARTIST, &artist ) ) ) {
				_tags.m_artist = artist;
				_tags.m_relevantTagsMask |= AudioFileTags::ARTIST;
			}

			if ( SUCCEEDED( fileProperties->GetStringValue( WPD_MUSIC_ALBUM, &album ) ) ) {
				_tags.m_album = album;
				_tags.m_relevantTagsMask |= AudioFileTags::ALBUM;
			}

			if ( SUCCEEDED( fileProperties->GetStringValue( WPD_MEDIA_GENRE, &genre ) ) ) {
				_tags.m_genre = genre;
				_tags.m_relevantTagsMask |= AudioFileTags::GENRE;
			}

			if ( SUCCEEDED( fileProperties->GetValue( WPD_MEDIA_RELEASE_DATE, &date ) ) ) {
				if ( date.vt == VT_DATE ) {
					SYSTEMTIME time;
					if ( VariantTimeToSystemTime( date.date, &time ) ) {
						_tags.m_year = time.wYear;
						_tags.m_relevantTagsMask |= AudioFileTags::YEAR;
					}
				}
			}

			if ( SUCCEEDED( fileProperties->GetUnsignedIntegerValue( WPD_MUSIC_TRACK, &track ) ) ) {
				_tags.m_track = track;
				_tags.m_relevantTagsMask |= AudioFileTags::TRACK;
			}

			// If no WPD_MEDIA/WPD_MUSIC property could be read the title (WPD_OBJECT_NAME) is most likely wrong
			if ( _tags.m_relevantTagsMask == AudioFileTags::TITLE ) {
				_tags.m_relevantTagsMask = 0;
			}

			CoTaskMemFree( title );
			CoTaskMemFree( artist );
			CoTaskMemFree( album );
			CoTaskMemFree( genre );
			PropVariantClear( &date );

			return true;
		}
	}

	return false;
}

void FileSystem::PortableDeviceExplorer::ExplorePath( const wchar_t * _objectID ) {
	m_ID = _objectID;

	ComPtr< IEnumPortableDeviceObjectIDs > objectIDs;
	if ( SUCCEEDED( m_content->EnumObjects( 0, _objectID, NULL, &objectIDs ) ) ) {
		static const int OBJECT_REQUEST_COUNT = 100;
		wchar_t * objectIDArray[ OBJECT_REQUEST_COUNT ] = { 0 };
		DWORD fetchedCount;

		while ( SUCCEEDED( objectIDs->Next( OBJECT_REQUEST_COUNT, objectIDArray, &fetchedCount ) )
			&& fetchedCount > 0 ) {
			for ( unsigned int objectIndex = 0; objectIndex < fetchedCount; objectIndex++ ) {
				wchar_t * objectID = objectIDArray[ objectIndex ];
				if ( objectID != NULL ) {
					ComPtr< IPortableDeviceValues > objectProperties;
					wchar_t * name = NULL;
					GUID contentType = GUID_NULL;

					if ( SUCCEEDED( m_properties->GetValues( objectID, m_genericProperties, &objectProperties ) )
						&& SUCCEEDED( objectProperties->GetGuidValue( WPD_OBJECT_CONTENT_TYPE, &contentType ) )
						&& ( SUCCEEDED( objectProperties->GetStringValue( WPD_OBJECT_ORIGINAL_FILE_NAME, &name ) )
							|| SUCCEEDED( objectProperties->GetStringValue( WPD_OBJECT_NAME, &name ) ) ) ) {
						if ( contentType == WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT || contentType == WPD_CONTENT_TYPE_FOLDER ) {
							m_hasSpecialFolder |= ( contentType == WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT );
							m_folderNames.Add( name );
							m_folderIDs.Add( objectID );
						} else {
							m_fileNames.Add( name );
							m_fileIDs.Add( objectID );
						}
					}

					CoTaskMemFree( objectID );
					CoTaskMemFree( name );
					objectIDArray[ objectIndex ] = NULL;
				}
			}
		}

		m_folderNames.Sort( QuickSortWithCallback< String >( this, FolderSwapCallback ) );
		m_fileNames.Sort( QuickSortWithCallback< String >( this, FileSwapCallback ) );
	}
}

bool FileSystem::PortableDeviceExplorer::GetClientInfo( IPortableDeviceValues *& _clientInfo ) {
	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &_clientInfo ) ) ) ) {
		_clientInfo->SetStringValue( WPD_CLIENT_NAME, TEXT( "MusicTools" ) );
		_clientInfo->SetUnsignedIntegerValue( WPD_CLIENT_MAJOR_VERSION, 1 );
		_clientInfo->SetUnsignedIntegerValue( WPD_CLIENT_MINOR_VERSION, 0 );
		_clientInfo->SetUnsignedIntegerValue( WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION );

		return true;
	} else {
		return false;
	}
}

bool FileSystem::PortableDeviceExplorer::GetGenericProperties( IPortableDeviceKeyCollection *& _properties ) {
	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceKeyCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &_properties ) ) ) ) {
		_properties->Add( WPD_OBJECT_CONTENT_TYPE );
		_properties->Add( WPD_OBJECT_ORIGINAL_FILE_NAME );
		_properties->Add( WPD_OBJECT_NAME );

		return true;
	} else {
		return false;
	}
}

bool FileSystem::PortableDeviceExplorer::GetAudioProperties( IPortableDeviceKeyCollection *& _properties ) {
	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceKeyCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &_properties ) ) ) ) {
		_properties->Add( WPD_MEDIA_TITLE );
		_properties->Add( WPD_OBJECT_NAME );
		_properties->Add( WPD_MEDIA_ARTIST );
		_properties->Add( WPD_MUSIC_ALBUM );
		_properties->Add( WPD_MEDIA_GENRE );
		_properties->Add( WPD_MEDIA_RELEASE_DATE );
		_properties->Add( WPD_MUSIC_TRACK );

		return true;
	} else {
		return false;
	}
}

bool FileSystem::PortableDeviceExplorer::GetSizeProperty( IPortableDeviceKeyCollection *& _properties ) {
	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceKeyCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &_properties ) ) ) ) {
		_properties->Add( WPD_OBJECT_SIZE );

		return true;
	} else {
		return false;
	}
}

void FileSystem::PortableDeviceExplorer::FolderSwapCallback( void * _ptr, unsigned int _firstIndex, unsigned int _secondIndex ) {
	PortableDeviceExplorer * explorer = static_cast< PortableDeviceExplorer * >( _ptr );
	Array< String > & folderIDs = explorer->m_folderIDs;
	std::swap( folderIDs[ _firstIndex ], folderIDs[ _secondIndex ] );
}

void FileSystem::PortableDeviceExplorer::FileSwapCallback( void * _ptr, unsigned int _firstIndex, unsigned int _secondIndex ) {
	PortableDeviceExplorer * explorer = static_cast< PortableDeviceExplorer * >( _ptr );
	Array< String > & fileIDs = explorer->m_fileIDs;
	std::swap( fileIDs[ _firstIndex ], fileIDs[ _secondIndex ] );
}

File * FileSystem::PortableDeviceExplorer::ReadStream( IStream & _stream, DWORD _optimalBufferSize, DWORD _fileSize ) {
	RefCountedPtr< Buffer > buffer = new Buffer( _fileSize );
	ULONG sizeRead = 0;
	ULONG totalSizeRead = 0;

	if ( _optimalBufferSize == 0 ) {
		_optimalBufferSize = _fileSize;
	}

	do {
		if ( SUCCEEDED( _stream.Read( buffer->Data() + totalSizeRead, MIN( _optimalBufferSize, _fileSize - totalSizeRead ), &sizeRead ) ) ) {
			totalSizeRead += sizeRead;
		} else {
			break;
		}
	} while ( sizeRead > 0 && totalSizeRead < _fileSize );

	if ( totalSizeRead == _fileSize ) {
		return new File( *buffer );
	} else {
		Printf( TEXT( "Read %u byte(s) but %u was expected\n" ), totalSizeRead, _fileSize );

		return NULL;
	}
}

bool FileSystem::PortableDeviceExplorer::WriteStream( IStream & _stream, DWORD _optimalBufferSize, const File & _file ) {
	const Buffer & buffer = _file.GetBuffer();
	const unsigned int fileSize = buffer.Size();
	ULONG sizeWritten = 0;
	ULONG totalSizeWritten = 0;
	bool succeeded = false;

	if ( _optimalBufferSize == 0 ) {
		_optimalBufferSize = fileSize;
	}

	do {
		if ( SUCCEEDED( _stream.Write( buffer.Data() + totalSizeWritten, MIN( _optimalBufferSize, fileSize - totalSizeWritten ), &sizeWritten ) ) ) {
			totalSizeWritten += sizeWritten;
		} else {
			break;
		}
	} while ( totalSizeWritten < fileSize );

	if ( totalSizeWritten == fileSize ) {
		succeeded = SUCCEEDED( _stream.Commit( STGC_DEFAULT ) );
	}

	return succeeded;
}
