#include "stdafx.h"
#include "FilePack.h"

#include "Tokenizer.h"
#include "MemFile.h"
#ifdef _REGENTOOL_
	#include "../GameSrv/ServerCommon/Console.h"
#else
	#include "Console.h"
#endif

cFileTime::cFileTime()
{
	mFileTime.dwLowDateTime = 0;
	mFileTime.dwHighDateTime = 0;
}

cFileTime::cFileTime( time_t time )
//: CTime( time )
{
	LONGLONG ll = Int32x32To64(time, 10000000) + 116444736000000000;
	mFileTime.dwLowDateTime = (DWORD)ll;
	mFileTime.dwHighDateTime = ll >> 32;
}

cFileTime::cFileTime( const FILETIME& time )
//: CTime( time )
{
	mFileTime = time;
}

const FILETIME& cFileTime::GetFileTime()
{
	SYSTEMTIME utc;
	FILETIME   filetime = mFileTime;

	FileTimeToSystemTime( &filetime, &utc );

	utc.wMilliseconds = 0;

	SystemTimeToFileTime( &utc, &filetime );

	mFileTime = filetime;
	return mFileTime;
}


bool cFileInPackInfo::SetPathName( LPCTSTR pathName )
{
	mPathName = pathName;
	return mHeader.SetFileName( pathName );
}

void cFileInPackInfo::SetWriteTime( const time_t& time )
{
	mWriteTime = time;
	mHeader.SetTime( time );
}


void cFileInPackInfo::UpdateFromHeader()
{
	/// 파일 이름
	mPathName = (const charT*)mHeader.GetFileName();

	/// 파일 시간
	mWriteTime = mHeader.GetTime();

	FILETIME   filetime = mWriteTime.GetFileTime( );
	SYSTEMTIME utc;

	FileTimeToSystemTime( &filetime, &utc );
	SystemTimeToFileTime( &utc, &filetime );
}

FILETIME* cFileInPackInfo::GetWriteTimeInExtraBuf()
{
	if( mHeader.GetExtraFieldSize() == 0)
		return 0;

	return (FILETIME*)mHeader.m_pExtraField.GetBuffer();
}

//////////////////////////////////////////////////////////////////////////
cFilePack::cFilePack()
{
}

cFilePack::~cFilePack()
{
	Close();
}

bool cFilePack::Open( LPCTSTR pathName, eOpenMode mode )
{
	if( mode == OPEN_READONLY || mode == OPEN )
	{
		if( ::FileExist( pathName ) == false )
			return false;
	}

	if( mZip.Open( pathName, mode ) == false )
	{
		return false;
	}
	mZip.EnableFindFast( true );
	return true;
}

void cFilePack::Close()
{
	mZip.Close();
}

int cFilePack::FindFile( const cStringT& pathName, bool caseSensitive )
{
	return mZip.FindFile( pathName.Cstr(), caseSensitive ? CZipArchive::ffCaseSens : CZipArchive::ffNoCaseSens );
}

void cFilePack::DeleteFile( int index )
{
	mZip.DeleteFile( ( WORD )index );
}

void cFilePack::DeleteFiles( const tArray<int>& indexArray )
{
	unsigned int size = indexArray.GetSize();
	CZipWordArray temp;

	for( unsigned int i = 0; i < size; ++i )
	{
		temp.Add( (WORD)indexArray[i] );
	}
	mZip.DeleteFiles( temp );
}

void cFilePack::DeleteFiles( CZipWordArray &indexArray )
{
	mZip.DeleteFiles( indexArray );
}

void cFilePack::DeleteFiles( CZipStringArray *stringArray )
{
	mZip.DeleteFiles( *stringArray );
}

bool cFilePack::RenameFile( int index, const cStringT& newPathName )
{
	return mZip.RenameFile( (WORD)index, newPathName.Cstr());
}

bool cFilePack::OpenFile( int index )
{
	return mZip.OpenFile( index );
}

bool cFilePack::CloseFile()
{
	return mZip.CloseFile() == 1 ? true : false;
}

unsigned int cFilePack::ReadFile( void* buffer, unsigned int size )
{
	return mZip.ReadFile( buffer, (DWORD)size );
}

bool cFilePack::OpenNewFile( cFileInPackInfo& fi, int compressLevel, LPCTSTR filePath )
{
	return mZip.OpenNewFile( fi.mHeader, compressLevel, filePath );
}

bool cFilePack::CloseNewFile()
{
	return mZip.CloseNewFile();
}

bool cFilePack::WriteNewFile( const void* buffer, unsigned int size )
{
	assert( buffer );

	if( size )
		return mZip.WriteNewFile( buffer, (DWORD)size );
	else
		return false;
}

void cFilePack::Flush()
{
	mZip.Flush();
}

bool cFilePack::GetFileInfo( cFileInPackInfo* fi, int index ) const
{
	assert( fi );

	if( mZip.GetFileInfo( fi->mHeader, ( unsigned short )index ) == false )
		return false;

	fi->UpdateFromHeader();
	return true;
}
