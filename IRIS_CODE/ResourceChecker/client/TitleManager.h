/* ==========================================================================
* 파    일 : TitleManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-01-18
* 주의사항 :
*===========================================================================*/
#include "BaseStage.h"

class cTitleDefine
{
public:
	cTitleDefine();

public:
	/// 타이틀명
	LPCTSTR mName;

	/// 타이틀설명
	LPCTSTR mDescription;

	/// 효과 인덱스
	long mInfluenceIndex;

	/// 타이틀 색상
	unsigned long mColor;
};

inline
cTitleDefine::cTitleDefine()
: mName(0)
, mDescription(0)
, mInfluenceIndex(0)
, mColor(0)
{
}

class cTitleManager
{
public: 
	cTitleManager();
	~cTitleManager();

	bool Init();

	cTitleDefine* GetTitleDefine( unsigned long titleIdx );

public:
	static cTitleManager* mpSingleton;
	static cTitleManager* GetSingleton();

private:
	bool Load( const cString& pathname );

// 	/// 호칭 UI 에 리스트 세팅
// 	void SortTitleList();

private:
	/// 호칭 정의
	typedef tPointerHashMap<unsigned long, void*> cTitleDefineMap;
	cTitleDefineMap	mTitleDefineMap;

	/// 소유 호칭 정보
	typedef tHashSet<unsigned long>	cHaveTitleSet;
	cHaveTitleSet	mHaveTitleSet;

	/// 변경 요청 호칭인덱스
	unsigned long mRequestTitleIndex;
};

inline cTitleManager* cTitleManager::GetSingleton()
{
	return mpSingleton;
}

#define TITLEMAN cTitleManager::GetSingleton()

