/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.04.28
* 내    용 : 엔진 파일
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <NiFile.h>

class cFileToRead;

/// 엔진 파일
class cEngineFile : public NiFile
{
	NiDeclareDerivedBinaryStream();

public:
	cEngineFile( const char* pathName );//cFileToRead* fileToRead );
	~cEngineFile();

	void Seek( unsigned int offset, int whence );
	unsigned int FileRead( void* buffer, unsigned int bytes );

	unsigned int GetLine( char* buffer, unsigned int maxBytes );
	unsigned int GetFileSize() const;

	virtual void SetEndianSwap(bool bDoSwap);

	const cString GetPathName() { return mPathName; }

protected:	
	cFileLoader* mFileLoader;
	cFileToRead* mFileToRead;

	cString mPathName;
};

template <class T> inline
void StreamLoadBinary( cEngineFile& file, T& value )
{
	file.Read( &value, sizeof(T) );
}

template <class T> inline
void StreamLoadBinary( cEngineFile& file, T* value, unsigned int bytes )
{
	unsigned int r = file.Read( value, bytes );

	if( r != bytes )
	{
		assert(0);

		char str[256]=  {0,};
		::sprintf_s( str, "Failed [%s] file read.", file.GetPathName() );
		NiMessageBox( str,"Error" );
	}
}

template <class T> inline
void BinaryStreamLoadEnum( cEngineFile& file, T* value )
{
	int temp;
	file.Read( &temp, sizeof(int) );
	*value = (T)temp;
}