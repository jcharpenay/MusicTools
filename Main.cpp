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
			String sourcePlaylistExtension;

			RefCountedPtr< FileSystem::Explorer > destinationMusicExplorer;
			RefCountedPtr< FileSystem::Explorer > destinationPlaylistsExplorer;
			String destinationMusicExtension;
			String destinationPlaylistExtension;

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

			RefCountedPtr< FileSystem::Explorer > sourceExplorer = MusicTools::ExploreDriveOrPortableDevice( drives, devices, source );
			if ( sourceExplorer != NULL ) {
				MusicTools::FindMusicAndPlaylists( sourceExplorer, sourceMusicExplorer, sourcePlaylistsExplorer, sourceMusicExtension, sourcePlaylistExtension );
			}

			MusicTools::PrintMusicAndPlaylists( sourceMusicExplorer, sourcePlaylistsExplorer, sourceMusicExtension, sourcePlaylistExtension );

			if ( sourceMusicExplorer != NULL ) {
				// Ask for the destination path
				String destination;
				Console::AskString( TEXT( "Destination: " ), destination );

				RefCountedPtr< FileSystem::Explorer > destinationExplorer = MusicTools::ExploreDriveOrPortableDevice( drives, devices, destination );
				if ( destinationExplorer != NULL ) {
					MusicTools::FindMusicAndPlaylists( destinationExplorer, destinationMusicExplorer, destinationPlaylistsExplorer, destinationMusicExtension, destinationPlaylistExtension );
				}

				MusicTools::PrintMusicAndPlaylists( destinationMusicExplorer, destinationPlaylistsExplorer, destinationMusicExtension, destinationPlaylistExtension );
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
					compareMusicInput.m_compareFolders = true;
					compareMusicInput.m_compareFiles = true;
					compareMusicInput.m_compareAudioFileTags = compareAudioFileTags;
					compareMusicInput.m_recursive = true;
					compareMusicInput.m_printProgress = true;
					compareMusicInput.m_printTagMismatch = true;

					FileSystem::ComparePaths( compareMusicInput, compareMusicOutput );
					compareMusicOutput.Print( compareMusicInput );

					// Ask for action
					if ( !( compareMusicOutput.m_missingFolders.IsEmpty() && compareMusicOutput.m_missingFiles.IsEmpty() )
						&& sourceMusicExtension == destinationMusicExtension
						&& Console::AskBool( TEXT( "Fix missing files & folders? (y/n) " ) ) ) {
						FileSystem::FixMissingFolders( compareMusicInput, compareMusicOutput );
						FileSystem::FixMissingFiles( compareMusicInput, compareMusicOutput );
					}

					if ( !( compareMusicOutput.m_extraFolders.IsEmpty() && compareMusicOutput.m_extraFiles.IsEmpty() )
						&& Console::AskBool( TEXT( "Delete extra files & folders? (y/n) " ) ) ) {
						FileSystem::FixExtraFolders( compareMusicInput, compareMusicOutput );
						FileSystem::FixExtraFiles( compareMusicInput, compareMusicOutput );
					}

					if ( !compareMusicOutput.m_differentFiles.IsEmpty()
						&& sourceMusicExtension == destinationMusicExtension
						&& Console::AskBool( TEXT( "Fix different files? (y/n) " ) ) ) {
						FileSystem::FixDifferentFiles( compareMusicInput, compareMusicOutput );
					}
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
						comparePlaylistsInput.m_playlistComparison.m_sourceMusicPath = sourcePlaylistsToMusicPath;
						comparePlaylistsInput.m_playlistComparison.m_destinationMusicPath = destinationPlaylistsToMusicPath;
						comparePlaylistsInput.m_playlistComparison.m_destinationMusicExtension = destinationMusicExtension;
						comparePlaylistsInput.m_playlistComparison.m_destinationPlaylistExtension = destinationPlaylistExtension;
						comparePlaylistsInput.m_compareFiles = true;
						comparePlaylistsInput.m_comparePlaylists = true;

						FileSystem::ComparePaths( comparePlaylistsInput, comparePlaylistsOutput );
						comparePlaylistsOutput.Print( comparePlaylistsInput );

						// Ask for action
						if ( !comparePlaylistsOutput.m_missingFiles.IsEmpty()
							&& Console::AskBool( TEXT( "Fix missing files? (y/n) " ) ) ) {
							FileSystem::FixMissingFiles( comparePlaylistsInput, comparePlaylistsOutput );
						}

						if ( !comparePlaylistsOutput.m_extraFiles.IsEmpty()
							&& Console::AskBool( TEXT( "Delete extra files? (y/n) " ) ) ) {
							FileSystem::FixExtraFiles( comparePlaylistsInput, comparePlaylistsOutput );
						}

						if ( !comparePlaylistsOutput.m_differentFiles.IsEmpty()
							&& Console::AskBool( TEXT( "Fix different files? (y/n) " ) ) ) {
							FileSystem::FixDifferentFiles( comparePlaylistsInput, comparePlaylistsOutput );
						}
					} else {
						// Nothing else to display, directly exit
						pause = false;
					}
				}
			}
		}

		CoUninitialize();

		DEBUG_ASSERT( RefCounted::NumObjects() == 0 );
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
