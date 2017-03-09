#include "Precompiled.h"
#include "Array.h"
#include "Console.h"
#include "File.h"
#include "FileSystem.h"
#include "MusicTools.h"
#include "RefCounted.h"
#include "String.h"

int wmain( int argc, wchar_t * argv[] ) {
	HRESULT result = CoInitializeEx( NULL, COINIT_MULTITHREADED );
	bool pause = true;

	if ( SUCCEEDED( result ) ) {
		Console::Init( TEXT( "Music Synchronization Utility " ) );

		{
			RefCountedPtr< FileSystem::Explorer > sourceMusicExplorer;
			RefCountedPtr< FileSystem::Explorer > sourcePlaylistsExplorer;
			String sourceMusicExtension;

			RefCountedPtr< FileSystem::Explorer > destinationMusicExplorer;
			RefCountedPtr< FileSystem::Explorer > destinationPlaylistsExplorer;
			String destinationMusicExtension;

			// Print drives
			Array< FileSystem::Drive > drives;
			Array< FileSystem::PortableDevice > devices;

			FileSystem::EnumerateDrives( drives );
			FileSystem::EnumeratePortableDevices( devices );

			FileSystem::PrintDrives( drives );
			FileSystem::PrintPortableDevices( devices );

			// Ask for the source path
			String source;
			Console::AskString( TEXT( "Source: " ), source );

			RefCountedPtr< FileSystem::Explorer > sourceExplorer = FileSystem::ExploreDriveOrPortableDevice( drives, devices, source );
			if ( sourceExplorer != NULL ) {
				MusicTools::FindMusicAndPlaylists( sourceExplorer, sourceMusicExplorer, sourcePlaylistsExplorer, sourceMusicExtension );
			}

			MusicTools::PrintMusicAndPlaylists( sourceMusicExplorer, sourcePlaylistsExplorer, sourceMusicExtension );

			if ( sourceMusicExplorer != NULL ) {
				// Ask for the destination path
				String destination;
				Console::AskString( TEXT( "Destination: " ), destination );

				RefCountedPtr< FileSystem::Explorer > destinationExplorer = FileSystem::ExploreDriveOrPortableDevice( drives, devices, destination );
				if ( destinationExplorer != NULL ) {
					MusicTools::FindMusicAndPlaylists( destinationExplorer, destinationMusicExplorer, destinationPlaylistsExplorer, destinationMusicExtension );
				}

				MusicTools::PrintMusicAndPlaylists( destinationMusicExplorer, destinationPlaylistsExplorer, destinationMusicExtension );
			}

			if ( sourceMusicExplorer != NULL && destinationMusicExplorer != NULL ) {
				// Ask for music comparison
				if ( Console::AskBool( TEXT( "Compare music? (y/n) " ) ) ) {
					// Ask for audio file tag comparison
					const bool compareAudioFileTags = Console::AskBool( TEXT( "Compare audio file tags? (y/n) " ) );

					// Compare music
					FileSystem::ComparePathsInput compareMusicInput;
					FileSystem::ComparePathsOutput compareMusicOutput;

					compareMusicInput.m_sourceExplorer = sourceMusicExplorer;
					compareMusicInput.m_destinationExplorer = destinationMusicExplorer;
					compareMusicInput.m_audioFileExtensions.Add( TEXT( "flac" ) );
					compareMusicInput.m_audioFileExtensions.Add( TEXT( "mp3" ) );
					compareMusicInput.m_compareFolders = true;
					compareMusicInput.m_compareFiles = true;
					compareMusicInput.m_compareAudioFileTags = compareAudioFileTags;
					compareMusicInput.m_recursive = true;
					compareMusicInput.m_printProgress = true;
					compareMusicInput.m_printTagMismatch = true;

					FileSystem::ComparePaths( compareMusicInput, compareMusicOutput );
					compareMusicOutput.Print( compareMusicInput );
				}

				if ( sourcePlaylistsExplorer != NULL && destinationPlaylistsExplorer != NULL ) {
					// Ask for playlist comparison
					if ( Console::AskBool( TEXT( "Compare playlists? (y/n) " ) ) ) {
						String sourcePlaylistsToMusicPath;
						String destinationPlaylistsToMusicPath;

						MusicTools::GetPlaylistToMusicPath( sourceMusicExplorer->GetPath(), sourcePlaylistsExplorer->GetPath(), sourcePlaylistsToMusicPath );
						MusicTools::GetPlaylistToMusicPath( destinationMusicExplorer->GetPath(), destinationPlaylistsExplorer->GetPath(), destinationPlaylistsToMusicPath );

						// Compare playlists
						FileSystem::ComparePathsInput comparePlaylistsInput;
						FileSystem::ComparePathsOutput comparePlaylistsOutput;

						comparePlaylistsInput.m_sourceExplorer = sourcePlaylistsExplorer;
						comparePlaylistsInput.m_destinationExplorer = destinationPlaylistsExplorer;
						comparePlaylistsInput.m_playlistFileExtensions.Add( TEXT( "m3u" ) );
						comparePlaylistsInput.m_playlistFileExtensions.Add( TEXT( "m3u8" ) );
						comparePlaylistsInput.m_playlistComparison.m_sourceMusicPath = sourcePlaylistsToMusicPath;
						comparePlaylistsInput.m_playlistComparison.m_destinationMusicPath = destinationPlaylistsToMusicPath;
						comparePlaylistsInput.m_playlistComparison.m_destinationMusicExtension = destinationMusicExtension;
						comparePlaylistsInput.m_compareFiles = true;
						comparePlaylistsInput.m_comparePlaylists = true;

						FileSystem::ComparePaths( comparePlaylistsInput, comparePlaylistsOutput );
						comparePlaylistsOutput.Print( comparePlaylistsInput );

						// Ask for action
						if ( !comparePlaylistsOutput.m_differentFiles.IsEmpty() && Console::AskBool( TEXT( "Fix different files? (y/n) " ) ) ) {
							comparePlaylistsOutput.FixDifferentPlaylistFiles( comparePlaylistsInput );
						}
					} else {
						// Nothing else to display, directly exit
						pause = false;
					}
				}
			}
		}

		CoUninitialize();
	} else {
		Printf( TEXT( "COM initialization failed\n" ) );
	}

	// Pause
	if ( pause ) {
		String dummy;
		Gets( dummy );
	}

	return 0;
}
