#pragma once

#include "FileSystem.h"

namespace FileSystem {
	class DriveExplorer : public Explorer {
	public:
		DriveExplorer( const Drive & _drive );

		virtual void GetID( FolderID & _folderID ) const override;
		virtual void GetFolderID( unsigned int _index, FolderID & _folderID ) const override;
		virtual void GetFileID( unsigned int _index, FileID & _fileID ) const override;

		virtual Explorer * ExploreFolder( const FolderID & _folderID ) const override;
		virtual bool CreateFolder( const FolderID & _parentFolderID, const String & _folderName, FolderID & _folderID ) const override;
		virtual bool DeleteFolder( const FolderID & _folderID ) const override;
		virtual File * ReadFile( const FileID & _fileID ) const override;
		virtual bool CreateFile( const FolderID & _parentFolderID, const String & _fileName, const File & _file, FileID & _fileID ) const override;
		virtual bool WriteFile( const FileID & _fileID, const File & _file ) const override;
		virtual bool DeleteFile( const FileID & _fileID ) const override;
		virtual bool IsFileReadOnly( const FileID & _fileID ) const override;
		virtual bool SetFileReadOnly( const FileID & _fileID, bool _readOnly ) const override;
		virtual bool FetchAudioFileTags( const FileID & _fileID, AudioFileTags & _tags ) const override;

	private:
		DriveExplorer() {}

		static bool ExplorePath( const String & _path, Array< String > & _folderNames, Array< String > & _fileNames );
	};
}
