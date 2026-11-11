/* =====================================================================
* 파    일 :	ActiveSkillObject.h , ActiveSkillObject.cpp
* 목    적 :	사용중인 Active 스킬 객체
* 작 성 자 :	정원주
* 작 성 일 :    2006년 9월 28일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* =======================================================================*/

#pragma once

#include "SkillObject.h"
#include "NiPoint3.h"

enum eSKILLSTATE;
class cBaseObject;

class cActiveSkillObject : public cSkillObject
{
private:
	
	void			CastingProcess();
	void			SkillActProcess();
	void			SkillDamageProcess();

	void			NonCastingSkillStart();
	void			TargetSkillStart();

public:

	void			Process();

	void			SetTarget( sTarget target )	{ mTarget = target; }
	void			SetTarget( eOBJECTTYPE targetKind, unsigned long targetIdx ) 
					{ mTarget.type = targetKind; mTarget.index = targetIdx; }
	sTarget			GetTarget() { return mTarget; }

	void			SetGroundSkillRange(float range) { mGroundSkillRange = range; }
	void			SetGroundSkillPos(NiPoint3 pos) { mGroundSkillPos = pos; }

	bool			GetSkillLife() { return mbSkillLife; }

	/// test 결과 실제 공격애니메이션에서 0.2초 뺀 시간이 가장 적당하다.
	void			SetSkillActTime(float castingtime, float actlifetime, float cooltime, unsigned int DamagePersent );

	static void*	operator new( size_t n ); 
	static void		operator delete( void* ptr, size_t n );
	
	cActiveSkillObject();
	~cActiveSkillObject();

private:
	
	///	Target의 객체
	sTarget			mTarget;

	/// 스킬이 적용될 범위 -> 범위 스킬중 땅에 적용되는 스킬에만 적용된다.
	float			mGroundSkillRange;
	NiPoint3		mGroundSkillPos;

	///	생명주기 체크 변수
	bool			mbSkillLife;

	///	생명주기 시간 관리
	float			mCurrentActTime;		/// SetSkillActTime() 이 호출된 시간 - 스킬발동시간
	float			mCastingTime;
	float			mActLifeTime;
	float			mCoolTime;
	float			mDamageCalcuTime;
	unsigned int	mDamagePersent;
	bool			mbDamageSend;
};
