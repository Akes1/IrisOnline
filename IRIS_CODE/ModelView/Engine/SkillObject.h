/* =====================================================================
* 파    일 :	SkillObject.h , SkillObject.cpp
* 목    적 :	사용중인 스킬 객체 , 상속을 받아 사용한다.
* 작 성 자 :	이법석
* 작 성 일 :    2007년 7월 13일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* =======================================================================*/
#pragma once
#include "BaseObject_Common.h"
#include "DramaturgyInfo.h"
#include "Skill_Common.h"

struct sTargetDamage;
struct sTargetHeal;

enum eAPPLYDRAMA_TYPE;

class cDynamicSceneNode;
class cEffectSceneNode;
class cSoundSceneNode;
struct sTrailInfo;

class cBaseObject;
class cBulletObj;

struct sApplyDrama
{
	sObject mTargetObj;
	cDramaturgyInfo* mpInfo;
};

struct sLoopEffect
{
	unsigned long mIdxByManager;
	cEffectSceneNode* mEffectNode;

	sLoopEffect()
	{
		mIdxByManager = (unsigned long)-1;
		mEffectNode = 0;
	}
};

struct sLoopSound
{
	unsigned long mIdxByManager;
	cSoundSceneNode* mSoundNode;

	sLoopSound()
	{
		mIdxByManager = (unsigned long)-1;
		mSoundNode = 0;
	}
};

///
class cDramaturgyObject 
{
public:
	virtual ~cDramaturgyObject();

	virtual void Process( unsigned long time ) = 0;

	/// TODO:
	/// 대상에게 객체를 링크 하면서
	/// 제거 하는 루틴을 넣어야 한다.
	/// Thread에 걸면서 추가 처리도 한다.

	/// TODO : 
	/// 연출 상태 변경 함수
	inline eDRAMASTATE GetDramaState() { return mCurrentState; }
	inline void SetDramaState( eDRAMASTATE state ) { mCurrentState = state; }

	inline unsigned long	GetManagedKey() { return mManagedKey; }
	inline unsigned long	GetSkillIdx() { return mSkillIdx; }

	void	Removed() { mRemoved = true; }
	bool	IsRemoved() { return mRemoved; }

protected:
	virtual void UpdateDramaObject( sDramaActionBase* pBase, bool bTarget = false, const NiPoint3& dir = -NiPoint3::UNIT_Y );
	virtual void UpdateTrailObject( sDramaActionBase* pBase );
	virtual void AddLoopEffect( cEffectSceneNode* pNode );
	virtual void AddLoopSound( cSoundSceneNode* pNode );
	virtual void AddLoopTrail( sTrailInfo* pInfo );

	void ChangeDramaState( eDRAMASTATE state );

protected:
	cDramaturgyObject();

	/// 연출 상태 제어 구조체
	struct sDramaStateController
	{
		sDramaStateController()
			: pAct(0), startTime(ULONG_MAX)
		{
			animationIdx = UINT_MAX;
		}
		sDramaActionBase*	pAct;
		unsigned int		animationIdx;


		unsigned long	startTime;

		tPointerList<void*>	mLoopEffect;
		tPointerList<void*>	mLoopSound;
		tPointerList<void*>	mLoopTrail;
	};

	/// 객체 보관소에 대한 참조 정보
	unsigned long			mManagedKey;

	/// 스킬 정보
	unsigned long			mSkillIdx;

	/// 연출 정보
	eDRAMASTATE				mCurrentState;
	cDramaturgyInfo*		mpDramaInfo;

	cDynamicSceneNode*		mViewNode;

	/// 적용 연출 정보
	sDramaStateController	mApplyController;

	bool			mRemoved;

};

//////////////////////////////////////////////////////////////////////////
enum eSPECIAL
{
	eSpecial_None = 0,
	eSpecial_Field,
	eSpecial_Blink,

	eSpecial_MapChange,
	eSpecial_Vehicle,
};

///
class cSkillObject : public cDramaturgyObject
{
public:
	cSkillObject();
	virtual ~cSkillObject();

	void*			operator new( size_t n ); 
	void			operator delete( void* ptr );

	enum eVERIFY
	{
		eVerify_Faile = 0,
		eVerify_Wait,
		eVerify_Success,
	};

	virtual bool Init( unsigned long skillIdx, float speedFactor );

	virtual void Process( unsigned long time );

	void			SetManagedKey( unsigned long key, bool isDummy ) { mManagedKey = key; mIsDummy = isDummy; }
	bool			IsDummySkill() { return mIsDummy; }

	void			WaitCancel();

	unsigned int	GetSpecialType() { return mSpecialType; }

	void			EndCast();

protected:
	/// 캐스팅 처리
	void			ProcessCasting( unsigned long time );
	/// 발동 처리
	void			ProcessActivity( unsigned long time );
	/// 적용 처리
	void			ProcessApply( unsigned long time );

	void			ApplyDamage( const sDramaActionBase* pAct );
	void			ApplyHeal( const sDramaActionBase* pAct );

	virtual void	AddLoopEffect( cEffectSceneNode* pNode );
	virtual void	AddLoopSound( cSoundSceneNode* pNode );
	virtual void	AddLoopTrail( sTrailInfo* pInfo );

	void FadeOutAllLoopEffect();
	void FadeOutAllSoundEffect();
	void RemoveAllTrail();

	void			ChangeDramaState( eDRAMASTATE state );

	/// 발사체
	void AddBulletObject( sDramaActionBase* pBase );

	void BulletProcess( unsigned long time );

protected:
	struct sDamageInfo
	{
		tArray<void*> damageArr;
		sObject obj;

		unsigned int viewCnt;

		sDamageInfo() : viewCnt(0) {}

		~sDamageInfo()
		{
			if( damageArr.IsEmpty() == false )
			{
				for( unsigned int i = 0, end = damageArr.GetSize(); i < end; ++i )
				{
					delete (sTargetDamageDetail*)damageArr[i];
				}
				damageArr.Clear();
			}
		}
	};

	enum
	{
		ePRINT_NONE =0,
		ePRINT_DAMAGE,
		ePRINT_HEAL,
	};

	/// 인증 여부
	bool					mIsDummy;

	eVERIFY					mVerifyState;

	/// 캐스팅 연출 정보
	sDramaStateController	mCastingController;

	/// 발동 연출 정보
	sDramaStateController	mActivityController;

	/// 시전자 정보
	unsigned char			mUserWeapon;

	/// damage 적용 타겟
	unsigned char			mPrintType;
	tArray<void*>			mTargetObjArray;

	/// 발사체 관리
	tPointerList<void*>		mBulletList;
	tArray<void*>			mDelBulletArray;

	/// Apply Drama 관리
	tPointerList<void*>		mApplyDramaList;

	/// 동작 speed 변수
	float	mSpeedFactor;

	unsigned long mCastingTime;

	bool	mUsedAnimation;

	eSPECIAL	mSpecialType;
	NiPoint3 mFieldPos;

	//	bool	mUseFieldTarget;
};
//////////////////////////////////////////////////////////////////////////

///
class cInfluenceObject : public cDramaturgyObject
{
public:
	cInfluenceObject();
	virtual ~cInfluenceObject();

	void*			operator new( size_t n ); 
	void			operator delete( void* ptr );

	virtual bool Init( unsigned long uniqueIdx, unsigned long influenceIdx, bool isbuff );

	/// 적용에 대한 처리만 존재
	virtual void Process( unsigned long time );

	/// 실제로 process시작하는 타이밍 제어용
	void StartProcess( long restTime = -1, bool onlyLoopEffect = false );

	enum
	{
		eTick_HP,
		eTick_MP,
	};

	void SetTickInfo( unsigned char tickType, long value );

	inline void SetRestTime( unsigned long restTime ) { mRestTime = restTime; }
	inline unsigned long GetRestTime() { return mRestTime; }

	inline unsigned long	GetInfluenceIdx() { return mInfluenceIdx; }

	/// hero 효과인 경우 처리를 위해서
	inline bool IsHeroInfluence() { return mIsHeroInfluce; }
	inline bool IsAuraType() { return mInfluenceType == eINFLUENCETYPEDETAIL_AURA; }
	inline bool IsStatic() { return mStaticInfluence; }

protected:
	void ApplyHP( sDramaActionBase* pAct );
	void ApplyMP( sDramaActionBase* pAct );

protected:
	bool			mIsBuff;

	bool			mCheckBlink;
	bool			mIsHeroInfluce;
	bool			mOnlyShowLoopEffect;
	unsigned long	mBlinkTime;

	unsigned long	mStartTime;
	unsigned long	mLifeTime;

	unsigned long	mRestTime;

	bool			mStartTimeCheck;

	unsigned long	mInfluenceIdx;
	bool			mStaticInfluence;
	unsigned long	mInfluenceType;

	/// tick효과 에서 사용한다.
	bool			mTickInit; /// 최초연출에선 제외시키기위한..
	bool			mTickInfluence;
	unsigned char	mTickType;
	float			mTickValue;
};

class cApplyObject : public cDramaturgyObject
{
public:
	cApplyObject();
	virtual ~cApplyObject();

	void*			operator new( size_t n ); 
	void			operator delete( void* ptr );

	/// apply drama에서 정보를 획득한다.
	virtual bool Init( unsigned long managedKey, unsigned char weaponType, eAPPLYDRAMA_TYPE type );

	/// 효과번호로 Apply만을 가져온다.(item 효과 적용등에 사용)
	virtual bool Init( unsigned long managedKey, unsigned long influenceIdx );

	virtual void Process( unsigned long time );

private:

	enum
	{
		eINDEX_DRAMA,
		eINDEX_INFLUENCE,
	};

	unsigned char		mUserWeapon;

	/// drama or influence
	unsigned char		mIndexType;
	///
	unsigned long		mApplyIndex;

	//	eAPPLYDRAMA_TYPE	mApplyType;
	//	unsigned long		mInfluenceIdx;
};