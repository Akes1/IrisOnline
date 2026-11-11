/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 파일 시스템
* 주의사항 : 
*===========================================================================*/
#pragma once

class cFileToRead;
class cFileToWrite;
class cMemFileToRead;

class cFileToReadW;
class cMemFileToReadW;

/// 파일 시스템
class cFileSystem
{
	static cFileSystem* mSingleton;

public:
	cFileSystem();
	virtual ~cFileSystem();

	/// 읽기 전용 파일 열기
	virtual cFileToRead* OpenFileToRead( const cString& pathName );
	virtual cMemFileToRead* OpenMemFileToRead( const cString& pathName );

	cFileToRead* OpenFileToRead_NoPack( const cString& pathName );
	cMemFileToRead* OpenMemFileToRead_NoPack( const cString& pathName );

	/// 쓰기 전용 파일 열기
	virtual cFileToWrite* OpenFileToWrite( const cString& pathName );

	virtual bool FileExist( const cString& pathName );

	LPCRITICAL_SECTION GetReadCS() { return &m_ReadCS; }
	LPCRITICAL_SECTION GetWriteCS() { return &m_WriteCS; }

	//----------------------------
	virtual cFileToReadW* OpenFileToReadW( const cStringW& pathName );
	virtual cMemFileToReadW* OpenMemFileToReadW( const cStringW& pathName );

	cFileToReadW* OpenFileToRead_NoPackW( const cStringW& pathName );
	cMemFileToReadW* OpenMemFileToRead_NoPackW( const cStringW& pathName );

	virtual cFileToWriteW* OpenFileToWriteW( const cStringW& pathName );

	virtual bool FileExistW( const cStringW& pathName );

public:
	/// 단일체를 리턴
	static cFileSystem* GetSingleton();

	CRITICAL_SECTION	m_ReadCS;
	CRITICAL_SECTION	m_WriteCS;
};

inline
cFileSystem* cFileSystem::GetSingleton()
{
	return mSingleton;
}

#define FILESYSTEM cFileSystem::GetSingleton()
