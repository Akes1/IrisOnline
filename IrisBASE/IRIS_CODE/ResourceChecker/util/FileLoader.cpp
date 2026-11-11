#include "stdafx.h"

#include "./util/FileLoader.h"
#include "FileSystem.h"
#include "File.h"
#include "MemFile.h"

cFileLoader::cFileLoader()
{
	mMemUsed = false;
	mFile = 0;
}

cFileLoader::~cFileLoader()
{
	Close();
}

bool cFileLoader::Open( const cString& pathName, bool useMemory )
{
	/// 이미 열려있는 지 검사
	if( mFile )
	{
		assert( 0 && "file is already opened" );
		return false;
	}

	mMemUsed = useMemory;

	if( useMemory )
	{
		mFile = FILESYSTEM->OpenMemFileToRead( pathName );
	}
	else
	{
		mFile = FILESYSTEM->OpenFileToRead( pathName );
	}
	
	if( mFile )
		FILESYSTEM->WriteLoadLog( pathName );

	return mFile != 0;
}

bool cFileLoader::OpenNoPack( const cString& pathName, bool useMemory )
{
	if( mFile )
	{
		assert( 0 && "file is already opened" );
		return false;
	}

	mMemUsed = useMemory;

	if( useMemory )
	{
		mFile = FILESYSTEM->OpenMemFileToRead_NoPack( pathName );
	}
	else
	{
		mFile = FILESYSTEM->OpenFileToRead_NoPack( pathName );
	}

	if( mFile )
		FILESYSTEM->WriteLoadLog( pathName );

	return mFile != 0;
}


int cFileLoader::Close()
{
	mMemUsed = false;

	if( mFile )
	{
		int ret = mFile->Close();
		delete mFile;
		mFile = 0;
		return ret;
	}
	return 0;
}

unsigned int cFileLoader::Read( void* buffer, unsigned int size )
{
	if( mFile )
	{
		return mFile->Read( buffer, size );
	}
	return 0;
}

bool cFileLoader::ReadInt( int* pvalue )
{
	if( mFile )
	{
		return mFile->Read( pvalue, sizeof(int) ) == sizeof(int);
	}
	return false;
}

bool cFileLoader::ReadUnsignedInt( unsigned int* pvalue )
{
	if( mFile )
	{
		return mFile->Read( pvalue, sizeof(unsigned int) ) == sizeof(unsigned int);
	}
	return false;
}

bool cFileLoader::ReadFloat( float* pvalue )
{
	if( mFile )
	{
		return mFile->Read( pvalue, sizeof(float) ) == sizeof(float);
	}
	return false;
}

unsigned int cFileLoader::ReadLine( char* buffer, unsigned int bufferSize )
{
	if( mFile )
	{
		return mFile->ReadLine( buffer, bufferSize );
	}
	return 0;
}

int cFileLoader::Seek( unsigned int offset, int origin )
{
	if( mFile )
	{
		return mFile->Seek( offset, origin );
	}
	return -1;
}

unsigned int cFileLoader::Tell() const
{
	if( mFile )
	{
		return mFile->Tell();
	}
	return 0;
}

unsigned int cFileLoader::GetSize() const
{
	if( mFile )
	{
		return mFile->GetSize();
	}
	return 0;
}

bool cFileLoader::IsEnd() const
{
	if( mFile )
	{
		return mFile->IsEnd();
	}
	return true;
}

bool cFileLoader::IsEmpty() const
{
	if( mFile )
	{
		return mFile->IsEmpty();
	}
	return true;
}

const char* cFileLoader::GetBufferPtr()
{
	if( mMemUsed && mFile )
	{
		return ((cMemFileToRead*)mFile)->GetBufferPtr();
	}
	return 0;
}
