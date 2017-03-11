#pragma once

#include "FileSystem.h"
#include "RefCounted.h"
#include "String.h"

namespace MusicTools {
	FileSystem::Explorer * ExploreDriveOrPortableDevice( const Array< FileSystem::Drive > & _drives, const Array< FileSystem::PortableDevice > & _devices, const String & _input );
	void FindMusicAndPlaylists( const RefCountedPtr< FileSystem::Explorer > & _explorer, RefCountedPtr< FileSystem::Explorer > & _musicExplorer, RefCountedPtr< FileSystem::Explorer > & _playlistExplorer, String & _musicExtension );
	void PrintMusicAndPlaylists( const FileSystem::Explorer * _musicExplorer, const FileSystem::Explorer * _playlistExplorer, const String & _musicExtension );

	void GetPlaylistToMusicPath( const String & _musicPath, const String & _playlistPath, String & _playlistToMusicPath );
	bool ConvertPlaylist( const String & _playlist, const FileSystem::ComparePathsInput::PlaylistComparison & _settings, String & _outPlaylist );
}
