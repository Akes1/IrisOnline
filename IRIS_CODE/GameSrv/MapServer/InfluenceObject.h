/* ==========================================================================
* 파    일 : InfluenceObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-08-21
* 주의사항 :
*===========================================================================*/
#pragma once

#include "BaseObject_Common.h"
#include "RangeCheck.h"

struct sInfluence;

enum eSHOTTYPE;
enum eAURATYPE;

const unsigned long AURA_CHECK_TIME = 1000;
const unsigned long AURAMP_CHECK_TIME = 5000;

class cInfluenceObject
{

public:

	cInfluenceObject();
	virtual ~cInfluenceObject();

	/// Skillmanager의 메모리풀에서 스킬 공간을 할당받음
	void* operator new( size_t n ); 
	/// 메모리풀에서 할당 받은 효과 객체를을 제거해줌
	void operator delete( void* ptr, size_t n );

	bool				Init( sObject attacker, sObject target, unsigned long uniqueIdx, unsigned long infClassIdx,
		unsigned long skillClassIdx, eSWITCH_TYPE infType, unsigned long parentIdx, unsigned long dotTotalTime, unsigned long* plusTime );
	void				AddModeType( unsigned char applyType, unsigned short range );

	bool				Process( unsigned long elapsedTime, unsigned long accumTime );

	bool				IsSavePlayerInfluence( bool onlyCash );
	void				SavePlayerInfluence( sInfluence* pInfluence );
	bool				RestorePlayerInfluence( unsigned long playerIdx, unsigned long uniqueIdx, sInfluence* pInfluence );


	/// 호출 시점부터 타이머를 가동
	void				ProcessStart();
	void				ProcessStop();

	bool				SetAllocDbIdx( unsigned long dbIdx );
	bool				IsDbIdx()	{ return mDBIdx == 0 ? false : true; }

	unsigned long		GetUniqueIdx()	{ return mUniqueIdx; }
	unsigned long		GetInfluenceClassIdx()	{ return mInfluenceClassIdx; }

	sObject				GetTarget()	{ return mTarget; }
	sObject				GetAttacker()	{ return mAttacker; }
	eSWITCH_TYPE		GetSwitchType()	{ return mSwitchType; }
	unsigned long		GetRarentIdx()	{ return mParentIdx; }
	unsigned long		GetSkillIdx()	{ return mSkillIdx; }

	unsigned char	    GetDeleteType();
	bool				IsRealTime()	{ return mIsRealTime; }
	unsigned long		GetInfluenceRestTime()	{ return mInfGameRestTime; }
	unsigned long		GetInfluenceEndTime()	{ return mInfluenceCreationTime + mInfGameRestTime; }
	TIMESTAMP_STRUCT	GetInfRealEndTime()	{ return mInfRealEndTime; }
	
	float				GetRange()	{ return mCopyRange; }
	bool				IsSleep()		{ return mIsSleep; }
	bool				IsTargetNoApply()	{ return mCopyApplyType == eAPPLYTYPE_ENEMY; }

	void				NoSendDelMsg()	{ mSendDelMsg = false; }
	bool				IsSendDelMsg()	{ return mSendDelMsg; }
	void				NoStatusCalc()	{ mDelStatusCalc = false; }
	bool				IsDelStatusCalc()	{ return mDelStatusCalc; }

	void				DelReady()	{ mIsDelReady = true; }
	bool				IsDelReady()	{ return mIsDelReady; }

	bool				CalcLeftGuard( unsigned char attributeType, unsigned long* damage );
	bool				CalcCashExpGuard();

	/// 토템에 있는 타겟 목록에서 제거
	void				TotemSetEraseTarget();

	unsigned char		GetGuardType()	{ return mGuardType; }
	unsigned short		GetLeftGuardValue()	{ return mLeftGuardValue; }
	unsigned char		GetLeftGuardCnt()	{ return mLeftGuardCnt; }

	unsigned char		GetChangeMPType()	{ return mChangeMPType; }
	unsigned char		GetChangeMPValue()	{ return mChangeMPValue; }

private:

	/// 오라를 통한 효과 복사
	void				CopyAuraInfluence();
	/// 복사된 오라(자식) 삭제 체크
	bool				DeleteChildAuraInfluence();

	/// 토템이 만들어낸 자식(오라) 삭제 체크
	bool				CheckTotemInfluence();

	/// 효과 객체 생성후 일정시간 주기로 효과 적용
	void				DotInfluence( eSTATUS_PLUS influenceIndex, long infValue, unsigned long totalTime, unsigned long dotTime );

private:

	//////////////////////////////////////////////////////////////////////////
	/// DB 기록부

	/// 효과 DB 고유IDX
	unsigned long		mDBIdx;
	/// 타겟
	sObject				mTarget;
	/// 효과 번호
	unsigned long		mInfluenceClassIdx;
	/// 공격자
	sObject				mAttacker;
	/// 효과 생성 시점 ( 절대 시간 )
	unsigned long		mInfluenceCreationTime;
	/// 시간 설정 구분
	bool				mIsRealTime;
	/// 남은 지속 시간 (생성 시점기준 이후로 부터)
	unsigned long		mInfGameRestTime;
	/// 남은 지속 시간 ( 실 시간 )
	TIMESTAMP_STRUCT	mInfRealEndTime;
	/// 지속효과 시간 ( 절대 시간 )
	/// ※ 0이 아닌 경우 다음번 데미지 계산을 해야 하는 시간
	unsigned long		mTime1;
	unsigned long		mTime2;
	/// 효과 값
	long				mInfluenceValue1;
	long				mInfluenceValue2;
	/// 공격 속성
	eATTRIBUTETYPE		mAttriButeType;
	unsigned char		mAttackerLv;
	unsigned long		mAttackerDamageMin;
	unsigned long		mAttackerDamageMax;
	unsigned long		mHeal;

	unsigned char		mLeftGuardCnt;
	unsigned short		mLeftGuardValue;

	//////////////////////////////////////////////////////////////////////////
	/// db 미 기록부

	/// 효과 고유 인덱스
	unsigned long		mUniqueIdx;
	/// 스킬 인덱스
	unsigned long		mSkillIdx;
	/// 프로세스 진행/정지 상태
	bool				mProcessStart;

	/// 계산수식을 사용하는 결과치 예비 수식값
	//long				mCalcTempValue1;
	//long				mCalcTempValue2;
	long				mDotTotalTime;


	/// 삭제시 메세지 발송여부 - 타겟 객체 죽음
    bool				mSendDelMsg;		

	/// 삭제후 스텟계산을 하는지 여부
	bool				mDelStatusCalc;

	/// 삭제 대기 상태
	bool				mIsDelReady;	

	cRangeCheck			mRangeCheck;

	/// 토템으로 생성된 효과
//	bool				mIsTotem;

	bool				mIsSleep;
	/// 어그로
	long				mDistressPoint;

	unsigned char		mGuardType;		// eINFGUARD_TYPE

	unsigned char		mChangeMPType;	/// eINFCHANGEMP_TYPE
	unsigned char		mChangeMPValue;

	// 일정시간 체크
	unsigned long		mDotValueAcuumTime;

	/*--------------- 스위치 on/off 형태의 효과 ---------------*/
	/// 효과 타입
	eSWITCH_TYPE		mSwitchType;
	/// 부모 효과의 uniqueidx
	unsigned long		mParentIdx;
	/// 범위
	unsigned short		mCopyRange;
	/// 체크시간
	unsigned long		mCopyCheckTime;
	/// 적용 타입
	eAPPLYTYPE			mCopyApplyType;
};
