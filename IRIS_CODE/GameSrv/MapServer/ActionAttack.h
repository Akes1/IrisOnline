/* ==========================================================================
* 파    일 : ActionAttack.h
* 목    적 : 전투상태 구현
* 작 성 자 : 박경희
* 작 성 일 : 2006-10-10
* 주의사항 :
*===========================================================================*/

#pragma once
#include "action.h"
#include "RangeCheck.h"

class cMonster;

class cActionAttack : public cAction
{
private:


public:

	/// 생성 소멸
	cActionAttack(void);
	~cActionAttack(void);

	/// 액션 객체 초기화
	void			ActionInit( cMonster* pMonster, unsigned long accumTime );

	/// 전투상태 구현부
	eACTION_CHANGE	Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime );

private:

	cRangeCheck		mRangechk;
};
