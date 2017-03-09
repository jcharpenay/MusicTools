#pragma once

#include "FileSystem.h"

namespace FileSystem {
	class PortableDeviceExplorer : public Explorer {
	public:
		PortableDeviceExplorer( const PortableDevice & _device );

		virtual void GetFolderID( unsigned int _index, FolderID & _folderID ) const override;
		virtual void GetFileID( unsigned int _index, FileID & _fileID ) const override;

		virtual Explorer * ExploreFolder( const FolderID & _folderID ) const override;
		virtual File * ReadFile( const FileID & _fileID ) const override { return NULL; }
		virtual bool WriteFile( const FileID & _fileID, const File & _file ) const override { return false; }
		virtual bool DeleteFile( const FileID & _fileID ) const override { return false; }
		virtual bool FetchAudioFileTags( const FileID & _fileID, AudioFileTags & _tags ) const override;

	private:
		PortableDeviceExplorer() {}

		void ExplorePath( const wchar_t * _objectID );

		static bool GetClientInfo( IPortableDeviceValues *& _clientInfo );
		static bool GetGenericPropertiesToRead( IPortableDeviceKeyCollection *& _propertiesToRead );
		static bool GetAudioPropertiesToRead( IPortableDeviceKeyCollection *& _propertiesToRead );
		static void FolderSwapCallback( void * _ptr, unsigned int _firstIndex, unsigned int _secondIndex );
		static void FileSwapCallback( void * _ptr, unsigned int _firstIndex, unsigned int _secondIndex );

		ComPtr< IPortableDeviceContent >				m_content;
		mutable ComPtr< IPortableDeviceProperties >		m_properties;
		ComPtr< IPortableDeviceKeyCollection >			m_genericPropertiesToRead;
		mutable ComPtr< IPortableDeviceKeyCollection >	m_audioPropertiesToRead;

		Array< String >		m_folderIDs;
		Array< String >		m_fileIDs;
	};
}
