/* ====================================================================
* 파    일 :	
* 목    적 :	
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
#include "../Common/CommonNetworkMsgStruct.h"
#include "BaseObject_Common.h"

enum eSKILLSTATE;

//class cSkillObject;
class cInfluenceObject;

const float AGGRO_NONE_RATE = 1.0f;
const float AGGRO_DAMAGE_RATE = 1.0f;
const float AGGRO_HEAL_RATE = 0.5f;
const float AGGRO_POTION_RATE = 0.5f;

enum eTAKEDAMAGE_TYPE
{
	/// player
	eTAKEDAMAGETYPE_NONE = 0,
	eTAKEDAMAGETYPE_DAMAGE,
	eTAKEDAMAGETYPE_HEAL,
	eTAKEDAMAGETYPE_POTION,
	eTAKEDAMAGETYPE_DAMAGE_SETTARGET,
	eTAKEDAMAGETYPE_NONETARGET,

	/// monster
	eTAKEDAMAGETYPE_ADDPLAYER,
};

class cBaseObject
{
public:
	cBaseObject( unsigned char objectType );
	virtual ~cBaseObject();

	bool operator == (cBaseObject& baseObject);
	bool operator != (cBaseObject& baseObject);

	sObject                 GetObject     ( ) { return mObject; }

	inline unsigned long	GetObjectID   ( ) { return mObject.index; }
	inline void				SetObjectID   ( unsigned long objectIndex ) { mObject.index = objectIndex; }

	inline unsigned char	GetObjectType ( ) { return mObject.type; }

	inline float			GetXPos       ( ) { return mObjectPos.x; }
	inline float			GetYPos       ( ) { return mObjectPos.y; }
	inline NiPoint2			GetPos        ( ) { return mObjectPos;   }
	inline virtual void		SetPos        ( float x, float y );

	/// 061127 PKH 지정하고 있는 타겟의 index로 object포인터를 얻어냄
	virtual cBaseObject*	GetTarget ( );
	/// 061201 PKH 목표(공격,추적등)의 인덱스와 object종류를 설정한다.
	virtual bool			SetTarget ( unsigned char kind, unsigned long idx );

	/// 061130 PKH 몬스터 생존 유무 셋팅
	inline virtual bool		GetStateDie( );
	/// 061201 PKH 오프젝트 생존유무 셋팅
	inline virtual void		SetStateDie( bool die ) { die; }

	virtual unsigned int	GetRaceGender ( ) { return (unsigned int)(0xffffffff); }
	virtual unsigned short	GetMoveSpeed  ( ) { return 0; }

	/// 070807 PKH HP감소
	virtual unsigned long	HPDamage( sObject /*attacker*/, unsigned long /*damage*/, bool* /*die*/, bool /*msgSend*/, bool cri = false ) { cri; return 0; }
	virtual void			HPUse( unsigned long /*useValue*/, bool /*msgSend*/ ) {}
	/// 070807 PKH 순식간에 HP증가
	virtual void			HPHeal( unsigned long /*hp*/, bool /*msgSend*/, sObject /*healer*/, long /*distressPoint*/, eTAKEDAMAGE_TYPE /*type*/ ) {}

	/// 070807 PKH mp처리
	virtual void			MPDamage ( unsigned long /*damage*/, bool /*msgSend*/ ) {}
	virtual void			MPUse( unsigned long /*useValue*/, bool /*msgSend*/ ) {}
	virtual void			MPHeal   ( unsigned long /*heal*/, bool /*msgSend*/   ) {}

	/// 현재 HP & MP
	virtual unsigned long	GetHP() { return 0; }
	virtual unsigned long	GetMP() { return 0; }
	virtual unsigned long	GetMaxHP() { return 0; }
	virtual unsigned long	GetMaxMP() { return 0; }

	virtual void			SetOriHPMax( unsigned long /*maxHP*/ ) {}
	virtual unsigned long	GetOriHPMax() { return 0; }
	virtual void			SetOriMPMax( unsigned long /*maxMP*/ ) {}
	virtual unsigned long	GetOriMPMax() { return 0; }

	void					SetMapNumber( unsigned short mapNumber ) { mMapNumber = mapNumber; }
	virtual unsigned short	GetMapNumber( ) { return mMapNumber; }

	virtual void			AddTakeDamage( sObject /*attacker*/, unsigned long /*damage*/, long /*distressPoint*/, eTAKEDAMAGE_TYPE /*type*/ ) { assert(NULL); }

	/// 효과 
	/// virtual bool				     AddInfluence( unsigned long /*uniqueIdx*/, unsigned long /*influenceClassIdx*/ ) { assert(NULL); return false; }
	virtual bool			AddInfluence( cInfluenceObject* /*pInf*/ )	{ assert(NULL); return false; }
	virtual tHashSet<unsigned long>* GetInfluenceSet() { assert(NULL); return NULL; }
	virtual void				     ReleaseInfluenceSet( bool /*logOut*/ ) { assert(NULL); } 
	virtual void				     DeleteInfluence( unsigned long /*influenceIdx*/ ) { assert(NULL); }

	virtual void			StatusCalc()	{ assert(NULL); }

	virtual bool			IsParentEqual( unsigned long /*parentUniqueIdx*/ ) { return false; }
	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	virtual float			GetFixedObjectSize()	{ return 0.0f; }

	/// 상태 이상
	virtual void			SetStateOddity( long* /*oddity*/ )	{ assert(NULL); }
	virtual unsigned long	GetStateOddity( eODDITYTYPE /*oddityType*/ )	{ assert(NULL); return 0; }

	virtual bool			IsCantSkill( unsigned long /*attributeType*/ )	{ return true; }
	virtual bool			IsCantMove()	{ return true; }

	virtual void			TargetChange( sObject /*attacker*/, long /*distressPoint*/ ) {}

	virtual float			Height();

	virtual int             GetGridPos( ) { return mGridPos; }
	virtual void            SetGridPos( int pos ) { mGridPos = pos; }

	virtual unsigned char	GetLevel( ) { return 1; }

	virtual sObject			GetSkillLastAttack()	{ assert(NULL); return mObject; }
	virtual unsigned char	GetSkillLastKind()	{ assert(NULL); return (unsigned char)0; }

	virtual bool			AttackSuccess( sObject /*target*/, unsigned long /*aType*/, short /*accuracyValue*/,
		unsigned long /*instantSuccess*/, unsigned long /*instantSuccessPer*/ ) { assert(NULL); return 0;}
	virtual bool			CriticalSuccess( sObject /*target*/, unsigned long /*aType*/, unsigned short /*criticalValue*/,
		unsigned long /*instantCri*/, unsigned long /*instantCriPer*/ ) { assert(NULL); return 0;}
	virtual unsigned long	DamageCalc( sObject /*target*/, unsigned long /*skillIdx*/, unsigned long /*aType*/, unsigned long /*rType*/, 
		unsigned long /*skillDamage*/, unsigned long /*skillDamagePer*/, bool /*isIgnoreDefence*/, float /*addTotalDamage*/, float /*addPerTotalDamage*/ ) { assert(NULL); return 0; }
	/// 다른 계산과 다르게 도트데미지는 공격자가없어도 계산을 해야되기때문에 기준객체가 타겟객체다!!
	virtual unsigned long	DotDamageCalc( unsigned char /*attackerType*/, unsigned char /*attackerLv*/, unsigned long /*attackMin*/, 
		unsigned long /*attackMax*/, unsigned char /*dotType*/, unsigned long /*skillDamage*/, 
		unsigned long /*skillDamagePer*/, unsigned long /*totalTime*/, unsigned long /*dotTime*/ )  { assert(NULL); return 0; }
	virtual unsigned long	HealCalc(  sObject /*target*/, long /*skillHealPlus*/, long /*skillHealPer*/, bool /*applyHP*/  )  { assert(NULL); return 0; }
	virtual unsigned long	DotHealCalc( sObject /*target*/, long /*infHealPlus*/, long /*infHealPer*/,
		unsigned long /*heal*/, unsigned long /*dotTotalTime*/, unsigned long /*dotTime*/ )	{ assert(NULL); return 0; }
	virtual unsigned long	ConditionOdd( sObject /*target*/, long /*influenceRate*/, unsigned long /*skillClassIdx*/ ) { assert(NULL); return 0; }
	virtual float			GetDirection()	{ assert(NULL); return 0; }

	virtual unsigned long	CalcChangeMPDamage( unsigned char /*attributeType*/, unsigned long /*damage*/, bool /*applyHP*/ )	{ assert(NULL); return 0; }
	virtual unsigned long	CalcGuard( unsigned char /*attributeType*/, unsigned long /*damage*/ )	{ assert(NULL); return 0; }

	virtual void			SetSkillPushPull( unsigned short /*type*/, cBaseObject* /*pAttacker*/, unsigned long /*range*/ )	{ assert(NULL); }
	virtual bool			IsSkillPushPull()	{ assert(NULL); return false; } 

protected:
	sObject					mObject;
	NiPoint2				mObjectPos;	///

	sObject					mTarget;

	unsigned short			mMapNumber;
	int                     mGridPos;
};

inline bool cBaseObject::operator == (cBaseObject& baseObject)
{
	return (mObject.index == baseObject.GetObjectID( ));
}

inline bool cBaseObject::operator != (cBaseObject& baseObject)
{
	return !operator==(baseObject);
}
