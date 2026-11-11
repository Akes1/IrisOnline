/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 파일 팩 시스템
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "FileSystem.h"

class cFileToRead;
class cFileToWrite;
class cMemFileToRead;
class cFilePack;

/// 파일 팩 시스템
/// 읽기 전용 파일을 열 때 팩킹 시스템에서 열지 못하면 로컬 시스템에서 연다.
/// 쓰기 전용 파일은 로컬 시스템에서만 열 수 있다.
class cFilePackSystem : public cFileSystem
{
public:
	cFilePackSystem();
	~cFilePackSystem();

	/// 초기화
	bool Init();

	/// 읽기 전용 파일 열기
	cFileToRead* OpenFileToRead( const cString& pathName );
	cMemFileToRead* OpenMemFileToRead( const cString& pathName );

	virtual bool FileExist( const cString& pathName );

private:
	/// 파일 팩
	typedef tArray<void*> cFilePackArray;
	cFilePackArray mFilePackArray;

	typedef tHashMap<cString, unsigned int> cFileIndexMap;
	cFileIndexMap mFileIndexMap;
};
