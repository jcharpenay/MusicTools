#include "Precompiled.h"
#include "PortableDeviceExplorer.h"

FileSystem::PortableDeviceExplorer::PortableDeviceExplorer( const PortableDevice & _device ) {
	ComPtr< IPortableDeviceManager > deviceManager;
	ComPtr< IPortableDevice > device;
	ComPtr< IPortableDeviceValues > clientInfo;
	ComPtr< IPortableDeviceContent > content;
	ComPtr< IPortableDeviceProperties > properties;
	ComPtr< IPortableDeviceKeyCollection > genericPropertiesToRead;

	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceManager ) ) )
		&& SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceFTM, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &device ) ) )
		&& GetClientInfo( *&clientInfo )
		&& SUCCEEDED( device->Open( _device.m_id, clientInfo ) )
		&& SUCCEEDED( device->Content( &content ) )
		&& SUCCEEDED( content->Properties( &properties ) )
		&& GetGenericPropertiesToRead( *&genericPropertiesToRead ) ) {
		m_content = content;
		m_properties = properties;
		m_genericPropertiesToRead = genericPropertiesToRead;

		m_path = _device.m_name;

		ExplorePath( WPD_DEVICE_OBJECT_ID );
	}
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
		explorer->m_genericPropertiesToRead = m_genericPropertiesToRead;
		explorer->m_audioPropertiesToRead = m_audioPropertiesToRead;

		explorer->ExplorePath( _folderID.GetID() );

		return explorer;
	} else {
		return NULL;
	}
}

bool FileSystem::PortableDeviceExplorer::FetchAudioFileTags( const FileID & _fileID, AudioFileTags & _tags ) const {
	if ( _fileID.IsValid() ) {
		if ( m_audioPropertiesToRead != NULL || GetAudioPropertiesToRead( *&m_audioPropertiesToRead ) ) {
			ComPtr< IPortableDeviceValues > fileProperties;
			if ( SUCCEEDED( m_properties->GetValues( _fileID.GetID(), m_audioPropertiesToRead, &fileProperties ) ) ) {
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
	}

	return false;
}

void FileSystem::PortableDeviceExplorer::ExplorePath( const wchar_t * _objectID ) {
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

					if ( SUCCEEDED( m_properties->GetValues( objectID, m_genericPropertiesToRead, &objectProperties ) )
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

bool FileSystem::PortableDeviceExplorer::GetGenericPropertiesToRead( IPortableDeviceKeyCollection *& _propertiesToRead ) {
	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceKeyCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &_propertiesToRead ) ) ) ) {
		_propertiesToRead->Add( WPD_OBJECT_CONTENT_TYPE );
		_propertiesToRead->Add( WPD_OBJECT_ORIGINAL_FILE_NAME );
		_propertiesToRead->Add( WPD_OBJECT_NAME );

		return true;
	} else {
		return false;
	}
}

bool FileSystem::PortableDeviceExplorer::GetAudioPropertiesToRead( IPortableDeviceKeyCollection *& _propertiesToRead ) {
	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceKeyCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &_propertiesToRead ) ) ) ) {
		_propertiesToRead->Add( WPD_MEDIA_TITLE );
		_propertiesToRead->Add( WPD_OBJECT_NAME );
		_propertiesToRead->Add( WPD_MEDIA_ARTIST );
		_propertiesToRead->Add( WPD_MUSIC_ALBUM );
		_propertiesToRead->Add( WPD_MEDIA_GENRE );
		_propertiesToRead->Add( WPD_MEDIA_RELEASE_DATE );
		_propertiesToRead->Add( WPD_MUSIC_TRACK );

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
