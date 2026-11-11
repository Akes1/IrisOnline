/* ==========================================================================
* 파    일 : ActionFollow.h
* 목    적 : 추적(쫓아가기) 상태
* 작 성 자 : 박경희
* 작 성 일 : 2006-11-16
* 주의사항 :
*===========================================================================*/
#pragma once

#include "action.h"
#include "RangeCheck.h"

class cMonster;

class cActionFollow : public cAction
{
public:
	/// 생성소멸
	cActionFollow(void);
	~cActionFollow(void);

	/// 액션 객체 초기화
	void			ActionInit( cMonster* pMonster, unsigned long accumTime );
	
	/// 추적상태 구현부
	eACTION_CHANGE	Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime );

	/// 몬스터 검색 범위에 해당하는 플레이어들을 검사해 타겟을 잡는다.
	cPlayer*		TargetSelect( cMonster* pMonster, unsigned long seekRange );

private:

	/// 추적 시작시 타겟의 좌표 - 타겟 이동 일정거리마다 위치 갱신함
	NiPoint2		mFollowTargetPos;

	cRangeCheck		mRangechk;

	bool			mFirstFollowMove;

	unsigned long	mDelayTime;

	/// 빠른몬스터가 플레이어 추적시 길찾기 끝난후 다시 길찾기 지연시간
	unsigned long	mRePathTime;

	/// 플레이어가 못가는 지역에 있는지 카운트
	unsigned char	mCantFollowPos;

	/// 스킬 선택 간격
	unsigned long	mNextSelectSkill;

	/// 추적 초기 셋팅시 몬스터 대사 발송 체크
	bool			mIsTalk;
};
