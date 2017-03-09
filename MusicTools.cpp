#include "Precompiled.h"
#include "Array.h"
#include "Console.h"
#include "MusicTools.h"

void MusicTools::FindMusicAndPlaylists( const RefCountedPtr< FileSystem::Explorer > & _explorer, RefCountedPtr< FileSystem::Explorer > & _musicExplorer, RefCountedPtr< FileSystem::Explorer > & _playlistExplorer, String & _musicExtension ) {
	RefCountedPtr< FileSystem::Explorer > current = _explorer;
	bool findMusicFolder = true;
	bool findPlaylistsFolder = true;
	bool findMusicExtension = true;
	FileSystem::FolderID nextFolderID;

	do {
		const Array< String > & folderNames = current->GetFolderNames();
		const Array< String > & fileNames = current->GetFileNames();

		nextFolderID.Clear();

		if ( current->HasSpecialFolder() ) {
			if ( folderNames.NumItems() == 1 ) {
				current->GetFolderID( 0, nextFolderID );
			} else {
				Printf( TEXT( "Special Folders:\n" ) );
				for ( unsigned int folderIndex = 0; folderIndex < folderNames.NumItems(); folderIndex++ ) {
					const String & folderName = folderNames[ folderIndex ];
					Printf( TEXT( "\t[%u] %s\n" ), folderIndex, folderName.AsChar() );
				}

				Printf( TEXT( "Selection: " ) );
	
				String selection;
				Gets( selection );

				const unsigned int folderIndex = _wtoi( selection );
				if ( folderIndex < folderNames.NumItems() ) {
					current->GetFolderID( folderIndex, nextFolderID );
				}
			}
		} else {
			// Find music folder
			if ( findMusicFolder ) {
				_musicExplorer = current;

				bool foundNewMusicFolder = false;
				for ( unsigned int folderIndex = 0; folderIndex < folderNames.NumItems(); folderIndex++ ) {
					const String & folderName = folderNames[ folderIndex ];
					if ( folderName == TEXT( "Music" ) || folderName == TEXT( "Musique" )
						|| folderName == TEXT( "Music FLAC" ) || folderName == TEXT( "Musique FLAC" )
						|| folderName == TEXT( "Music MP3" ) || folderName == TEXT( "Musique MP3" ) ) {
						current->GetFolderID( folderIndex, nextFolderID );
						foundNewMusicFolder = true;
						break;
					}
				}

				findMusicFolder &= foundNewMusicFolder;
			}

			// Find playlists folder
			if ( findPlaylistsFolder ) {
				_playlistExplorer = current;

				for ( unsigned int fileIndex = 0; fileIndex < fileNames.NumItems(); fileIndex++ ) {
					const String & fileName = fileNames[ fileIndex ];
					const wchar_t * extension = fileName.GetExtension();
					if ( String::Compare( extension, TEXT( "m3u" ) ) == 0 || String::Compare( extension, TEXT( "m3u8" ) ) == 0 ) {
						findPlaylistsFolder = false;
						break;
					}
				}
			}

			// Find music extension
			if ( findMusicExtension && !nextFolderID.IsValid() ) {
				if ( folderNames.IsEmpty() ) {
					for ( unsigned int fileIndex = 0; fileIndex < fileNames.NumItems(); fileIndex++ ) {
						const String & fileName = fileNames[ fileIndex ];
						const wchar_t * extension = fileName.GetExtension();
						if ( String::Compare( extension, TEXT( "flac" ) ) == 0 || String::Compare( extension, TEXT( "mp3" ) ) == 0 ) {
							_musicExtension = extension;
							findMusicExtension = false;
							break;
						}
					}
				} else {
					current->GetFolderID( 0, nextFolderID );
				}
			}
		}

		if ( nextFolderID.IsValid() ) {
			current = current->ExploreFolder( nextFolderID );
		}
	} while ( nextFolderID.IsValid() );
}

void MusicTools::PrintMusicAndPlaylists( const FileSystem::Explorer * _musicExplorer, const FileSystem::Explorer * _playlistExplorer, const String & _musicExtension ) {
	if ( _musicExplorer != NULL ) {
		Printf( TEXT( "\tMusic Path: \"%s\"\n" ), _musicExplorer->GetPath().AsChar() );
	} else {
		Printf( TEXT( "\tMusic Path not found\n" ) );
	}

	if ( _playlistExplorer != NULL ) {
		Printf( TEXT( "\tPlaylists Path: \"%s\"\n" ), _playlistExplorer->GetPath().AsChar() );
	} else {
		Printf( TEXT( "\tPlaylists Path not found\n" ) );
	}

	if ( _musicExtension.IsEmpty() ) {
		Printf( TEXT( "\tMusic Extension not found\n" ) );
	} else {
		Printf( TEXT( "\tMusic Extension: \"%s\"\n" ), _musicExtension.AsChar() );
	}
}

void MusicTools::GetPlaylistToMusicPath( const String & _musicPath, const String & _playlistPath, String & _playlistToMusicPath ) {
	if ( _musicPath.Length() > _playlistPath.Length() ) {
		_playlistToMusicPath = _musicPath + _playlistPath.Length() + 1;
	}
}

bool MusicTools::ConvertPlaylist( const String & _playlist, const FileSystem::ComparePathsInput::PlaylistComparison & _settings, String & _outPlaylist ) {
	_outPlaylist.Clear();
	_outPlaylist.EnsureAllocated( _playlist.Length() + 1 );

	unsigned int index = 0;
	unsigned int filePathIndex;
	unsigned int lastDotIndex;
	unsigned int lineFeedIndex;

	do {
		filePathIndex = 0;
		lastDotIndex = 0;
		lineFeedIndex = 0;

		if ( !_settings.m_sourceMusicPath.IsEmpty() ) {
			if ( String::Compare( _playlist + index, _settings.m_sourceMusicPath, _settings.m_sourceMusicPath.Length() ) == 0 ) {
				index += _settings.m_sourceMusicPath.Length();
				if ( _playlist[ index++ ] != TEXT( '\\' ) ) {
					return false;
				}
			} else {
				return false;
			}
		}

		filePathIndex = index;

		if ( !_settings.m_destinationMusicPath.IsEmpty() ) {
			_outPlaylist += _settings.m_destinationMusicPath;
			_outPlaylist += TEXT( '\\' );
		}

		while ( index < _playlist.Length() ) {
			const wchar_t character = _playlist[ index ];
			if ( character == TEXT( '.' ) ) {
				lastDotIndex = index;
			} else if ( character == TEXT( '\n' ) ) {
				if ( _playlist[ index - 1 ] == TEXT( '\r' ) ) {
					lineFeedIndex = index - 1;
				} else {
					lineFeedIndex = index;
				}
				index++;
				break;
			}
			index++;
		}

		if ( lastDotIndex > 0 ) {
			const unsigned int filePathLength = lastDotIndex - filePathIndex;
			_outPlaylist.Add( _playlist + filePathIndex, filePathLength );
			_outPlaylist += TEXT( '.' );
			_outPlaylist += _settings.m_destinationMusicExtension;
			if ( lineFeedIndex > 0 ) {
				_outPlaylist.Add( _playlist + lineFeedIndex, index - lineFeedIndex );
			}
		} else {
			return false;
		}
	} while ( index < _playlist.Length() );

	return true;
}
