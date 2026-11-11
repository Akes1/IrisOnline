/// character select & make stage에서 사용하는 카드 연출 노드

#pragma once
#include "StaticSceneNode.h"

class cCardSceneNodeParam : public cStaticSceneNodeParam
{
public:
	cCardSceneNodeParam();

public:
	unsigned char mCardIndex;
};

inline
cCardSceneNodeParam::cCardSceneNodeParam()
: mCardIndex(0)
{
}

class cEffectSceneNode;
class cDisplayObject;

class cCardSceneNode : public cStaticSceneNode
{
	friend class cSceneManager;

public:
	cCardSceneNode( eType type = eSTATIC );
	virtual ~cCardSceneNode();

	enum
	{
		eCard_Stop = 0,
		eCard_Play,
		eCard_Rew,
	};

	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	void UpdateCard( float time );

	/// 초기화
	virtual bool Init( const cCardSceneNodeParam& param );

	void SetFrameTime( float time );

	virtual void SetViewNode( bool view );

	NiPoint3 GetDummyTrans();
	NiMatrix3 GetDummyRot();

	virtual void Play( unsigned long );
	virtual void Rew( unsigned long );

	inline unsigned char GetMoveState() { return mMoveState; }

	inline void SetCurrentPosIdx( int idx ) { mCurrentPosIdx = idx; }
	inline int	GetCurrentPosIdx() { return mCurrentPosIdx; }

	void DestroyEffect();

	inline void SetDisplayObj( cDisplayObject* obj ) { mDisplayObj = obj; }

protected:
	virtual void Stop();

protected:
	cNifTimeManager*	mNifTimeManager;

	unsigned long		mEffectIdx;
	cEffectSceneNode*	mEffect;

	NiAVObject*			mDummy;

	bool			mCardUpdate;
	unsigned long	mCardGotoTime;
	unsigned long	mGotoDeltaTime;

	int				mCurrentPosIdx;

	unsigned char	mMoveState;

	cDisplayObject*	mDisplayObj;
};