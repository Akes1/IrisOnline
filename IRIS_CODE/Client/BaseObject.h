/* ====================================================================
* 파    일 :	BaseObject.h
* 목    적 :	관리를 위한 기본 게임 객체
* 작 성 자 :	정성대
* 작 성 일 :	2006.08.29
*
* 추가내용 :
*		추가날짜	작업자	추가내용
*		2006.08.29	정성대	생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once

#include "BaseObject_Common.h"

class cEffectSceneNode;
class cSoundSceneNode;
class cDynamicSceneNode;	
class cInfluenceObject;

struct sTrailInfo;

class cBaseObject
{
public:
	virtual ~cBaseObject();

public:
	/// 갱신부
	virtual void Update( unsigned long deltaTime, unsigned long accumTime ) = 0;

	virtual void UpdateAnimationSpeed(float val);
	virtual float GetAnimationSpeed();

	/// skillObject에서만 사용되는 함수
	virtual void UpdateSkillAnimation( unsigned int aniIdx, bool checkWeapon = false );

	virtual void ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker = 0 );

	/// 다른 스킬을 사용할수 있는 상태
	/// 현재 Animation을 종료하기 전까지 어떠한 변화도 줄 수 없는 경우

	inline bool IsReadyUseSkill() { return mReadyUseSkill; }
	virtual void SetReadyUseSkill( bool used ) 
	{
		if( mReadyUseSkill != used )
			mReadyUseSkill = used; 
	}

	inline virtual unsigned long	GetObjectID() = 0;

	float			GetXPos();
	float			GetYPos();
	float			GetZPos();
	NiPoint3		GetPos();
	void			SetPos( float x, float y, float z );
	void			SetPos( const NiPoint3& pos );
	void			SetWorldPos( const NiPoint3& pos );

	NiMatrix3		GetModelRot();
	float			GetModelScale();

	virtual void				SetState( unsigned char, bool) {}
	virtual unsigned int		GetState() { return 0; }

	/// 외부에서 객체의 부분 위치를 얻어 올때 사용한다.
	virtual bool	GetDummyPos( unsigned int dummyId, NiPoint3& pos );

	/// 070808 PKH 좌표보간
	void			FixPos( float x, float y, bool resetCamera = false );

	inline virtual cDynamicSceneNode* GetSceneNode() { return mpObjectSceneNode; }

	/// 객체의 그래픽 데이터에 대한 제거
	virtual void RemoveObjectSceneNode();

	/// 061127 PKH 지정하고 있는 타겟의 index로 object포인터를 얻어냄
	cBaseObject* GetTargetObject();
	inline const sObject* GetTargetInfo() { return &mTarget; }

	/// 061127 PKH 목표(공격,추적등)의 인덱스와 object종류를 설정한다.
	virtual void SetTargetObject( unsigned char type, unsigned long idx );

	/// 061211 PKH 지정하고 있는 타겟의 종류를 알아냄
	inline unsigned char GetTargetType() { return mTarget.type; }

	inline unsigned char GetObjectType() { return mObjectType; }

	/// 테스트 - 상태표시창 출력을위해 만듬
	virtual inline LPTSTR GetName() = 0;

	/// 061205 PKH 
	virtual unsigned long	GetHP() { return 0; }
	virtual unsigned long	GetMaxHP() { return 0; }

	virtual unsigned long	GetMP() { return 0; }
	virtual unsigned long	GetMaxMP() { return 0; }

	virtual void			HPChange( unsigned long /*objectHP*/, unsigned long /*objectMaxHP*/ ){}
	virtual void			HPChangeDie( unsigned long /*objectMaxHP*/, cBaseObject* /*pAttacker*/, unsigned char /*attackType*/ ){}
	virtual void			MPChange( unsigned long /*objectMP*/, unsigned long /*objectMaxMP*/ ){}

	/// 
	virtual unsigned char	GetLevel() { return 0; }

	/// 061213 wonju
	virtual unsigned int	SetLinkdObject( unsigned int /*num*/, char* /*nif*/ ) { return 0xffffffff; }
	virtual void			UnLinkdObject( unsigned int /*num*/ ) {}

	/// 이펙트를 객체에 링크 시키기.
	virtual cEffectSceneNode*	SetLinkdEffect( unsigned int num, const char* nif, NiTransform* trans, bool bLoop=false, bool bFollow=true );
	virtual cSoundSceneNode*	SetLinkdSound( unsigned long soundIdx, bool bLoop=false );
	virtual cSoundSceneNode*	SetStepSound( unsigned long soundIdx );
	virtual cEffectSceneNode*	SetLinkdDamage( unsigned int num, const char* nif, NiTransform* trans, bool IsMiss, bool zFalse );

	virtual sTrailInfo* CreateTrailEffect( cString str, unsigned long lifeTime, unsigned int link1, unsigned int link2, float lenPer, float factor );
	void DeleteTrailEffect( sTrailInfo* pInfo );

	///
	virtual void AddBuff( unsigned long uniqueIdx, cInfluenceObject* p );
	virtual void DelBuff( unsigned long uniqueIdx );

	virtual void AddDeBuff( unsigned long uniqueIdx, cInfluenceObject* p );
	virtual void DelDeBuff( unsigned long uniqueIdx );

	virtual void BlinkBuffIcon( unsigned long /*uniqueIdx*/, unsigned long /*influenceIdx*/ ) {}
	virtual void BlinkDeBuffIcon( unsigned long /*uniqueIdx*/, unsigned long /*influenceIdx*/ ) {}

	bool IsBuffHave( unsigned long influenceIdx );
	bool IsHaveApplyBuffType( unsigned short applyValueType );
	bool IsHaveApplyDeBuffType( unsigned short applyValueType );

	/// 071023 PKH 버프 디버프 참조
	tPointerHashMap<unsigned long, void*>* GetBufMap()	{ return &mBuffMap; }
	tPointerHashMap<unsigned long, void*>* GetDeBufMap()	{ return &mDeBuffMap; }

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	virtual float			GetFixedObjectSize()	{ return 0.0f; }
	float GetStatureValue();

	///
	void SetOverFlag( bool over );

	virtual bool IsDie() { return false; }
	virtual void FadeOutObject();

	virtual void SetSkillPushPull( float /*posX*/, float /*posY*/ ) { assert(NULL); }

protected:
	virtual void ResetState() {}

	// virtual void ClearAllBuff();
	// virtual void ClearAllDeBuff();
	/// 090302 PKH 객체 사망시 안지워지는 효과 처리 추가
	virtual void ClearAllBuff( bool delDieInf );
	virtual void ClearAllDeBuff(  bool delDieInf );
	
protected:
	cBaseObject( unsigned char type );

	virtual void		Interpret( float /*fTime*/ ) {}

	enum eANIEVENT
	{
		eANIEVENT_GATHER_MINING,

		eANIEVENT_RUN_RIGHT,
		eANIEVENT_RUN_LEFT,

		eANIEVENT_JUMP_START,
		eANIEVENT_JUMP_END,

		eANIEVENT_MAX,
	};

	/// type 
	unsigned char		mObjectType;

	/// 그래픽 데이터 객체 참조
	cDynamicSceneNode*	mpObjectSceneNode;

	/// 061127 PKH 타겟 인덱스값 지정
	sObject				mTarget;

	bool				mReadyUseSkill;

	/// 적용 효과(버프/디버프)에 대한 참조
	typedef tPointerHashMap<unsigned long, void*>	cInfluenceMap;
	cInfluenceMap		mBuffMap;
	cInfluenceMap		mDeBuffMap;


	/// 
//	float				mStatureValue;

	unsigned long	mIdleTime;
	float mRevisionEventTime[eANIEVENT_MAX];
};

