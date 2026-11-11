#pragma once
#include "BaseObject_Common.h"

class cDynamicSceneNode;
class cEffectSceneNode;
class cDramaturgyInfo;

struct sBulletAction;
struct sCrashInfo;

class cBulletObj
{
public:
	cBulletObj();
	virtual ~cBulletObj();

	bool Init( cDramaturgyInfo* pInfo, sBulletAction* pAct, cDynamicSceneNode* pNode );
	bool Update( unsigned long time );

	inline void SpeedUp( float val ) { mSpeed += val; }

protected:
	void Crash( NiPoint3 dir = -NiPoint3::UNIT_Y );

	void BulletRotate( NiPoint3 startAt, NiPoint3 endAt );

protected:

	struct sCrashInfo
	{
		unsigned short	crashEffectIdx;
		unsigned int	crashLinkPos;
		NiPoint3		crashVar;
	};

	/// 발사체
	unsigned long		mBulletEffectIdx;
	cEffectSceneNode*	mBullet;

	float				mSpeed;
	NiPoint3			mDesiredDir;

	unsigned long		mAccumTime;

	/// 충돌 처리를 위한 정보
	sObject				mTarget;
	unsigned int		mTargetLinkPos;
	NiPoint3			mLastTargetPos;

	///
	bool				mUseFieldTarget;

	/// 충돌시 효과 정보
	bool				mExistCrashEffect;

	unsigned int		mCrashCount;
	sCrashInfo			mCrashInfo[5];

	/// 참조 변수
	cDramaturgyInfo*	mpDramaInfo;
};