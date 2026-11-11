#include "stdafx.h"
#include "MemFile.h"

cMemFileToRead::cMemFileToRead()
{
	mPos = 0;
}

cMemFileToRead::~cMemFileToRead()
{
	Close();
}

bool cMemFileToRead::Open( const cString& pathName )
{
	mPos = 0;

	FILE* file =0;
	if( fopen_s( &file, pathName.Cstr(), "rb" ) != 0 )
	{
		return false;
	}

	fseek( file, 0, SEEK_END );
	mSize = ftell( file );
	if( mSize == 0 )
	{
		fclose( file );
		return true;
	}
	else
	{
		fseek( file, 0, SEEK_SET );
	}

	/// 파일을 버퍼에 통째로 읽어 들임
	mBuffer.Resize( mSize );


	if( fread( &mBuffer[0], 1, mSize, file ) < mSize )
	{
		mBuffer.Clear();
		fclose( file );
		assert( 0 && "failed to load file into memory" );
		return false;
	}
	fclose( file );
	return true;
}

int cMemFileToRead::Close()
{
	mPos = 0;
	mBuffer.Clear();
	return cFileToRead::Close();
}

unsigned int cMemFileToRead::Read( void* p, unsigned int bytes )
{
	assert( p );
	assert( mSize > 0 );

	if( bytes == 0 || mPos >= mSize )
	{
		assert(0);
		return 0;
	}
	if( mPos + bytes > mSize )
	{
		bytes = mSize - mPos;
	}
	memcpy( p, &mBuffer[mPos], bytes );
	mPos += bytes;
	return bytes;
}

unsigned int cMemFileToRead::ReadLine( char* p, unsigned int bufferSize )
{
	assert( p );
	assert( mSize > 0 );

	int c;
	unsigned int i = 0;
	char* buffer = (char*)&mBuffer[mPos];

	for( --bufferSize; i < bufferSize; ++mPos, ++i, ++buffer )
	{
		c = *buffer;

		if( mPos >= mSize )
		{
			break;
		}
		if( c == '\n' )
		{
			++mPos;
			break;
		}

		p[i] = (char)c;
	}

	p[i] = 0;
	return i;
}

int cMemFileToRead::Seek( unsigned int offset, int origin )
{
	switch( origin )
	{
	case SEEK_SET:
		break;
	case SEEK_CUR:
		offset += mPos;
		break;
	default:
		assert( 0 );
		return -1;
	}

	if( offset >= mSize )
	{
		offset = mSize - 1;
	}
	mPos = offset;
	return 0;
}

unsigned int cMemFileToRead::Tell() const
{
	return mPos;
}

bool cMemFileToRead::IsEnd() const
{
	return mPos >= mSize;
}

const tArray<char>& cMemFileToRead::GetBuffer() const
{
	return mBuffer;
}

const char* cMemFileToRead::GetBufferPtr()
{
	return &mBuffer[0];
}

#if defined(_CLIENT) || defined(_MODELVIEW) || defined(REGEN_TOOL) || defined(MAP_EDITOR)
#include "FilePack.h"

cMemFileInPackToRead::cMemFileInPackToRead()
{
}

cMemFileInPackToRead::~cMemFileInPackToRead()
{
	Close();
}

bool cMemFileInPackToRead::Open( cFilePack* pack, int fileIndex )
{
	if( pack == 0 || pack->IsOpened() == false )
	{
		assert( 0 && "zip archive is not opened" );
		return false;
	}
	if( fileIndex < 0 )
	{
		assert( 0 );
		return false;
	}

	/// 압축 저장소에서 파일 열기
	if( pack->OpenFile( fileIndex ) == false )
	{
		assert( 0 && "failed to open file to read in zip" );
		return false;
	}

	/// 파일 크기 얻어옴
	cFileInPackInfo fi;
	if( pack->GetFileInfo( &fi, fileIndex ) == false )
	{
		assert( 0 && "failed to get file info in zip" );
		return false;
	}

	mSize = fi.GetSize();
	if( mSize == 0 )
	{
		assert( 0 && "file size is 0" );
		return false;
	}

	/// 파일을 버퍼에 통째로 읽어 들임
	mBuffer.Resize( mSize );

	if( pack->ReadFile( &mBuffer[0], (unsigned int)mSize ) != mSize )
	{
		mBuffer.Clear();
		pack->CloseFile();
		assert( 0 && "failed to load file in zip into memory" );
		return false;
	}
	pack->CloseFile();
	return true;
}

int cMemFileInPackToRead::Close()
{
	return cMemFileToRead::Close();
}
#endif

//---------------------------------------------

//-----------------------------------------------------------------------
cMemFileToReadW::cMemFileToReadW()
{
	mPos = 0;
}

cMemFileToReadW::~cMemFileToReadW()
{
	Close();
}

bool cMemFileToReadW::Open( const cStringW& pathName )
{
	mPos = 0;

	FILE* file = 0;
	if( _wfopen_s( &file, pathName.Cstr(), L"rb" ) != 0 )
	{
		return false;
	}

	fseek( file, 0, SEEK_END );
	mSize = ftell( file );
	if( mSize == 0 )
	{
		return true;
	}
	else
	{
		fseek( file, 0, SEEK_SET );
	}

	/// 파일을 버퍼에 통째로 읽어 들임
	mBuffer.Resize( mSize );

	unsigned int bytes = fread( &mBuffer[0], 1, mSize, file );

	//cStringT str;
	//str.Format( _T("%d"), bytes );
	//OutputDebugString( (LPCTSTR)str.Cstr() );

	if( bytes < mSize )
	{
		mBuffer.Clear();
		fclose( file );
		assert( 0 && "failed to load file into memory" );
		return false;
	}

	//for( unsigned int i = 0; i < mBuffer.GetSize(); ++i )
	//{
	//	wchar_t c = mBuffer[i];
	//	str.Format( _T("%c"), c );
	//	OutputDebugString( (LPCTSTR)str.Cstr() );
	//}

	fclose( file );
	return true;
}

int cMemFileToReadW::Close()
{
	mPos = 0;
	mBuffer.Clear();
	return cFileToReadW::Close();
}

unsigned int cMemFileToReadW::Read( void* p, unsigned int bytes )
{
	assert( p );
	assert( mSize > 0 );

	if( bytes == 0 || mPos >= mSize )
	{
		assert(0);
		return 0;
	}
	if( mPos + bytes > mSize )
	{
		bytes = mSize - mPos;
	}
	memcpy( p, &mBuffer[mPos], bytes );
	mPos += bytes;
	return bytes;
}

unsigned int cMemFileToReadW::ReadLine( wchar_t* p, unsigned int bufferSize )
{
	assert( p );
	assert( mSize > 0 );

	int c;
	unsigned int i = 0;
	wchar_t* buffer = (wchar_t*)&mBuffer[mPos];

	for( --bufferSize; i < bufferSize; ++mPos, ++i, ++buffer )
	{
		c = *buffer;

		if( mPos >= mSize )
		{
			break;
		}
		if( c == L'\n' )
		{
			++mPos;
			break;
		}

		p[i] = (wchar_t)c;
	}

	p[i] = 0;
	return i;
}

int cMemFileToReadW::Seek( unsigned int offset, int origin )
{
	switch( origin )
	{
	case SEEK_SET:
		break;
	case SEEK_CUR:
		offset += mPos;
		break;
	default:
		assert( 0 );
		return -1;
	}

	if( offset >= mSize )
	{
		offset = mSize - 1;
	}
	mPos = offset;
	return 0;
}

unsigned int cMemFileToReadW::Tell() const
{
	return mPos;
}

bool cMemFileToReadW::IsEnd() const
{
	return mPos >= mSize;
}

const tArray<wchar_t>& cMemFileToReadW::GetBuffer() const
{
	return mBuffer;
}

const wchar_t* cMemFileToReadW::GetBufferPtr()
{
	return &mBuffer[0];
}

#if defined(_CLIENT) || defined(_MODELVIEW) || defined(REGEN_TOOL) || defined(MAP_EDITOR)
#include "FilePack.h"

cMemFileInPackToReadW::cMemFileInPackToReadW()
{
}

cMemFileInPackToReadW::~cMemFileInPackToReadW()
{
	Close();
}

bool cMemFileInPackToReadW::Open( cFilePack* pack, int fileIndex )
{
	if( pack == 0 || pack->IsOpened() == false )
	{
		assert( 0 && "zip archive is not opened" );
		return false;
	}
	if( fileIndex < 0 )
	{
		assert( 0 );
		return false;
	}

	/// 압축 저장소에서 파일 열기
	if( pack->OpenFile( fileIndex ) == false )
	{
		assert( 0 && "failed to open file to read in zip" );
		return false;
	}

	/// 파일 크기 얻어옴
	cFileInPackInfo fi;
	if( pack->GetFileInfo( &fi, fileIndex ) == false )
	{
		assert( 0 && "failed to get file info in zip" );
		return false;
	}

	mSize = fi.GetSize();
	if( mSize == 0 )
	{
		assert( 0 && "file size is 0" );
		return false;
	}

	/// 파일을 버퍼에 통째로 읽어 들임
	mBuffer.Resize( mSize );

	if( pack->ReadFile( &mBuffer[0], (unsigned int)mSize ) != mSize )
	{
		mBuffer.Clear();
		pack->CloseFile();
		assert( 0 && "failed to load file in zip into memory" );
		return false;
	}
	pack->CloseFile();
	return true;
}

int cMemFileInPackToReadW::Close()
{
	return cMemFileToReadW::Close();
}
#endif