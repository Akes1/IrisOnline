/* ==========================================================================
* 파    일 : ActionMove.h
* 목    적 : ai 이동
* 작 성 자 : 박경희
* 작 성 일 : 2006-09-20
* 주의사항 :
*===========================================================================*/

#pragma once
#include "action.h"
#include "RangeCheck.h"
#include "Monster_Common.h"

class cMonster;
class cPlayer;

const unsigned long MOVE_WAIT_TIME = 3000;
const unsigned long FIRSTMOVE_WAIT_TIME = 5000;

const unsigned long IDLE2_ANI_SEND_PER = 100;

class cActionMove :	public cAction
{

public:
	/// 생성소멸
	cActionMove(void);
	~cActionMove(void);

	/// 액션 객체 초기화
	void			ActionInit( cMonster* pMonster, unsigned long accumTime );

	/// 이동상태 프로세스
	eACTION_CHANGE	Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime );

private:

	/// 이동방법(속도) 선택
	eMOVE_SPEED_TYPE RandMoveSpeed( cMonster* pMonster, unsigned long *moveSpeed );

	/// 몬스터 검색 범위에 해당하는 플레이어들을 검사해 타겟을 잡는다.
	cPlayer*		TargetSelect( cMonster* pMonster, unsigned long seekRange );

private:

	/// 행동이 끝나고 일정시간 대기
	unsigned long	mWaitEndTime;

	cRangeCheck		mRangechk;

	/// 서버 부하 방지 지연시간
	unsigned long	mDelayTime;
	unsigned long	mDelayTime2;

	/// 리젠 후 일정시간 이동 정지 시간
	unsigned long	mRegenCantMoveEndTime;

	/// 자유이동 경로 설정 오류 카운트 
	/// ※ 몬스터가 자유이동 시점에서 너무 멀어 복귀 경로를 설정 못하는 경우
	unsigned long	mSetPathErrCnd;

	/// 대기동작2 발송
	bool			mIsIdle2Send;
	unsigned long	mIdle2SendTime;

	/// 이동 끝난 후 처음 프레임사용 변수
	bool			mFirstMoveEndCheck;

	/// 대기시간 종료후 처음 프레임 사용 변수
	bool			mFirstWaitEndCheck;
};
