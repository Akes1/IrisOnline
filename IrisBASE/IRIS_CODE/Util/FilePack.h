/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.19
* 
* 내    용 : 파일 팩
* 주의사항 : 
*===========================================================================*/
#pragma once

//#include <atltime.h>
#include "ZipArchive.h"

/// 파일 시간
class cFileTime
{
public:
	cFileTime();
	cFileTime( time_t time );
	cFileTime( const FILETIME& time );

	const FILETIME& GetFileTime();

//	void operator =( const cFileTime& that );

protected:
	FILETIME mFileTime;
};

/// 팩 안에 있는 파일의 정보
class cFileInPackInfo
{
	friend class cFilePack;

public:
	cFileInPackInfo() {}
	~cFileInPackInfo() {}

	bool SetPathName( LPCTSTR pathName );
	void SetWriteTime( const time_t& time );

	const cStringT&  GetPathName() const;
	unsigned int	 GetSize() const;
	const cFileTime& GetWriteTime() const;
	bool			 IsDirectory() const;
	time_t			 GetUnixWriteTime();
	
	FILETIME*		 GetWriteTimeInExtraBuf();
	WORD			 GetExtraFieldSize();
	CZipString		 GetComment();

protected:
	void UpdateFromHeader();

protected:
	/// 헤더
	CZipFileHeader mHeader;

	/// 이름
	cStringT mPathName;

	/// 시간
	cFileTime mWriteTime;
};

inline CZipString cFileInPackInfo::GetComment()
{
	return mHeader.GetComment();
}

inline
WORD cFileInPackInfo::GetExtraFieldSize()
{
	return mHeader.GetExtraFieldSize();
}

inline
time_t cFileInPackInfo::GetUnixWriteTime()
{
	return mHeader.GetTime();
}

inline
const cStringT& cFileInPackInfo::GetPathName() const
{
	return mPathName;
}

inline
unsigned int cFileInPackInfo::GetSize() const
{
	return (unsigned int)mHeader.m_uUncomprSize;
}

inline
const cFileTime& cFileInPackInfo::GetWriteTime() const
{
	return mWriteTime;
}

inline
bool cFileInPackInfo::IsDirectory() const
{
	return mHeader.IsDirectory();
}

static BYTE SY_KEY_TABLE[64]=
{
	0x23, 0xbe, 0x84, 0xe1, 
	0x6c, 0xd6, 0xae, 0x52, 
	0x90, 0x49, 0xf1, 0xf1, 
	0xbb, 0xe9, 0xeb, 0xb3, 
	0xa6, 0xdb, 0x3c, 0x87, 
	0x0c, 0x3e, 0x99, 0x24, 
	0x5e, 0x0d, 0x1c, 0x06, 
	0xb7, 0x47, 0xde, 0xb3, 
	0x12, 0x4d, 0xc8, 0x43, 
	0xbb, 0x8b, 0xa6, 0x1f, 
	0x03, 0x5a, 0x7d, 0x09, 
	0x38, 0x25, 0x1f, 0x5d, 
	0xd4, 0xcb, 0xfc, 0x96, 
	0xf5, 0x45, 0x3b, 0x13, 
	0x0d, 0x89, 0x0a, 0x1c, 
	0xdb, 0xae, 0x32, 0x20
};

static BYTE MIX_TABLE[64] =
{
	58, 22, 59, 17, 
	45, 46, 36,  8, 
	57, 62, 18, 27, 
	37,  3, 37, 60, 
	52,  5, 19, 24, 
	40, 14, 11, 53, 
	5,  32, 25, 61, 
	23, 39, 10, 25, 
	54, 45, 16,  2, 
	9,  13, 40, 20, 
	9,  42, 57, 37, 
	8,  33,  0, 20,
	25, 44, 33, 38, 
	30, 14, 33, 48, 
	28, 62, 25, 49, 
	31, 34, 37, 34
};

/// 파일 팩
class cFilePack
{
public:
	/// 열기 모드
	enum eOpenMode
	{
		OPEN_READONLY = CZipArchive::zipOpenReadOnly,
		OPEN = CZipArchive::zipOpen,
		CREATE = CZipArchive::zipCreate
	};

public:
	cFilePack();
	~cFilePack();

	/// 열기
	bool Open( LPCTSTR pathName, eOpenMode mode );

	/// 닫기
	void Close();

	/// 패스워드
	/// 파일 팩을 연 후, 파일을 생성하거나 추가하기 전에 사용한다.
	bool SetPassword( LPCTSTR password = 0 );

	/// Decrypt한 패스워드를 넣는다.
	bool SetPasswordEx( const char* password = 0, int len = 0 );

	/// 파일 검색
	/// 파일 인덱스를 리턴한다. 실패하면 -1을 리턴한다.
	int FindFile( const cStringT& pathName, bool caseSensitive = false );

	/// 파일 삭제
	void DeleteFile( int index );
	void DeleteFiles( const tArray<int>& indexArray );
	void DeleteFiles( CZipWordArray& indexArray );
	void DeleteFiles( CZipStringArray *stringArray );


	/// 파일 이름 변경
	bool RenameFile( int index, const cStringT& newPathName );

	/// 파일 열기
	bool OpenFile( int index );

	/// 파일 닫기
	bool CloseFile();

	/// 파일 읽기
	/// OpenFile() 함수로 열려 있어야 한다.
	/// 성공하면 실제로 읽어들인 바이트 수를 리턴하고, 실패하면 0을 리턴한다.
	unsigned int ReadFile( void* buffer, unsigned int size );

	/// 새 파일 열기
	/// compressLevel : 압축 수준
	/// filePath : 해당 경로의 파일로부터 시간 정보와 속성을 가져온다.
	bool OpenNewFile( cFileInPackInfo& fi, int compressLevel = Z_NO_COMPRESSION, LPCTSTR filePath = 0 );

	/// 압축 파일 안의 파일 닫기
	bool CloseNewFile();

	/// 버퍼의 내용을 압축해서 저장
	/// OpenNewFile() 함수로 열려 있어야 한다.
	bool WriteNewFile( const void* buffer, unsigned int size );

	/// 내부 버퍼의 내용을 모두 저장소에 기록
	void Flush();

	/// 파일이 열렸는지를 리턴
	bool IsOpened();

	/// 파일 수를 리턴
	int GetNumFiles() const;

	/// 파일 정보를 리턴
	bool GetFileInfo( cFileInPackInfo* fi, int index ) const;

	/// 추가 메모리 설정
	const char*  SetExtraFieldEx(const char *pBuf, WORD iSize);

	/// 파일 이름들로 인덱스들 얻어오기
	void  GetIndexes(const CZipStringArray &aNames, CZipWordArray& aIndexes);

	/// 파일 이름으로 인덱스 얻어오기
	int   GetIndex( LPCTSTR Names );

	/// 메모리에 압축 풀기
	bool ExtractFileInMem(WORD uIndex, CZipMemFile& mf, bool bRewind = true, DWORD nBufSize = 65536);

	/// 메모리에 있는 파일을 팩에 추가하기.
	bool AddNewFile(CZipMemFile& mf, LPCTSTR lpszFileNameInZip,	int iComprLevel = -1, int iSmartLevel = CZipArchive::zipsmSafeSmart,	unsigned long nBufSize = 65536);
	
	bool SetFileComment(WORD uIndex, LPCTSTR lpszComment);

private:
	/// 압축 저장소
	CZipArchive mZip;
};

inline
bool cFilePack::SetFileComment(WORD uIndex, LPCTSTR lpszComment)
{
	return mZip.SetFileComment( uIndex, lpszComment );
}

inline
int cFilePack::GetIndex( LPCTSTR Names )
{
	return mZip.FindFile( Names );
}

inline
bool cFilePack::AddNewFile( CZipMemFile& mf, LPCTSTR lpszFileNameInZip, int iComprLevel /* = -1 */, int iSmartLevel /* = CZipArchive::zipsmSafeSmart */, unsigned long nBufSize /* = 65536 */ )
{
	return mZip.AddNewFile( mf, lpszFileNameInZip, iComprLevel, iSmartLevel, nBufSize );
}

inline
bool cFilePack::ExtractFileInMem(WORD uIndex, CZipMemFile& mf, bool bRewind /* = true */, DWORD nBufSize /* = 65536 */)
{
	return mZip.ExtractFile( uIndex, mf, bRewind, nBufSize );
}

inline
void cFilePack::GetIndexes( const CZipStringArray &aNames, CZipWordArray& aIndexes )
{
	return mZip.GetIndexes( aNames, aIndexes );
}

inline
bool cFilePack::SetPassword( const charT* password )
{
	return mZip.SetPassword( password );
}

inline
bool cFilePack::SetPasswordEx( const char* password , int len )
{
 	char *pDecryptPassword;
 	pDecryptPassword = new char[len+1];
	pDecryptPassword[len] = 0;
	for( int idx = 0; idx < len; ++idx )
	{
		BYTE keyIdx = MIX_TABLE[ idx % 64 ];
		pDecryptPassword[idx] = password[idx] ^ SY_KEY_TABLE[keyIdx];
	}

 	bool ret = false; 

	charT *pUnicodeDecryptPassword;
	pUnicodeDecryptPassword = new charT[len+2];
	pUnicodeDecryptPassword[len+1] = 0;

#ifdef _UNICODE
	size_t lenDecrypt = ::strlen(pDecryptPassword);
	if( lenDecrypt == 0 )
		return ret;
	int iLen = ::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pDecryptPassword, lenDecrypt, NULL, 0 );
	if (iLen > 0)
	{
		assert( iLen+1 < (int)lenDecrypt+2 );
		iLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pDecryptPassword, -1, (LPWSTR)pUnicodeDecryptPassword, iLen+1 );
		assert(iLen != 0);
	}
	else
	{
		iLen --; // return -1
	}
#else
	#if (_MSC_VER >= 1400)
		_tcscpy_s( pUnicodeDecryptPassword, len+2, pDecryptPassword );	
	#else  
		_tcscpy( pUnicodeDecryptPassword, pDecryptPassword );	
	#endif
#endif

	ret = mZip.SetPassword( pUnicodeDecryptPassword );

	delete[] pDecryptPassword;
	delete[] pUnicodeDecryptPassword;

	return ret;
}

inline
bool cFilePack::IsOpened()
{
	return !mZip.IsClosed();
}

inline
int cFilePack::GetNumFiles() const
{
	return mZip.GetCount();
}

inline
const char* cFilePack::SetExtraFieldEx( const char *pBuf, WORD iSize )
{
	return mZip.SetExtraFieldEx( pBuf, iSize );
};

