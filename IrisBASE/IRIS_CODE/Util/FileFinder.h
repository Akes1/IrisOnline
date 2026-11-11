/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.15
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 파일 검색자
class cFileFinder
{
public:
	cFileFinder();
	~cFileFinder();

	bool FindFirst( const charT* dir );
	bool FindNext();
	void Close();

	bool IsDirectory() const;
	DWORD GetFileAttributes() const;
	const charT* GetFileName() const;
	DWORD GetFileSize() const;
	FILETIME GetFileCreateTime() const;
	FILETIME GetFileAccessTime() const;
	FILETIME GetFileWriteTime() const;

protected:
	WIN32_FIND_DATA mFindData;
	HANDLE mHandle;
};

inline
cFileFinder::cFileFinder()
: mHandle( 0 )
{
}

inline
cFileFinder::~cFileFinder()
{
	Close();
}

inline
void cFileFinder::Close()
{
	if( mHandle )
	{
		FindClose( mHandle );
		mHandle = 0;
	}
}

inline
bool cFileFinder::FindFirst( const charT* dir )
{
	Close();

	mHandle = FindFirstFile( dir, &mFindData );
	if( mHandle == INVALID_HANDLE_VALUE )
	{
		mHandle = 0;
		return false;
	}
	else
	{
		return true;
	}
}

inline
bool cFileFinder::FindNext()
{
	return FindNextFile( mHandle, &mFindData ) != 0;
}

inline
bool cFileFinder::IsDirectory() const
{
	return (mFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline
DWORD cFileFinder::GetFileAttributes() const
{
	return mFindData.dwFileAttributes;
}

inline
const charT* cFileFinder::GetFileName() const
{
	return mFindData.cFileName;
}

inline
DWORD cFileFinder::GetFileSize() const
{
	return (mFindData.nFileSizeHigh * MAXDWORD) + mFindData.nFileSizeLow;
}

inline
FILETIME cFileFinder::GetFileCreateTime() const
{
	return mFindData.ftCreationTime;
}

inline
FILETIME cFileFinder::GetFileAccessTime() const
{
	return mFindData.ftLastAccessTime;
}

inline
FILETIME cFileFinder::GetFileWriteTime() const
{
	return mFindData.ftLastWriteTime;
}
