#pragma once

#include "Array.h"
#include "File.h"
#include "FileSystem.h"
#include "Interlocked.h"
#include "Lock.h"
#include "RefCounted.h"
#include "Thread.h"

namespace FileSystem {
	class IOThread : public Thread {
	public:
		// Constructor
		IOThread( Explorer & _explorer, bool _verbose, int64_t _tasksMaxDataSize );

		// Operations
		void CreateFolder( const FolderID & _parentFolderID, const String & _folderName );
		void DeleteFolder( const FolderID & _folderID );
		void CreateFile( const FolderID & _parentFolderID, const String & _fileName, File & _file, bool _readOnly );
		void WriteFile( const FileID & _fileID, File & _file );
		void DeleteFile( const FileID & _fileID );
		void SetFileReadOnly( const FileID & _fileID, bool _readOnly );

		void WaitTaskCompletion();

	protected:
		virtual void Run() override;

	private:
		class Task {
		public:
			virtual ~Task() {}
			virtual void Run( Explorer & _explorer, bool _verbose ) = 0;
			virtual int64_t DataSize() const { return 0; }
		};

		class CreateFolderTask : public Task {
		public:
			CreateFolderTask( const FolderID & _parentFolderID, const String & _folderName );
			virtual void Run( Explorer & _explorer, bool _verbose ) override;

		private:
			FolderID	m_parentFolderID;
			String		m_folderName;
		};

		class DeleteFolderTask : public Task {
		public:
			DeleteFolderTask( const FolderID & _folderID );
			virtual void Run( Explorer & _explorer, bool _verbose ) override;

		private:
			FolderID	m_folderID;
		};

		class CreateFileTask : public Task {
		public:
			CreateFileTask( const FolderID & _parentFolderID, const String & _fileName, File & _file, bool _readOnly );
			virtual void Run( Explorer & _explorer, bool _verbose ) override;
			virtual int64_t DataSize() const override;

		private:
			FolderID				m_parentFolderID;
			String					m_fileName;
			RefCountedPtr< File >	m_file;
			bool					m_readOnly;
		};

		class WriteFileTask : public Task {
		public:
			WriteFileTask( const FileID & _fileID, File & _file );
			virtual void Run( Explorer & _explorer, bool _verbose ) override;
			virtual int64_t DataSize() const override;

		private:
			FileID					m_fileID;
			RefCountedPtr< File >	m_file;
		};

		class DeleteFileTask : public Task {
		public:
			DeleteFileTask( const FileID & _fileID );
			virtual void Run( Explorer & _explorer, bool _verbose ) override;

		private:
			FileID	m_fileID;
		};

		class SetFileReadOnlyTask : public Task {
		public:
			SetFileReadOnlyTask( const FileID & _fileID, bool _readOnly );
			virtual void Run( Explorer & _explorer, bool _verbose ) override;

		private:
			FileID	m_fileID;
			bool	m_readOnly;
		};

		Explorer *			m_explorer;
		bool				m_verbose;
		SpinLock			m_tasksLock;
		Array< Task * >		m_tasks;
		InterlockedInt64	m_tasksDataSize;
		int64_t				m_tasksMaxDataSize;

		void AddTask( Task * _task );
		Task * GetCurrentTask();
		void CurrentTaskCompleted();
	};
};
