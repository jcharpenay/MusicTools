#pragma once

#include "Array.h"
#include "RefCounted.h"
#include "String.h"

class File;

namespace FileSystem {
	class Drive {
	public:
		String	m_path;
		String	m_name;
	};

	class PortableDevice {
	public:
		String	m_id;
		String	m_name;
	};

	class ObjectID {
	public:
		ObjectID() {}

		void Set( const String & _path )						{ m_id.Clear(); m_path = _path; }
		void Set( const String & _id, const String & _path )	{ m_id = _id; m_path = _path; }
		void Clear()											{ m_id.Clear(); m_path.Clear(); }

		bool IsValid() const			{ return !( m_id.IsEmpty() && m_path.IsEmpty() ); }
		const String & GetID() const	{ return m_id; }
		const String & GetPath() const	{ return m_path; }

	private:
		String	m_id;
		String	m_path;
	};

	typedef ObjectID FolderID;
	typedef ObjectID FileID;

	class AudioFileTags {
	public:
		enum Flags {
			TITLE	= BIT( 0 ),
			ARTIST	= BIT( 1 ),
			ALBUM	= BIT( 2 ),
			GENRE	= BIT( 3 ),
			YEAR	= BIT( 4 ),
			TRACK	= BIT( 5 ),
			ALL		= BIT( 6 ) - 1
		};

		String			m_title;
		String			m_artist;
		String			m_album;
		String			m_genre;
		unsigned int	m_year;
		unsigned int	m_track;
		unsigned int	m_relevantTagsMask;

		AudioFileTags() : m_year( 0 ), m_track( 0 ), m_relevantTagsMask( 0 ) {}

		bool operator==( const AudioFileTags & _other ) const;
		bool operator!=( const AudioFileTags & _other ) const;

		void PrintMismatch( const String & _file, const AudioFileTags & _other ) const;
	};

	class Explorer : public RefCounted {
	public:
		virtual void GetFolderID( unsigned int _index, FolderID & _folderID ) const = 0;
		virtual void GetFileID( unsigned int _index, FileID & _fileID ) const = 0;

		virtual Explorer * ExploreFolder( const FolderID & _folderID ) const = 0;
		virtual File * ReadFile( const FileID & _fileID ) const = 0;
		virtual bool WriteFile( const FileID & _fileID, const File & _file ) const = 0;
		virtual bool DeleteFile( const FileID & _fileID ) const = 0;
		virtual bool FetchAudioFileTags( const FileID & _fileID, AudioFileTags & _tags ) const = 0;

		const String & GetPath() const					{ return m_path; }
		const Array< String > & GetFolderNames() const	{ return m_folderNames; }
		const Array< String > & GetFileNames() const	{ return m_fileNames; }
		bool HasSpecialFolder() const					{ return m_hasSpecialFolder; }

	protected:
		Explorer() : m_hasSpecialFolder( false ) {}

		String				m_path;
		Array< String >		m_folderNames;
		Array< String >		m_fileNames;
		bool				m_hasSpecialFolder;
	};

	class ComparePathsInput {
	public:
		class PlaylistComparison {
		public:
			String	m_sourceMusicPath;
			String	m_destinationMusicPath;
			String	m_destinationMusicExtension;
		};

		const Explorer *	m_sourceExplorer;
		const Explorer *	m_destinationExplorer;
		Array< String >		m_audioFileExtensions;
		Array< String >		m_playlistFileExtensions;
		PlaylistComparison	m_playlistComparison;
		bool				m_compareFolders;
		bool				m_compareFiles;
		bool				m_compareAudioFileTags;
		bool				m_comparePlaylists;
		bool				m_recursive;
		bool				m_printProgress;
		bool				m_printTagMismatch;

		ComparePathsInput();

		bool IsAudioFile( const wchar_t * _extension ) const;
		bool IsPlaylistFile( const wchar_t * _extension ) const;
	};

	class ComparePathsOutput {
	public:
		Array< FolderID >	m_missingFolders;
		Array< FolderID >	m_extraFolders;
		Array< FileID >		m_missingFiles;
		Array< FileID >		m_extraFiles;
		Array< FileID >		m_differentFiles;
		unsigned int		m_audioFileTagsReadFromSource;
		unsigned int		m_audioFileTagsReadFromDestination;
		Array< String >		m_expectedPlaylists;

		ComparePathsOutput();

		void Print( const ComparePathsInput & _input ) const;
		void FixDifferentPlaylistFiles( const ComparePathsInput & _input );
		bool IsEmpty() const;

	private:
		void PrintAudioFileTagsNotRead( unsigned int _tagsReadMask ) const;
	};

	void EnumerateDrives( Array< Drive > & _drives );
	void EnumeratePortableDevices( Array< PortableDevice > & _devices );

	void PrintDrives( const Array< Drive > & _drives );
	void PrintPortableDevices( const Array< PortableDevice > & _devices );

	Explorer * ExploreDriveOrPortableDevice( const Array< Drive > & _drives, const Array< PortableDevice > & _devices, const String & _input );

	void ComparePaths( const ComparePathsInput & _input, ComparePathsOutput & _output );
}
