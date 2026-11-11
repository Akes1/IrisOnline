/* ==========================================================================
* 파    일 : FilterManager.cpp
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-08-23
* 주의사항 :
*===========================================================================*/
#pragma once

class cFilterManager
{
public:
	static cFilterManager* mSingleton;

public:
	cFilterManager();
	~cFilterManager();

	///
	bool Init();

	/// 전체 매치 검사 ( 동일한 단어 있는지 검사 )
	bool CheckWholeMatch( LPCTSTR msg );

	/// 부분 매치 검사 ( 포함되는 단어 있는지 검사 )
	bool CheckPartMatch( LPCTSTR msg );

	void AddEmotionFilter( LPCTSTR text, unsigned long index );
	unsigned long GetEmontionIndex( LPCTSTR msg );

public:
	static cFilterManager* GetSingleton();

private:
	/// 스크립트 로드
	bool LoadFilter( const cStringW& pathName );

	/// 전체 매치 찾기
	bool FindWholeMatch( const cStringT& str );

private:
	/// 동일하면 필터할 단어 셋
	typedef tHashSet<cStringT> cWholeSet;
	cWholeSet	mWholeSet;

	/// 포함되었는지 검사할 단어
	typedef tHashMap<cStringT, void*> cPartMap;
	cPartMap	mPartMap;

	///
	typedef tHashMap<cStringT, unsigned long> cEmotionSibling;
	typedef tHashMap<cStringT, void*> cEmotionFilterMap;
	cEmotionFilterMap	mEmotionFilterMap;


	/// 동일한 글자로 시작되는 배열
	typedef tArray<cStringT> cSiblingArr;
};

inline cFilterManager* cFilterManager::GetSingleton()
{
	return mSingleton;
}

#define FILTERMAN cFilterManager::GetSingleton()