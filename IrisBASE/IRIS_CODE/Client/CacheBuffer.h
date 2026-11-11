/* ====================================================================
* 파    일 :	CacheBuffer.h
* 목    적 :	캐쉬를 관리하기 위햔 메모리를 제공하고, 검색을 지원한다.
* 작 성 자 :	이법석
* 작 성 일 :	2006.11.10
*
* 추가내용 :
*		추가날짜 	작업자		추가내용
*		06.11.10	이법석		묵향 소스 참조
* 
* 주의사항 : 글자 길이를 완전유동으로 할 수는 없다.
*			  하지만, 처음 생성시에 고정 사이즈를 변경할 수 있도록 지원한다.
* =================================================================== */
#pragma once

#include "CacheTxtHash.h"

class cCacheBuffer
{
public:
	cCacheBuffer();
	~cCacheBuffer();

	/// 초기부 및 종료부
	bool		Initialize( unsigned long maxContainerNum, unsigned long maxStringSize );
	void		Cleanup();

	/// Item에 대한 제공 인터페이스

	void*		InsertValue( LPTSTR string, unsigned long strSize, HFONT hFont, unsigned long color, void* pValue );
	void		DeleteValue( void* pHashHandle );
	void*		SearchValue( LPTSTR string, unsigned long strSize, HFONT hFont, unsigned long color );

protected:
	sCacheKey*	AllocKeyMem();
	void		FreeKeyMem( sCacheKey* pKey );


protected:
	/// 
	unsigned long			mMaxContainerNum;
	unsigned long			mUsedContainerNum;

	/// Key 구조체 사이즈 (sizeof(sCacheKey) - sizeof(char) + maxStringSize)
	unsigned long 			mKeyMemSize;

	/// 남은 키의 양
	unsigned long			mReservedKeyNum;

	/// Cache Key의 양만큼 메모리를 생성한다.
	char*					mpKeyPool;

	/// Cache Key 테이블 (제공 되어질 대기 Key)
	sCacheKey**				mpKeyTable;

	///
	cCaheTxtHash<void*>*	mpHash;

	sCacheKey*				mpSearchKey;
};