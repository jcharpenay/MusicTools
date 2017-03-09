#include "Precompiled.h"
#include "Console.h"
#include "DriveExplorer.h"
#include "File.h"
#include "FileSystem.h"
#include "MusicTools.h"
#include "PortableDeviceExplorer.h"

bool FileSystem::AudioFileTags::operator==( const AudioFileTags & _other ) const {
	const unsigned int relevantTagsMask = m_relevantTagsMask & _other.m_relevantTagsMask;
	return ( m_title == _other.m_title		|| ( relevantTagsMask & TITLE ) == 0 )
		&& ( m_artist == _other.m_artist	|| ( relevantTagsMask & ARTIST ) == 0 )
		&& ( m_album == _other.m_album		|| ( relevantTagsMask & ALBUM ) == 0 )
		&& ( m_genre == _other.m_genre		|| ( relevantTagsMask & GENRE ) == 0 )
		&& ( m_year == _other.m_year		|| ( relevantTagsMask & YEAR ) == 0 )
		&& ( m_track == _other.m_track		|| ( relevantTagsMask & TRACK ) == 0 );
}

bool FileSystem::AudioFileTags::operator!=( const AudioFileTags & _other ) const {
	return !operator==( _other );
}

void FileSystem::AudioFileTags::PrintMismatch( const String & _file, const AudioFileTags & _other ) const {
	const unsigned int relevantTagsMask = m_relevantTagsMask & _other.m_relevantTagsMask;

	Printf( TEXT( "Tag mismatch in \"%s\":\n" ), _file.AsChar() );

	if ( m_title != _other.m_title && ( relevantTagsMask & TITLE ) != 0 ) {
		Printf( TEXT( "\t\"%s\"\n\t\"%s\"\n" ), m_title.AsChar(), _other.m_title.AsChar() );
	}

	if ( m_artist != _other.m_artist && ( relevantTagsMask & ARTIST ) != 0 ) {
		Printf( TEXT( "\t\"%s\"\n\t\"%s\"\n" ), m_artist.AsChar(), _other.m_artist.AsChar() );
	}

	if ( m_album != _other.m_album && ( relevantTagsMask & ALBUM ) != 0 ) {
		Printf( TEXT( "\t\"%s\"\n\t\"%s\"\n" ), m_album.AsChar(), _other.m_album.AsChar() );
	}

	if ( m_genre != _other.m_genre && ( relevantTagsMask & GENRE ) != 0 ) {
		Printf( TEXT( "\t\"%s\"\n\t\"%s\"\n" ), m_genre.AsChar(), _other.m_genre.AsChar() );
	}

	if ( m_year != _other.m_year && ( relevantTagsMask & YEAR ) != 0 ) {
		Printf( TEXT( "\t\"%u\"\n\t\"%u\"\n" ), m_year, _other.m_year );
	}

	if ( m_track != _other.m_track && ( relevantTagsMask & TRACK ) != 0 ) {
		Printf( TEXT( "\t\"%u\"\n\t\"%u\"\n" ), m_track, _other.m_track );
	}
}

FileSystem::ComparePathsInput::ComparePathsInput() :
	m_compareFolders( false ),
	m_compareFiles( false ),
	m_compareAudioFileTags( false ),
	m_comparePlaylists( false ),
	m_recursive( false ),
	m_printProgress( false ),
	m_printTagMismatch( false ) {
}

bool FileSystem::ComparePathsInput::IsAudioFile( const wchar_t * _extension ) const {
	for ( unsigned int extensionIndex = 0; extensionIndex < m_audioFileExtensions.NumItems(); extensionIndex++ ) {
		if ( m_audioFileExtensions[ extensionIndex ] == _extension ) {
			return true;
		}
	}

	return false;
}

bool FileSystem::ComparePathsInput::IsPlaylistFile( const wchar_t * _extension ) const {
	for ( unsigned int extensionIndex = 0; extensionIndex < m_playlistFileExtensions.NumItems(); extensionIndex++ ) {
		if ( m_playlistFileExtensions[ extensionIndex ] == _extension ) {
			return true;
		}
	}

	return false;
}

FileSystem::ComparePathsOutput::ComparePathsOutput() :
	m_audioFileTagsReadFromSource( 0 ),
	m_audioFileTagsReadFromDestination( 0 ) {
}

void FileSystem::ComparePathsOutput::Print( const ComparePathsInput & _input ) const {
	if ( IsEmpty() ) {
		Printf( TEXT( "Everything is OK\n" ) );
	} else {
		const String & sourcePath = _input.m_sourceExplorer->GetPath();
		const String & destinationPath = _input.m_destinationExplorer->GetPath();

		if ( !m_missingFolders.IsEmpty() ) {
			Printf( TEXT( "Missing folders in \"%s\":\n" ), destinationPath.AsChar() );
			for ( unsigned int index = 0; index < m_missingFolders.NumItems(); index++ ) {
				Printf( TEXT( "\t\"%s\"\n" ), m_missingFolders[ index ].GetPath().AsChar() + sourcePath.Length() + 1 );
			}
		}

		if ( !m_extraFolders.IsEmpty() ) {
			Printf( TEXT( "Extra folders in \"%s\":\n" ), destinationPath.AsChar() );
			for ( unsigned int index = 0; index < m_extraFolders.NumItems(); index++ ) {
				Printf( TEXT( "\t\"%s\"\n" ), m_extraFolders[ index ].GetPath().AsChar() + destinationPath.Length() + 1 );
			}
		}

		if ( !m_missingFiles.IsEmpty() ) {
			Printf( TEXT( "Missing files in \"%s\":\n" ), destinationPath.AsChar() );
			for ( unsigned int index = 0; index < m_missingFiles.NumItems(); index++ ) {
				Printf( TEXT( "\t\"%s\"\n" ), m_missingFiles[ index ].GetPath().AsChar() + sourcePath.Length() + 1 );
			}
		}

		if ( !m_extraFiles.IsEmpty() ) {
			Printf( TEXT( "Extra files in \"%s\":\n" ), destinationPath.AsChar() );
			for ( unsigned int index = 0; index < m_extraFiles.NumItems(); index++ ) {
				Printf( TEXT( "\t\"%s\"\n" ), m_extraFiles[ index ].GetPath().AsChar() + destinationPath.Length() + 1 );
			}
		}

		if ( !m_differentFiles.IsEmpty() ) {
			Printf( TEXT( "Different files in \"%s\":\n" ), destinationPath.AsChar() );
			for ( unsigned int index = 0; index < m_differentFiles.NumItems(); index++ ) {
				Printf( TEXT( "\t\"%s\"\n" ), m_differentFiles[ index ].GetPath().AsChar() + destinationPath.Length() + 1 );
			}
		}

		if ( _input.m_compareAudioFileTags ) {
			if ( m_audioFileTagsReadFromSource != AudioFileTags::ALL ) {
				Printf( TEXT( "Audio file tags not successfully read from source: " ) );
				PrintAudioFileTagsNotRead( m_audioFileTagsReadFromSource );
			}

			if ( m_audioFileTagsReadFromDestination != AudioFileTags::ALL ) {
				Printf( TEXT( "Audio file tags not successfully read from destination: " ) );
				PrintAudioFileTagsNotRead( m_audioFileTagsReadFromDestination );
			}
		}
	}
}

void FileSystem::ComparePathsOutput::FixDifferentPlaylistFiles( const ComparePathsInput & _input ) {
	unsigned int playlistIndex = 0;

	for ( unsigned int index = 0; index < m_differentFiles.NumItems(); index++ ) {
		const FileSystem::FileID & fileID = m_differentFiles[ index ];
		const wchar_t * fileExtension = fileID.GetPath().GetExtension();

		if ( _input.IsPlaylistFile( fileExtension ) ) {
			const String & expectedPlaylist = m_expectedPlaylists[ playlistIndex++ ];
								
			TextFile file;
			file.Set( TextFile::UTF_8, expectedPlaylist );

			if ( _input.m_destinationExplorer->WriteFile( fileID, file ) ) {
				Printf( TEXT( "Fixed playlist file \"%s\"\n" ), fileID.GetPath().AsChar() );
			} else {
				Printf( TEXT( "Couldn't write file \"%s\"\n" ), fileID.GetPath().AsChar() );
			}
		}
	}
}

bool FileSystem::ComparePathsOutput::IsEmpty() const {
	return m_missingFolders.IsEmpty()
		&& m_extraFolders.IsEmpty()
		&& m_missingFiles.IsEmpty()
		&& m_extraFiles.IsEmpty()
		&& m_differentFiles.IsEmpty();
}

void FileSystem::ComparePathsOutput::PrintAudioFileTagsNotRead( unsigned int _tagsReadMask ) const {
	if ( ( _tagsReadMask & AudioFileTags::TITLE ) == 0 ) {
		Printf( TEXT( "Title " ) );
	}

	if ( ( _tagsReadMask & AudioFileTags::ARTIST ) == 0 ) {
		Printf( TEXT( "Artist " ) );
	}

	if ( ( _tagsReadMask & AudioFileTags::ALBUM ) == 0 ) {
		Printf( TEXT( "Album " ) );
	}

	if ( ( _tagsReadMask & AudioFileTags::GENRE ) == 0 ) {
		Printf( TEXT( "Genre " ) );
	}

	if ( ( _tagsReadMask & AudioFileTags::YEAR ) == 0 ) {
		Printf( TEXT( "Year " ) );
	}

	if ( ( _tagsReadMask & AudioFileTags::TRACK ) == 0 ) {
		Printf( TEXT( "Track " ) );
	}

	Printf( TEXT( "\n" ) );
}

void FileSystem::EnumerateDrives( Array< FileSystem::Drive > & _drives ) {
	const DWORD driveMask = GetLogicalDrives();
	for ( wchar_t driveLetter = TEXT( 'A' ); driveLetter <= TEXT( 'Z' ); driveLetter++ ) {
		if ( driveMask & ( 1 << ( driveLetter - 'A' ) ) ) {
			Drive & drive = _drives.Add();
			drive.m_path = driveLetter;
			drive.m_path += TEXT( ":\\" );
			
			if ( GetVolumeInformation( drive.m_path, drive.m_name, drive.m_name.MaxLength(), NULL, NULL, NULL, NULL, 0 ) == 0 ) {
				drive.m_name.Clear();
			}

			drive.m_path[ 2 ] = TEXT( '\0' );
			drive.m_path.RefreshLength();
		}
	}
}

void FileSystem::EnumeratePortableDevices( Array< PortableDevice > & _devices ) {
	ComPtr< IPortableDeviceManager > deviceManager;
	if ( SUCCEEDED( CoCreateInstance( CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceManager ) ) ) ) {
		DWORD deviceCount;
		if ( SUCCEEDED( deviceManager->GetDevices( NULL, &deviceCount ) ) && deviceCount > 0 ) {
			wchar_t ** deviceIDs = static_cast< wchar_t ** >( alloca( deviceCount * sizeof( wchar_t * ) ) );
			memset( deviceIDs, 0, deviceCount * sizeof( wchar_t * ) );

			if ( SUCCEEDED( deviceManager->GetDevices( deviceIDs, &deviceCount ) ) ) {
				for ( unsigned int deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++ ) {
					wchar_t * deviceID = deviceIDs[ deviceIndex ];
					DWORD nameLength;

					if ( SUCCEEDED( deviceManager->GetDeviceFriendlyName( deviceID, NULL, &nameLength ) ) && nameLength > 0 ) {
						String name;
						name.EnsureAllocated( nameLength + 1 );

						if ( SUCCEEDED( deviceManager->GetDeviceFriendlyName( deviceID, name, &nameLength ) ) ) {
							name.RefreshLength();

							PortableDevice & device = _devices.Add();
							device.m_id = deviceID;
							device.m_name = name;
						}
					}

					CoTaskMemFree( deviceID );
				}
			}
		}
	}
}

void FileSystem::PrintDrives( const Array< FileSystem::Drive > & _drives ) {
	Printf( TEXT( "Drives:\n" ) );
	for ( unsigned int driveIndex = 0; driveIndex < _drives.NumItems(); driveIndex++ ) {
		const FileSystem::Drive & drive = _drives[ driveIndex ];
		Printf( TEXT( "\t%s\t%s\n" ), drive.m_path.AsChar(), drive.m_name.AsChar() );
	}
}

void FileSystem::PrintPortableDevices( const Array< PortableDevice > & _devices ) {
	Printf( TEXT( "Portable Devices:\n" ) );
	for ( unsigned int deviceIndex = 0; deviceIndex < _devices.NumItems(); deviceIndex++ ) {
		const FileSystem::PortableDevice & device = _devices[ deviceIndex ];
		Printf( TEXT( "\t[%u]\t%s\n" ), deviceIndex, device.m_name.AsChar() );
	}
}

FileSystem::Explorer * FileSystem::ExploreDriveOrPortableDevice( const Array< Drive > & _drives, const Array< PortableDevice > & _devices, const String & _input ) {
	if ( _input[ 0 ] >= TEXT( '0' ) && _input[ 0 ] <= TEXT( '9' ) ) {
		const unsigned int deviceIndex = _wtoi( _input );
		if ( deviceIndex < _devices.NumItems() ) {
			return new PortableDeviceExplorer( _devices[ deviceIndex ] );
		}
	} else {
		wchar_t driveLetter = _input[ 0 ];
		if ( driveLetter >= 'a' && driveLetter <= 'z' ) {
			driveLetter = 'A' + ( driveLetter - 'a' );
		}

		for ( unsigned int driveIndex = 0; driveIndex < _drives.NumItems(); driveIndex++ ) {
			const FileSystem::Drive & drive = _drives[ driveIndex ];
			if ( driveLetter == drive.m_path[ 0 ] ) {
				return new DriveExplorer( drive );
			}
		}
	}

	return NULL;
}

void FileSystem::ComparePaths( const ComparePathsInput & _input, ComparePathsOutput & _output ) {
	const Array< String > & sourceFolderNames = _input.m_sourceExplorer->GetFolderNames();
	const Array< String > & sourceFileNames = _input.m_sourceExplorer->GetFileNames();

	const Array< String > & destinationFolderNames = _input.m_destinationExplorer->GetFolderNames();
	const Array< String > & destinationFileNames = _input.m_destinationExplorer->GetFileNames();

	COORD cursorPosition;
	if ( _input.m_printProgress ) {
		cursorPosition = Console::GetCursorPosition();
		Printf( TEXT( "[0%%]\n" ) );
	}

	// Compare folders
	if ( _input.m_compareFolders ) {
		unsigned int sourceFolderIndex = 0;
		unsigned int destinationFolderIndex = 0;

		while ( sourceFolderIndex < sourceFolderNames.NumItems() && destinationFolderIndex < destinationFolderNames.NumItems() ) {
			const String & sourceFolderName = sourceFolderNames[ sourceFolderIndex ];
			const String & destinationFolderName = destinationFolderNames[ destinationFolderIndex ];
			const int comparison = sourceFolderName.Compare( destinationFolderName );

			if ( comparison == 0 ) {
				if ( _input.m_recursive ) {
					FolderID sourceFolderID;
					FolderID destinationFolderID;

					_input.m_sourceExplorer->GetFolderID( sourceFolderIndex, sourceFolderID );
					_input.m_destinationExplorer->GetFolderID( destinationFolderIndex, destinationFolderID );

					Explorer * sourceFolderExplorer = _input.m_sourceExplorer->ExploreFolder( sourceFolderID );
					Explorer * destinationFolderExplorer = _input.m_destinationExplorer->ExploreFolder( destinationFolderID );

					ComparePathsInput subPathInput = _input;
					subPathInput.m_sourceExplorer = sourceFolderExplorer;
					subPathInput.m_destinationExplorer = destinationFolderExplorer;
					subPathInput.m_printProgress = false;

					if ( _input.m_printProgress ) {
						const unsigned int progress = ( sourceFolderIndex * 100 ) / sourceFolderNames.NumItems();
						Console::WriteLine( cursorPosition, TEXT( "[%u%%] Exploring \"%s\"" ), progress, sourceFolderExplorer->GetPath().AsChar() );
					}

					ComparePaths( subPathInput, _output );

					sourceFolderExplorer->Release();
					destinationFolderExplorer->Release();
				}

				sourceFolderIndex++;
				destinationFolderIndex++;
			} else if ( comparison < 0 ) {
				_input.m_sourceExplorer->GetFolderID( sourceFolderIndex, _output.m_missingFolders.Add() );
				sourceFolderIndex++;
			} else {
				_input.m_destinationExplorer->GetFolderID( destinationFolderIndex, _output.m_extraFolders.Add() );
				destinationFolderIndex++;
			}
		}

		while ( sourceFolderIndex < sourceFolderNames.NumItems() ) {
			_input.m_sourceExplorer->GetFolderID( sourceFolderIndex, _output.m_missingFolders.Add() );
			sourceFolderIndex++;
		}

		while ( destinationFolderIndex < destinationFolderNames.NumItems() ) {
			_input.m_destinationExplorer->GetFolderID( destinationFolderIndex, _output.m_extraFolders.Add() );
			destinationFolderIndex++;
		}
	}

	// Compare files
	if ( _input.m_compareFiles ) {
		unsigned int sourceFileIndex = 0;
		unsigned int destinationFileIndex = 0;

		while ( sourceFileIndex < sourceFileNames.NumItems() && destinationFileIndex < destinationFileNames.NumItems() ) {
			const String & sourceFileName = sourceFileNames[ sourceFileIndex ];
			const String & destinationFileName = destinationFileNames[ destinationFileIndex ];
			const wchar_t * sourceFileExtension = sourceFileName.GetExtension();
			const wchar_t * destinationFileExtension = destinationFileName.GetExtension();
			bool bothAreAudioFiles = false;
			bool bothArePlaylistFiles = false;
			bool ignoreExtensions = false;
			int comparison = 0;

			if ( !_input.m_audioFileExtensions.IsEmpty() ) {
				bothAreAudioFiles = _input.IsAudioFile( sourceFileExtension ) && _input.IsAudioFile( destinationFileExtension );
				ignoreExtensions |= bothAreAudioFiles;
			}

			if ( !_input.m_playlistFileExtensions.IsEmpty() ) {
				bothArePlaylistFiles = _input.IsPlaylistFile( sourceFileExtension ) && _input.IsPlaylistFile( destinationFileExtension );
				ignoreExtensions |= bothArePlaylistFiles;
			}

			if ( ignoreExtensions ) {
				const unsigned int sourceFileNameLength = static_cast< unsigned int >( sourceFileExtension - sourceFileName.AsChar() ) - 1;
				const unsigned int destinationFileNameLength = static_cast< unsigned int >( destinationFileExtension - destinationFileName.AsChar() ) - 1;

				if ( sourceFileNameLength == destinationFileNameLength ) {
					comparison = sourceFileName.Compare( destinationFileName, sourceFileNameLength );
				} else {
					comparison = sourceFileName.Compare( destinationFileName );
				}
			} else {
				comparison = sourceFileName.Compare( destinationFileName );
			}

			if ( comparison == 0 ) {
				if ( bothAreAudioFiles && _input.m_compareAudioFileTags ) {
					FileID sourceFileID;
					FileID destinationFileID;

					_input.m_sourceExplorer->GetFileID( sourceFileIndex, sourceFileID );
					_input.m_destinationExplorer->GetFileID( destinationFileIndex, destinationFileID );

					AudioFileTags sourceFileTags;
					AudioFileTags destinationFileTags;

					if ( _input.m_sourceExplorer->FetchAudioFileTags( sourceFileID, sourceFileTags )
						&& _input.m_destinationExplorer->FetchAudioFileTags( destinationFileID, destinationFileTags )
						&& sourceFileTags != destinationFileTags ) {
						if ( _input.m_printTagMismatch ) {
							sourceFileTags.PrintMismatch( destinationFileName, destinationFileTags );
						}

						_input.m_destinationExplorer->GetFileID( destinationFileIndex, _output.m_differentFiles.Add() );
					}

					_output.m_audioFileTagsReadFromSource |= sourceFileTags.m_relevantTagsMask;
					_output.m_audioFileTagsReadFromDestination |= destinationFileTags.m_relevantTagsMask;
				} else if ( bothArePlaylistFiles && _input.m_comparePlaylists ) {
					FileID sourceFileID;
					FileID destinationFileID;

					_input.m_sourceExplorer->GetFileID( sourceFileIndex, sourceFileID );
					_input.m_destinationExplorer->GetFileID( destinationFileIndex, destinationFileID );

					RefCountedPtr< File > sourceFile = _input.m_sourceExplorer->ReadFile( sourceFileID );
					RefCountedPtr< File > destinationFile = _input.m_destinationExplorer->ReadFile( destinationFileID );

					if ( sourceFile != NULL && destinationFile != NULL ) {
						TextFile sourcePlaylistFile( *sourceFile );
						String sourcePlaylistString;
						String expectedPlaylistString;

						sourcePlaylistFile.ToString( sourcePlaylistString );
						MusicTools::ConvertPlaylist( sourcePlaylistString, _input.m_playlistComparison, expectedPlaylistString );

						TextFile destinationPlaylistFile( *destinationFile );
						String destinationPlaylistString;

						destinationPlaylistFile.ToString( destinationPlaylistString );

						if ( expectedPlaylistString != destinationPlaylistString ) {
							_input.m_destinationExplorer->GetFileID( destinationFileIndex, _output.m_differentFiles.Add() );
							_output.m_expectedPlaylists.Add( expectedPlaylistString );
						}
					}
				}

				sourceFileIndex++;
				destinationFileIndex++;
			} else if ( comparison < 0 ) {
				_input.m_sourceExplorer->GetFileID( sourceFileIndex, _output.m_missingFiles.Add() );
				sourceFileIndex++;
			} else {
				_input.m_destinationExplorer->GetFileID( destinationFileIndex, _output.m_extraFiles.Add() );
				destinationFileIndex++;
			}
		}

		while ( sourceFileIndex < sourceFileNames.NumItems() ) {
			_input.m_sourceExplorer->GetFileID( sourceFileIndex, _output.m_missingFiles.Add() );
			sourceFileIndex++;
		}

		while ( destinationFileIndex < destinationFileNames.NumItems() ) {
			_input.m_destinationExplorer->GetFileID( destinationFileIndex, _output.m_extraFiles.Add() );
			destinationFileIndex++;
		}
	}

	if ( _input.m_printProgress ) {
		Console::WriteLine( cursorPosition, TEXT( "[100%%]" ) );
	}
}
