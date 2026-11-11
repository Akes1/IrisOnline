/* ==========================================================================
* 파    일 : ActionDie.h
* 목    적 : 죽은경우 상태 처리
* 작 성 자 : 박경희
* 작 성 일 : 2006-10-02
* 주의사항 :
*===========================================================================*/

#pragma once
#include "action.h"

class cMonster;

class cActionDie :	public cAction
{
public:

	/// 생성소멸
	cActionDie(void);
	~cActionDie(void);

	/// 액션 객체 초기화
	void			ActionInit( cMonster* pMonster, unsigned long accumTime );

	/// 죽은경우 상태 처리
	eACTION_CHANGE	Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime );

private:

	bool			mFirstRun;
	unsigned long	mDieEndTime;
};
