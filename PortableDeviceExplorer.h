#pragma once

#include "FileSystem.h"

namespace FileSystem {
	class PortableDeviceExplorer : public Explorer {
	public:
		PortableDeviceExplorer( const PortableDevice & _device );

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
		virtual bool IsFileReadOnly( const FileID & _fileID ) const override { return false; }
		virtual bool SetFileReadOnly( const FileID & _fileID, bool _readOnly ) const override { return false; }
		virtual bool FetchAudioFileTags( const FileID & _fileID, AudioFileTags & _tags ) const override;

	private:
		PortableDeviceExplorer() {}

		void ExplorePath( const wchar_t * _objectID );

		static bool GetClientInfo( IPortableDeviceValues *& _clientInfo );
		static bool GetGenericProperties( IPortableDeviceKeyCollection *& _properties );
		static bool GetAudioProperties( IPortableDeviceKeyCollection *& _properties );
		static bool GetSizeProperty( IPortableDeviceKeyCollection *& _properties );
		static void FolderSwapCallback( void * _ptr, unsigned int _firstIndex, unsigned int _secondIndex );
		static void FileSwapCallback( void * _ptr, unsigned int _firstIndex, unsigned int _secondIndex );

		static File * ReadStream( IStream & _stream, DWORD _optimalBufferSize, DWORD _fileSize );
		static bool WriteStream( IStream & _stream, DWORD _optimalBufferSize, const File & _file );

		mutable ComPtr< IPortableDeviceContent >		m_content;
		mutable ComPtr< IPortableDeviceProperties >		m_properties;
		mutable ComPtr< IPortableDeviceKeyCollection >	m_genericProperties;
		mutable ComPtr< IPortableDeviceKeyCollection >	m_audioProperties;
		mutable ComPtr< IPortableDeviceKeyCollection >	m_sizeProperty;

		String				m_ID;
		Array< String >		m_folderIDs;
		Array< String >		m_fileIDs;
	};
}
