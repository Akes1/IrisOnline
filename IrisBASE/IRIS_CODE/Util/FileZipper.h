/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.20
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "zlib.h"

class cFileZipper
{
public:
	cFileZipper();
	~cFileZipper();

	/// 열기
	bool Open( const cString& pathName );

	/// 닫기
	void Close();

	/// 데이타 쓰기
	/// 실제로 쓴 데이타 크기를 리턴한다.
	/// 실패하면 0을 리턴한다.
	unsigned int Write( const void* buffer, unsigned int bufferSize );

private:
	gzFile mZipFile;
};

inline
cFileZipper::cFileZipper()
: mZipFile( 0 )
{
}

inline
cFileZipper::~cFileZipper()
{
	Close();
}

inline
bool cFileZipper::Open( const cString& pathName )
{
	assert( mZipFile == 0 );

	mZipFile = gzopen( pathName.Cstr(), "wb" );
	return mZipFile != 0;
}

inline
void cFileZipper::Close()
{
	if( mZipFile )
	{
		gzclose( mZipFile );
		mZipFile = 0;
	}
}

inline
unsigned int cFileZipper::Write( const void* buffer, unsigned int bufferSize )
{
	assert( mZipFile );

	if( mZipFile )
		return gzwrite( mZipFile, buffer, bufferSize );
	else
		return 0;
}
