/* =====================================================================
* 파    일 :	SkillExecuter.h , SkillExecuter.cpp
* 목    적 :	스킬을 사용목적에 맞게 실행하도록 한다.
* 작 성 자 :	정원주
* 작 성 일 :    2007년 3월 26일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 : 키가 눌렸을 경우나 마우스가 클릭, 더블클릭 되었을 경우에만 사용된다.
*			 네크워크 메시지 라든지, 후처리 같은 내부적인 동작에 관여하지 않는다.
*		 	 스킬의 사용에 대한 각종 제한 체크, 상태 체크 등이 이루어진다.
* =======================================================================*/

#pragma once
#include "BaseObject_Common.h"
#include "skill_common.h"

struct sTransMonSkillInfo
{
	unsigned int mUsePer;
	float mRange;
};

enum
{
	eCharge_None = 0,
	eCharge_Target,
	eCharge_Field,
};

struct sChargeInfo
{
	unsigned char	mChargeType;

	unsigned long	mSkillIdx;
	char	mSkillStep;

	float			mSkillRange;
};

class cSkillObject;

class cAttackPattern
{
public:
	cAttackPattern()
	{
		::memset( mPattern, -1, sizeof(int)*3*6 );

		/// 패턴 생성
		mPattern[0][0] = 0;mPattern[0][1] = 1;mPattern[0][2] = 2;mPattern[0][3] = 0;mPattern[0][4] = 2;mPattern[0][5] = 1;
		mPattern[1][0] = 1;mPattern[1][1] = 0;mPattern[1][2] = 2;mPattern[1][3] = 1;mPattern[1][4] = 2;mPattern[1][5] = 0;
		mPattern[2][0] = 2;mPattern[2][1] = 1;mPattern[2][2] = 0;mPattern[2][3] = 1;mPattern[1][4] = 0;mPattern[1][5] = 2;

		mPatternIdx = 0;
		mProgressIdx = 0;
		mOldAttackIdx = -1;
	}
	~cAttackPattern() {}

	unsigned int GetAttackFlag();
	void ResetPatternIdx() { mPatternIdx = -1; }

protected:

	int	mPattern[3][6];

	/// 패턴 번호
	int	mPatternIdx;
	int mOldAttackIdx;

	/// 진행 단계
	unsigned int	mProgressIdx;
};


class cSkillExecuter
{
public:
	static cSkillExecuter* mpExec;
public:
	cSkillExecuter();
	~cSkillExecuter();

	void Close();

	bool AutoAttack( cBaseObject* pTarget );

	/// 기본 공격 발동
	bool ExecuteAttack( cBaseObject* pTarget );

	/// 스킬 발동
	void ExecuteSkill( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pTarget = 0 );

	/// 
	void ExecuteSkill( unsigned long skillIdx, unsigned char skillStep, NiPoint3 fieldPos );

	void ResetAttackFlag() { mAttackPattern.ResetPatternIdx(); }

	/// 장착 해제
	void ChargeOff( unsigned long skillIdx, unsigned char skillStep );

	sChargeInfo		GetChargeSkill();
	sChargeInfo		GetCurrentChargeSkill() { return mChargeSkill; }

	bool IsSkillCharge();
	bool IsFieldSkillCharge() { return mChargeSkill.mChargeType == eCharge_Field; }

	bool IsPossibleUseSkill( unsigned long skillIdx, unsigned char skillStep );
	bool IsPossibleUseSkillTarget( unsigned long skillIdx, cBaseObject* pTarget );
	bool IsRushUseSkill( unsigned long skillIdx, unsigned char skillStep );

	void ReCharge() { ChargeOn( NORMAL_ATTACK_SKILL, 0 ); }

	/// 현재 사용중 스킬의 상태를 사용하기 위한 인터페이스
	inline cSkillObject* GetCurrentHeroSkillObject() { return mpCurrentHeroSkillObject; }
	inline void SetCurrentHeroSkillObject( cSkillObject* p ) { mpCurrentHeroSkillObject = p; }

	void SetMonsterSkillInfo( unsigned long monsterClassIdx );

	bool IsPossibleApplyValueType( unsigned long skillIdx, cBaseObject* pTarget );

protected:

	/// 스킬 장착
	void ChargeOn( unsigned long skillIdx, unsigned char skillstep );
	void FieldSkillOn( unsigned long skillIdx, unsigned char step );

	///
	unsigned long GenerateTransMonsAttackIdx();
	void ChargeOn_TransMon( unsigned long skillIdx );
	void ChargeOff_TransMon( unsigned long skillIdx );

	bool IsPossibleUseSkill_TransMon( unsigned long skillIdx );
	bool IsPossibleUseSkillTarget_TransMon( unsigned long skillIdx, cBaseObject* pTarget );

	bool AutoAttack_TransMon( unsigned long skillIdx,cBaseObject* pTarget );
	bool ExecuteAttack_TransMon( unsigned long skillIdx, cBaseObject* pTarget );
	bool ExecuteSkill_TransMon( unsigned long skillIdx, cBaseObject* pTarget );
	bool ExecuteSkill_TransMon( unsigned long skillIdx, NiPoint3 applyPos );

	void FieldSkillOn_TransMon( unsigned long monsterClassIdx, unsigned long skillIdx );

	void ExecuteAutoPos( unsigned long skillIdx, unsigned char step );
	bool ExecuteAutoPos_TransMon( unsigned long skillIdx );

protected:

	/// 현재 사용중 스킬 객체
	cSkillObject*	mpCurrentHeroSkillObject;

	/// 바닥 기준 범위 스킬 영역 설정
//	bool			mIsFieldCheck;
//	bool			mIsCharge;

//	unsigned long	mChargeSkillIdx;
//	float			mSkillRange;

	sChargeInfo		mChargeSkill;

	/// 이전 공격대상과 현재 타겟간의 비교를 위한 변수
	sObject			mAttackTarget;

	/// 기본 공격의 패턴
	cAttackPattern	mAttackPattern;

	sTransMonSkillInfo mMonsterAttackInfo[eMONSTERATTACK_MAX];
	unsigned long mTotalMonsterAttackPer;
};

#define SKILLEXECUTER cSkillExecuter::mpExec
