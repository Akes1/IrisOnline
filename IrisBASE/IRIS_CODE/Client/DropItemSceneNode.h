#pragma once
#include "staticscenenode.h"
#include "Item_Common.h"

class cDropItemSceneNodeParam : public cStaticSceneNodeParam
{
public:
	cDropItemSceneNodeParam();

public:
	/// Drop 좌표
	NiPoint2		mDropStartPos;
	NiPoint2		mDropEndPos;

	/// 서버 관리용 인덱스
	unsigned long	mDropIndex;

	/// 
	unsigned long	mOwner[ MAX_ITEM_OWNER ];
	unsigned long	mTimeToOwn;

	unsigned long	mParty[ MAX_ITEM_PARTY ];
	unsigned long	mTimeToParty;

	bool			mIsRareItem;
};

inline cDropItemSceneNodeParam::cDropItemSceneNodeParam()
: mDropIndex(UINT_MAX)
, mDropStartPos(0.0f, 0.0f)
, mDropEndPos(0.0f, 0.0f)
, mTimeToOwn(0)
, mTimeToParty(0)
{
	memset( mOwner, 0, sizeof(mOwner) );
	memset( mParty, 0, sizeof(mParty) );
}


class cEffectSceneNode;
class cTutorialSignboard;

class cDropItemSceneNode : public cManagedStaticSceneNode
{
	friend class cSceneManager;

public:
	/// 처리
	//virtual void OnProcess( unsigned long time );

	inline unsigned long GetDropIndex() { return mDropIndex; }

	bool IsHeroOwner();

	void ClearEffect();

	void UpdateTutorialBoard();
	void ClearTutorialBoard();

	unsigned long* GetParty() { return mParty; }

protected:
	cDropItemSceneNode( eType type = eDROPITEM );
	virtual ~cDropItemSceneNode();

	/// 초기화
	virtual bool Init( cDropItemSceneNodeParam& param );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	virtual void AddToVisibleArray();

protected:
	/// 드랍 아이템 관리자용 인덱스
	unsigned long mDropIndex;

	/// Drop 목적지 위치(몬스터 위치, 플레이어 위치)
	NiPoint2	mDropStartPos;
	NiPoint2	mDropEndPos;

	unsigned long mOwner[MAX_ITEM_OWNER];
	unsigned long mTimeToOwn;

	unsigned long	mParty[ MAX_ITEM_PARTY ];
	unsigned long	mTimeToParty;

	unsigned long		mEffectIdxByManager;
	cEffectSceneNode*	mOwnerEffect;

	bool		mIsRareItem;

	// 튜토리얼용
	cTutorialSignboard*		mpTutorialBoard;
};
