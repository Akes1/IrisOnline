#include "stdafx.h"
#include "File.h"

#include <io.h>
#include <tchar.h>

#pragma warning( disable: 4267 )
//#pragma warning( disable: 4996 )

bool GetFilePath( cString* out, const cString& str )
{
	for( int i = (int)str.GetLength() - 1; i >= 0; --i )
	{
		if( str[i] == '/' || str[i] == '\\' )
		{
			*out = str.Left( i + 1 );
			return true;
		}
	}
	return false;
}

bool GetFilePath( cStringW* out, const cStringW& str )
{
	for( int i = (int)str.GetLength() - 1; i >= 0; --i )
	{
		if( str[i] == L'/' || str[i] == L'\\' )
		{
			*out = str.Left( i + 1 );
			return true;
		}
	}
	return false;
}

bool GetFileName( cString* out, const cString& str )
{
	for( int i = (int)str.GetLength() - 1; i >= 0; --i )
	{
		if( str[i] == '/' || str[i] == '\\' )
		{
			*out = str.Mid( i + 1 );
			return true;
		}
	}
	*out = str;
	return false;
}

bool GetFileExtension( cString* out, const cString& str )
{
	unsigned int pos = str.FindReverse('.');
	if( pos == cString::NPOS )
	{
		return false;
	}
	*out = str.Mid( pos + 1 );
	return true;
}

cString GetFileExtension( const cString& str )
{
	unsigned int pos = str.FindReverse('.');
	if( pos == cString::NPOS )
	{
		return cString();
	}
	return str.Mid( pos + 1 );
}

cString GetFileExtension( const char* str,  unsigned int strLen )
{
	for( unsigned int i = strLen - 1; i >= 0; --i )
	{
		if( str[i] == '.' )
			return cString(str + i + 1);
	}
	return cString();
}

bool FileExist( const cString& pathName )
{
	return _access( pathName.Cstr(), 0 ) != -1;
}

bool FileExist( const cStringW& pathName )
{
	return _waccess( pathName.Cstr(), 0 ) != -1;
}

bool FileRemove( const cString& pathName )
{
	return remove( pathName.Cstr() ) == 0;
}

bool FileRename( const cString& oldName, const cString& newName )
{
	return rename( oldName.Cstr(), newName.Cstr() ) == 0;
}

bool FileBackup( const cString& pathName )
{
	cString newName = pathName;
	newName += ".bak";

	if( FileExist( newName ) )
	{
		FileBackup( newName );
	}
	return FileRename( pathName, newName );
}

cFileToRead::cFileToRead()
{
	mSize = 0;
	mFile = 0;
}

cFileToRead::~cFileToRead()
{
	Close();
}

bool cFileToRead::Open( const cString& pathName )
{
	assert( mFile == 0 );

	mFile = 0;

	if( fopen_s( &mFile, pathName.Cstr(), "rb" ) != 0 )
		return false;

	fseek( mFile, 0, SEEK_END );
	mSize = ftell( mFile );
	if( mSize == 0 )
	{
		assert( 0 && "file size is 0" );
		return false;
	}

	fseek( mFile, 0, SEEK_SET );
	return true;
}

int cFileToRead::Close()
{
	mSize = 0;

	if( mFile )
	{
		int ret = fclose( mFile );
		mFile = 0;
		return ret;
	}
	return 0;
}

unsigned int cFileToRead::Read( void* p, unsigned int bytes )
{
	assert( p && bytes > 0 );
	assert( mFile );

	return fread( p, 1, bytes, mFile );
}

unsigned int cFileToRead::ReadLine( char* p, unsigned int bufferSize )
{
	assert( p && bufferSize > 0 );
	assert( mFile );

	int c;
	unsigned int i = 0;

	for( --bufferSize; i < bufferSize; ++i )
	{
		c = fgetc( mFile );

		if( feof( mFile ) || c == '\n' )
		{
			break;
		}

		p[i] = (char)c;
	}

	p[i] = 0;
	return i;
}

int cFileToRead::Seek( unsigned int offset, int origin )
{
	assert( mFile );

	return fseek( mFile, ( long )offset, origin );
}

unsigned int cFileToRead::Tell() const
{
	assert( mFile );

	long pos = ftell( mFile );
	return pos != -1 ? (unsigned int)pos : 0;
}

unsigned int cFileToRead::GetSize() const
{
	return mSize;
}

bool cFileToRead::IsEnd() const
{
	assert( mFile );

	return feof( mFile ) != 0;
}

cFileToWrite::cFileToWrite()
{
	mFile = 0;
}

cFileToWrite::~cFileToWrite()
{
	Close();
}

bool cFileToWrite::Open( const cString& pathName )
{
	assert( mFile == 0 );

	mFile = 0;
	if( fopen_s( &mFile, pathName.Cstr(), "wb" ) != 0 )
		return false;
	else
		return true;
}

int cFileToWrite::Close()
{
	if( mFile )
	{
		int ret = fclose( mFile );
		mFile = 0;
		return ret;
	}
	return 0;
}

unsigned int cFileToWrite::Write( const void* buffer, unsigned int bytes )
{
	assert( mFile );

	return fwrite( buffer, 1, bytes, mFile );
}

int cFileToWrite::Flush()
{
	assert( mFile );

	return fflush( mFile );
}


//-----------------------------------------------------------
cFileToReadW::cFileToReadW()
{
	mSize = 0;
	mFile = 0;
}

cFileToReadW::~cFileToReadW()
{
	Close();
}

bool cFileToReadW::Open( const cStringW& pathName )
{
	assert( mFile == 0 );

	mFile = 0;

	if( _wfopen_s( &mFile, pathName.Cstr(), L"rb" ) != 0 )
		return false;

	fseek( mFile, 0, SEEK_END );
	mSize = ftell( mFile );
	if( mSize == 0 )
	{
		assert( 0 && "file size is 0" );
		return false;
	}

	fseek( mFile, 0, SEEK_SET );
	return true;
}

int cFileToReadW::Close()
{
	mSize = 0;

	if( mFile )
	{
		int ret = fclose( mFile );
		mFile = 0;
		return ret;
	}
	return 0;
}

unsigned int cFileToReadW::Read( void* p, unsigned int bytes )
{
	assert( p && bytes > 0 );
	assert( mFile );

	return fread( p, 1, bytes, mFile );
}

unsigned int cFileToReadW::ReadLine( wchar_t* p, unsigned int bufferSize )
{
	assert( p && bufferSize > 0 );
	assert( mFile );

	int c;
	unsigned int i = 0;

	for( --bufferSize; i < bufferSize; ++i )
	{
		c = fgetc( mFile );

		if( feof( mFile ) || c == L'\n' )
		{
			break;
		}

		p[i] = (char)c;
	}

	p[i] = 0;
	return i;
}

int cFileToReadW::Seek( unsigned int offset, int origin )
{
	assert( mFile );

	return fseek( mFile, ( long )offset, origin );
}

unsigned int cFileToReadW::Tell() const
{
	assert( mFile );

	long pos = ftell( mFile );
	return pos != -1 ? (unsigned int)pos : 0;
}

unsigned int cFileToReadW::GetSize() const
{
	return mSize;
}

bool cFileToReadW::IsEnd() const
{
	assert( mFile );

	return feof( mFile ) != 0;
}




cFileToWriteW::cFileToWriteW()
{
	mFile = 0;
}

cFileToWriteW::~cFileToWriteW()
{
	Close();
}

bool cFileToWriteW::Open( const cStringW& pathName )
{
	assert( mFile == 0 );

	mFile = 0;
	if( _wfopen_s( &mFile, pathName.Cstr(), L"wb" ) != 0 )
		return false;
	else
		return true;
}

int cFileToWriteW::Close()
{
	if( mFile )
	{
		int ret = fclose( mFile );
		mFile = 0;
		return ret;
	}
	return 0;
}

unsigned int cFileToWriteW::Write( const void* buffer, unsigned int bytes )
{
	assert( mFile );

	return fwrite( buffer, 1, bytes, mFile );
}

int cFileToWriteW::Flush()
{
	assert( mFile );

	return fflush( mFile );
}
