#include "stdafx.h"
#include "FileSystem.h"

#include "File.h"
#include "MemFile.h"

cFileSystem* cFileSystem::mSingleton = 0;

cFileSystem::cFileSystem()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
	mfileStream = 0;
}

cFileSystem::~cFileSystem()
{
	mSingleton = 0;
	mLoadFileList.Clear();
	
	if( mfileStream )
	{
		fclose( mfileStream );
		mfileStream = NULL;
	}
}

cFileToRead* cFileSystem::OpenFileToRead( const cString& pathName )
{
	cFileToRead* p = new cFileToRead;

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

cMemFileToRead* cFileSystem::OpenMemFileToRead( const cString& pathName )
{
	cMemFileToRead* p = new cMemFileToRead;

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

bool cFileSystem::FileExist( const cString& pathName )
{
	return ::FileExist( pathName );
}

void cFileSystem::WriteLoadLog( cString pathName )
{

	char currentDirectory[MAX_PATH] = { 0, };
	::GetCurrentDirectoryA( sizeof(currentDirectory), currentDirectory );
	cString dir = currentDirectory;
	dir.Replace( '\\', '/' );
	dir.ToLower();
	
	pathName.Replace( '\\', '/' );
	pathName.ToLower();

	char buffer[ 1024 ] = { 0, };
	if( pathName.Find( "./" ) == cString::NPOS )
		sprintf_s( buffer ,"%s/%s" , dir.Cstr(), pathName.Cstr() );
	else
	{
		pathName.Replace( "./", "/" );
		sprintf_s( buffer ,"%s%s" , dir.Cstr(), pathName.Cstr() );
	}

	if( mLoadFileList.Insert( buffer ) == true )
	{
		/// 파일로 저장.
		if( mfileStream == 0 )
		{
			cString fileLIstName;
			fileLIstName.Format( "%s\\LoadList.txt", dir.Cstr() );

			fopen_s( &mfileStream, fileLIstName.Cstr(), "wt" );
			if( mfileStream == 0 )
			{
				assert(0);
				return;
			}
		}

		const char* const enter = "\n\0";
		fputs( buffer, mfileStream );
		fputs( enter, mfileStream );
	}

// 	pathName.Replace( '\\', '/' );
// 	pathName.ToLower();
// 
// 	char buffer[ 1024 ] = { 0, };
// 	if( pathName.Find( "./" ) == cString::NPOS )
// 		sprintf_s( buffer ,"./%s" , pathName.Cstr() );
// 	else
// 		sprintf_s( buffer ,"%s" , pathName.Cstr() );
// 
// 	mLoadFileList.Insert( buffer );
// 
// 	/// 파일로 저장.
// 	if( mfileStream == 0 )
// 	{
// 		char currentPathA[MAX_PATH] = {0,};
// 		::GetCurrentDirectoryA( MAX_PATH, currentPathA );
// 		cString fileLIstName;
// 		fileLIstName.Format( "%s\\LoadList.txt", currentPathA );
// 		
// 		fopen_s( &mfileStream, fileLIstName.Cstr(), "wt" );
// 		if( mfileStream == 0 )
// 		{
// 			assert(0);
// 			return;
// 		}
// 	}
// 	 	
//  	if( pathName.Find( "./" ) == cString::NPOS )
//  		sprintf_s( buffer ,"./%s\n" , pathName.Cstr() );
//  	else
//  		sprintf_s( buffer ,"%s\n" , pathName.Cstr() );
// 
// 	
// 	fputs( buffer, mfileStream );
// 

}