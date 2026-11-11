/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.20
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "zlib.h"

class cFileUnzipper
{
public:
	cFileUnzipper();
	~cFileUnzipper();

	/// 열기
	bool Open( const cString& pathName );

	/// 닫기
	void Close();

	/// 데이타 읽기
	/// 실제로 읽은 압축해제된 데이타 크기를 리턴한다.
	/// 파일 끝에 도달하면 0, 실패하면 -1을 리턴한다.
	int Read( void* buffer, unsigned bufferSize );

private:
	gzFile mZipFile;
};

inline
cFileUnzipper::cFileUnzipper()
: mZipFile( 0 )
{
}

inline
cFileUnzipper::~cFileUnzipper()
{
	Close();
}

inline
bool cFileUnzipper::Open( const cString& pathName )
{
	assert( mZipFile == 0 );

	mZipFile = gzopen( pathName.Cstr(), "rb" );
	return mZipFile != 0;
}

inline
void cFileUnzipper::Close()
{
	if( mZipFile )
	{
		gzclose( mZipFile );
		mZipFile = 0;
	}
}

inline
int cFileUnzipper::Read( void* buffer, unsigned bufferSize )
{
	assert( mZipFile );

	if( mZipFile )
		return gzread( mZipFile, buffer, bufferSize );
	else
		return 0;
}
