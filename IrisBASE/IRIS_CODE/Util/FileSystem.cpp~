#include "stdafx.h"
#include "FileSystem.h"

#include "File.h"
#include "MemFile.h"

cFileSystem* cFileSystem::mSingleton = 0;

cFileSystem::cFileSystem()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	InitializeCriticalSection(&m_ReadCS);
	InitializeCriticalSection(&m_WriteCS);
}

cFileSystem::~cFileSystem()
{
	mSingleton = 0;

	DeleteCriticalSection(&m_ReadCS);
	DeleteCriticalSection(&m_WriteCS);
}

cFileToRead* cFileSystem::OpenFileToRead( const cString& pathName )
{
	cFileToRead* p = new cFileToRead;

	EnterCriticalSection(&m_ReadCS);

	if( p->Open( pathName ) == true )
	{
		LeaveCriticalSection(&m_ReadCS);
		return p;
	}
	else
	{
		delete p;
		LeaveCriticalSection(&m_ReadCS);
		return 0;
	}
}

cMemFileToRead* cFileSystem::OpenMemFileToRead( const cString& pathName )
{
	cMemFileToRead* p = new cMemFileToRead;

	EnterCriticalSection(&m_ReadCS);

	if( p->Open( pathName ) == true )
	{
		LeaveCriticalSection(&m_ReadCS);
		return p;
	}
	else
	{
		delete p;
		LeaveCriticalSection(&m_ReadCS);
		return 0;
	}
}

cFileToRead* cFileSystem::OpenFileToRead_NoPack( const cString& pathName )
{
	return cFileSystem::OpenFileToRead( pathName );
}

cMemFileToRead* cFileSystem::OpenMemFileToRead_NoPack( const cString& pathName )
{
	return cFileSystem::OpenMemFileToRead( pathName );
}

cFileToWrite* cFileSystem::OpenFileToWrite( const cString& pathName )
{
	cFileToWrite* p = new cFileToWrite;
	EnterCriticalSection(&m_WriteCS);

	if( p->Open( pathName ) == true )
	{
		LeaveCriticalSection(&m_WriteCS);
		return p;
	}
	else
	{
		delete p;
		LeaveCriticalSection(&m_WriteCS);
		return 0;
	}
}

bool cFileSystem::FileExist( const cString& pathName )
{
	return ::FileExist( pathName );
}

//-------------------------------------------------------------------
cFileToReadW* cFileSystem::OpenFileToReadW( const cStringW& pathName )
{
	cFileToReadW* p = new cFileToReadW;

	if( p->Open( pathName ) == true )
	{
		return p;
	}
	else
	{
		delete p;
		return 0;
	}
}

cMemFileToReadW* cFileSystem::OpenMemFileToReadW( const cStringW& pathName )
{
	cMemFileToReadW* p = new cMemFileToReadW;

	if( p->Open( pathName ) == true )
	{
		return p;
	}
	else
	{
		delete p;
		return 0;
	}
}

cFileToReadW* cFileSystem::OpenFileToRead_NoPackW( const cStringW& pathName )
{
	return cFileSystem::OpenFileToReadW( pathName );
}

cMemFileToReadW* cFileSystem::OpenMemFileToRead_NoPackW( const cStringW& pathName )
{
	return cFileSystem::OpenMemFileToReadW( pathName );
}

cFileToWriteW* cFileSystem::OpenFileToWriteW( const cStringW& pathName )
{
	cFileToWriteW* p = new cFileToWriteW;
	EnterCriticalSection(&m_WriteCS);

	if( p->Open( pathName ) == true )
	{
		LeaveCriticalSection(&m_WriteCS);
		return p;
	}
	else
	{
		delete p;
		LeaveCriticalSection(&m_WriteCS);
		return 0;
	}
}


bool cFileSystem::FileExistW( const cStringW& pathName )
{
	return ::FileExist( pathName );
}
