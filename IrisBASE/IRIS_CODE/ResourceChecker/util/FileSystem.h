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

/// 파일 시스템
class cFileSystem
{
	static cFileSystem* mSingleton;

public:
	cFileSystem();
	~cFileSystem();

	/// 읽기 전용 파일 열기
	virtual cFileToRead* OpenFileToRead( const cString& pathName );
	virtual cMemFileToRead* OpenMemFileToRead( const cString& pathName );

	cFileToRead* OpenFileToRead_NoPack( const cString& pathName );
	cMemFileToRead* OpenMemFileToRead_NoPack( const cString& pathName );

	/// 쓰기 전용 파일 열기
	virtual cFileToWrite* OpenFileToWrite( const cString& pathName );

	virtual bool FileExist( const cString& pathName );

	void WriteLoadLog( cString pathName );
	tHashSet<cString>& GetLog(){ return mLoadFileList; }

public:
	/// 단일체를 리턴
	static cFileSystem* GetSingleton();

private:
	tHashSet<cString> mLoadFileList;
	FILE* mfileStream;
};

inline
cFileSystem* cFileSystem::GetSingleton()
{
	return mSingleton;
}

#define FILESYSTEM cFileSystem::GetSingleton()
