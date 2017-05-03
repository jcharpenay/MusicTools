#include "Precompiled.h"
#include "Console.h"
#include "IOThread.h"

FileSystem::IOThread::IOThread( Explorer & _explorer, bool _verbose, int64_t _tasksMaxDataSize ) :
	m_explorer( &_explorer ),
	m_verbose( _verbose ),
	m_tasksMaxDataSize( _tasksMaxDataSize ) {
}

void FileSystem::IOThread::CreateFolder( const FolderID & _parentFolderID, const String & _folderName ) {
	AddTask( new CreateFolderTask( _parentFolderID, _folderName ) );
}

void FileSystem::IOThread::DeleteFolder( const FolderID & _folderID ) {
	AddTask( new DeleteFolderTask( _folderID ) );
}

void FileSystem::IOThread::CreateFile( const FolderID & _parentFolderID, const String & _fileName, File & _file, bool _readOnly ) {
	AddTask( new CreateFileTask( _parentFolderID, _fileName, _file, _readOnly ) );
}

void FileSystem::IOThread::WriteFile( const FileID & _fileID, File & _file ) {
	AddTask( new WriteFileTask( _fileID, _file ) );
}

void FileSystem::IOThread::DeleteFile( const FileID & _fileID ) {
	AddTask( new DeleteFileTask( _fileID ) );
}

void FileSystem::IOThread::SetFileReadOnly( const FileID & _fileID, bool _readOnly ) {
	AddTask( new SetFileReadOnlyTask( _fileID, _readOnly ) );
}

void FileSystem::IOThread::WaitTaskCompletion() {
	for ( ;; ) {
		{
			ScopedLock< SpinLock > scopedLock( m_tasksLock );
			if ( m_tasks.IsEmpty() ) {
				break;
			}
		}

		Sleep( 0 );
	}
}

void FileSystem::IOThread::Run() {
	while ( !ShouldStopThread() ) {
		Task * task = GetCurrentTask();
		if ( task != NULL ) {
			task->Run( *m_explorer, m_verbose );
			CurrentTaskCompleted();
		} else {
			Sleep( 0 );
		}
	}
}

FileSystem::IOThread::CreateFolderTask::CreateFolderTask( const FolderID & _parentFolderID, const String & _folderName ) :
	m_parentFolderID( _parentFolderID ),
	m_folderName( _folderName ) {
}

void FileSystem::IOThread::CreateFolderTask::Run( Explorer & _explorer, bool _verbose ) {
	FolderID createdFolderID;
	const bool succeeded = _explorer.CreateFolder( m_parentFolderID, m_folderName, createdFolderID );
	if ( _verbose ) {
		if ( succeeded ) {
			Printf( TEXT( "Created folder \"%s\"\n" ), createdFolderID.GetPath().AsChar() );
		} else {
			Printf( TEXT( "Couldn't create folder \"%s\"\n" ), m_folderName.AsChar() );
		}
	}
}

FileSystem::IOThread::DeleteFolderTask::DeleteFolderTask( const FolderID & _folderID ) :
	m_folderID( _folderID ) {
}

void FileSystem::IOThread::DeleteFolderTask::Run( Explorer & _explorer, bool _verbose ) {
	const bool succeeded = _explorer.DeleteFolder( m_folderID );
	if ( _verbose ) {
		if ( succeeded ) {
			Printf( TEXT( "Deleted folder \"%s\"\n" ), m_folderID.GetPath().AsChar() );
		} else {
			Printf( TEXT( "Couldn't delete folder \"%s\"\n" ), m_folderID.GetPath().AsChar() );
		}
	}
}

FileSystem::IOThread::CreateFileTask::CreateFileTask( const FolderID & _parentFolderID, const String & _fileName, File & _file, bool _readOnly ) :
	m_parentFolderID( _parentFolderID ),
	m_fileName( _fileName ),
	m_file( &_file ),
	m_readOnly( _readOnly ) {
}

void FileSystem::IOThread::CreateFileTask::Run( Explorer & _explorer, bool _verbose ) {
	FileID createdFileID;
	const bool succeeded = _explorer.CreateFile( m_parentFolderID, m_fileName, *m_file, createdFileID );
	if ( succeeded && m_readOnly ) {
		_explorer.SetFileReadOnly( createdFileID, true );
	}
	if ( _verbose ) {
		if ( succeeded ) {
			Printf( TEXT( "Created file \"%s\"\n" ), createdFileID.GetPath().AsChar() );
		} else {
			Printf( TEXT( "Couldn't create file \"%s\"\n" ), m_fileName.AsChar() );
		}
	}
}

int64_t FileSystem::IOThread::CreateFileTask::DataSize() const {
	return m_file->GetBuffer().Size();
}

FileSystem::IOThread::WriteFileTask::WriteFileTask( const FileID & _fileID, File & _file ) :
	m_fileID( _fileID ),
	m_file( &_file ) {
}

void FileSystem::IOThread::WriteFileTask::Run( Explorer & _explorer, bool _verbose ) {
	const bool succeeded = _explorer.WriteFile( m_fileID, *m_file );
	if ( _verbose ) {
		if ( succeeded ) {
			Printf( TEXT( "Updated file \"%s\"\n" ), m_fileID.GetPath().AsChar() );
		} else {
			Printf( TEXT( "Couldn't write file \"%s\"\n" ), m_fileID.GetPath().AsChar() );
		}
	}
}

int64_t FileSystem::IOThread::WriteFileTask::DataSize() const {
	return m_file->GetBuffer().Size();
}

FileSystem::IOThread::DeleteFileTask::DeleteFileTask( const FileID & _fileID ) :
	m_fileID( _fileID ) {
}

void FileSystem::IOThread::DeleteFileTask::Run( Explorer & _explorer, bool _verbose ) {
	const bool succeeded = _explorer.DeleteFile( m_fileID );
	if ( _verbose ) {
		if ( succeeded ) {
			Printf( TEXT( "Deleted file \"%s\"\n" ), m_fileID.GetPath().AsChar() );
		} else {
			Printf( TEXT( "Couldn't delete file \"%s\"\n" ), m_fileID.GetPath().AsChar() );
		}
	}
}

FileSystem::IOThread::SetFileReadOnlyTask::SetFileReadOnlyTask( const FileID & _fileID, bool _readOnly ) :
	m_fileID( _fileID ),
	m_readOnly( _readOnly ) {
}

void FileSystem::IOThread::SetFileReadOnlyTask::Run( Explorer & _explorer, bool _verbose ) {
	_explorer.SetFileReadOnly( m_fileID, m_readOnly );
}

void FileSystem::IOThread::AddTask( Task * _task ) {
	if ( m_tasksMaxDataSize > 0 ) {
		while ( m_tasksDataSize >= m_tasksMaxDataSize ) {
			Sleep( 0 );
		}
	}

	ScopedLock< SpinLock > scopedLock( m_tasksLock );
	m_tasks.Add( _task );
	m_tasksDataSize.Add( _task->DataSize() );
}

FileSystem::IOThread::Task * FileSystem::IOThread::GetCurrentTask() {
	ScopedLock< SpinLock > scopedLock( m_tasksLock );
	if ( m_tasks.IsEmpty() ) {
		return NULL;
	} else {
		return m_tasks.First();
	}
}

void FileSystem::IOThread::CurrentTaskCompleted() {
	Task * task;
	{
		ScopedLock< SpinLock > scopedLock( m_tasksLock );
		task = m_tasks.First();
		m_tasks.RemoveIndex( 0 );
		m_tasksDataSize.Add( -task->DataSize() );
	}
	delete task;
}
