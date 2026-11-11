#include "stdafx.h"
#include "FileSaver.h"

#include "FileSystem.h"
#include "File.h"

cFileSaver::cFileSaver()
{
	mFile = 0;
}

cFileSaver::~cFileSaver()
{
	Close();
}

bool cFileSaver::Open( const cString& pathName )
{
	/// 이미 열려있는 지 검사
	if( mFile )
	{
		assert( 0 && "file is already opened" );
		return false;
	}

	///
	mFile = FILESYSTEM->OpenFileToWrite( pathName );
	return mFile != 0;
}

int cFileSaver::Close()
{
	if( mFile )
	{
		int ret = mFile->Close();
		delete mFile;
		mFile = 0;
		return ret;
	}
	return 0;
}

unsigned int cFileSaver::Write( const void* buffer, unsigned int bytes )
{
	if( mFile )
	{
		return mFile->Write( buffer, bytes );
	}
	return 0;
}

bool cFileSaver::WriteInt( int value )
{
	if( mFile )
	{
		return mFile->Write( &value, sizeof(int) ) == sizeof(int);
	}
	return false;
}

bool cFileSaver::WriteUnsignedInt( unsigned int value )
{
	if( mFile )
	{
		return mFile->Write( &value, sizeof(unsigned int) ) == sizeof(unsigned int);
	}
	return false;
}

bool cFileSaver::WriteFloat( float value )
{
	if( mFile )
	{
		return mFile->Write( &value, sizeof(float) ) == sizeof(float);
	}
	return false;
}

int cFileSaver::Flush()
{
	if( mFile )
	{
		return mFile->Flush();
	}
	return -1;
}

//-----------------------------------------------------

cFileSaverW::cFileSaverW()
{
	mFile = 0;
}

cFileSaverW::~cFileSaverW()
{
	Close();
}

bool cFileSaverW::Open( const cStringW& pathName )
{
	/// 이미 열려있는 지 검사
	if( mFile )
	{
		assert( 0 && "file is already opened" );
		return false;
	}

	///
	mFile = FILESYSTEM->OpenFileToWriteW( pathName );
	return mFile != 0;
}

int cFileSaverW::Close()
{
	if( mFile )
	{
		int ret = mFile->Close();
		delete mFile;
		mFile = 0;
		return ret;
	}
	return 0;
}

unsigned int cFileSaverW::Write( const void* buffer, unsigned int bytes )
{
	if( mFile )
	{
		return mFile->Write( buffer, bytes );
	}
	return 0;
}

bool cFileSaverW::WriteInt( int value )
{
	if( mFile )
	{
		return mFile->Write( &value, sizeof(int) ) == sizeof(int);
	}
	return false;
}

bool cFileSaverW::WriteUnsignedInt( unsigned int value )
{
	if( mFile )
	{
		return mFile->Write( &value, sizeof(unsigned int) ) == sizeof(unsigned int);
	}
	return false;
}

bool cFileSaverW::WriteFloat( float value )
{
	if( mFile )
	{
		return mFile->Write( &value, sizeof(float) ) == sizeof(float);
	}
	return false;
}

int cFileSaverW::Flush()
{
	if( mFile )
	{
		return mFile->Flush();
	}
	return -1;
}

