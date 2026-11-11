/* ==========================================================================
* 파    일 : TitleManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-03-07
* 주의사항 :
*===========================================================================*/
#pragma once

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
	void Release();

	cTitleDefine* GetTitleDefine( unsigned long index );

	///
	tPointerHashMap<unsigned int, void*>* GetTitleDefineMap() { return &mTitleDefineMap; }

public:
	static cTitleManager* mpSingleton;
	static cTitleManager* GetSingleton();

protected:
	bool Load( const cString& pathname );

private:
	typedef tPointerHashMap<unsigned int, void*> cTitleDefineMap;
	cTitleDefineMap	mTitleDefineMap;
};

inline cTitleManager* cTitleManager::GetSingleton()
{
	return mpSingleton;
}

#define TITLEMAN cTitleManager::GetSingleton()
