#include "stdafx.h"
#include "cachebuffer.h"

cCacheBuffer::cCacheBuffer() :
mpKeyPool(0), mpKeyTable(0), mpSearchKey(0)
{
	mKeyMemSize = 0;
	mReservedKeyNum = 0;

	mMaxContainerNum = 0;
	mUsedContainerNum = 0;
}

cCacheBuffer::~cCacheBuffer()
{
	Cleanup();
}

/* ------------------------------------------------------------------
* 함수이름 :	Initialize( unsigned long maxContainerNum, unsigned long maxStringSize )
* 목    적 :	캐쉬 관리 메모리를 할당
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cCacheBuffer::Initialize( unsigned long maxContainerNum, unsigned long maxStringSize )
{
	mMaxContainerNum = maxContainerNum;
	mKeyMemSize = sizeof(sCacheKey) + (maxStringSize-1)*sizeof(TCHAR);

	/// Key 메모리 생성
	mpKeyPool = new char[mKeyMemSize*mMaxContainerNum];
	::memset( mpKeyPool, 0, mKeyMemSize*mMaxContainerNum );

	/// 제공되어질 Key 테이블
	mpKeyTable = new sCacheKey*[mMaxContainerNum];
	::memset( mpKeyTable, 0, sizeof(sCacheKey*)*mMaxContainerNum );

	/// Key의 메모리 주소 할당 및 미제공 갯수 셋팅
	for( unsigned long i=0; i<mMaxContainerNum; i++ )
	{
		mpKeyTable[i] = (sCacheKey*)(mpKeyPool + mKeyMemSize*i);
	}
	mReservedKeyNum = mMaxContainerNum;

	/// Hash 설정
	mpHash = NiNew cCaheTxtHash<void*>( maxStringSize );
	mpSearchKey = (sCacheKey*) new char[mKeyMemSize];

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	Cleanup()
* 목    적 :	할당된 메모리 해제
* 주의사항 :	
* ------------------------------------------------------------------ */
void cCacheBuffer::Cleanup()
{
	SAFE_DELETE_ARRAY(mpSearchKey);

	if( mpHash )
	{
		mpHash->RemoveAll();
		NiDelete mpHash;
	}

	SAFE_DELETE_ARRAY(mpKeyTable);

	SAFE_DELETE_ARRAY(mpKeyPool);

	::memset( this, 0, sizeof(cCacheBuffer) );
}

/* ------------------------------------------------------------------
* 함수이름 :	InsertValue( char* string, unsigned long strSize, HFONT hFont, unsigned long color, void* pValue )
* 목    적 :	컨테이너 추가
* 주의사항 :	
* ------------------------------------------------------------------ */
void* cCacheBuffer::InsertValue( LPTSTR string, unsigned long strSize, HFONT hFont, unsigned long color, void* pValue )
{
	/// 저장 컨테이너 키값 생성..
	sCacheKey* pKey = AllocKeyMem();
	if( !pKey )
		return NULL;

	pKey->hFont = hFont;
	pKey->strSize = strSize;
	pKey->color = color;
	
	::_tcsncpy_s( pKey->szString, strSize+1, string, strSize );
	pKey->szString[strSize] = 0;

	mpHash->SetAt( pKey, pValue );

	mUsedContainerNum++;

	return pKey;
}

/* ------------------------------------------------------------------
* 함수이름 :	DeleteValue( void* pHashHandle )
* 목    적 :	컨테이너 제거
* 주의사항 :	
* ------------------------------------------------------------------ */
void cCacheBuffer::DeleteValue( void* pHashHandle )
{
	sCacheKey* pKey = (sCacheKey*)pHashHandle;

	if( mpHash->RemoveAt( pKey ) == false )
	{
		assert(0);
	}

	FreeKeyMem( pKey );
}

/* ------------------------------------------------------------------
* 함수이름 :	SearchValue( char* string, unsigned long strSize, HFONT hFont, unsigned long color )
* 목    적 :	사용 메모리 검색
* 주의사항 :	
* ------------------------------------------------------------------ */
void* cCacheBuffer::SearchValue( LPTSTR string, unsigned long strSize, HFONT hFont, unsigned long color )
{
	mpSearchKey->color = color;
	mpSearchKey->hFont = hFont;
	mpSearchKey->strSize = strSize;

	::_tcsncpy_s( mpSearchKey->szString, strSize+1, string, strSize );
	mpSearchKey->szString[strSize] = 0;

	void* rValue = NULL;
	if( mpHash->GetAt( mpSearchKey, rValue ) == false )
		return 0;

	return rValue;
}

/* ------------------------------------------------------------------
* 함수이름 :	AllocKeyMem()
* 목    적 :	메모리 할당 내부 함수
* 주의사항 :	
* ------------------------------------------------------------------ */

sCacheKey* cCacheBuffer::AllocKeyMem()
{
	///
	sCacheKey* pKey;
	//	pKey = mpKeyTable[0];

	///
	//	mpKeyTable[0] = mpKeyTable[mReservedKeyNum-1];
	//	mReservedKeyNum--;

	pKey = mpKeyTable[mReservedKeyNum-1];
	mReservedKeyNum--;

	return pKey;
}

/* ------------------------------------------------------------------
* 함수이름 :	FreeKeyMem( sCacheKey* pKey )
* 목    적 :	메모리 해제 내부 함수
* 주의사항 :	
* ------------------------------------------------------------------ */
void cCacheBuffer::FreeKeyMem( sCacheKey* pKey )
{
	mpKeyTable[mReservedKeyNum] = pKey;
	mReservedKeyNum++;
}