/* ==========================================================================
* 파    일 : ActionPushPull.h
* 목    적 : ai 밀림/당겨짐
* 작 성 자 : PKH
* 작 성 일 : 2009-12-16
* 주의사항 :
*===========================================================================*/

#pragma once
#include "action.h"
#include "Monster_Common.h"

class cMonster;
class cPlayer;

class cActionPushPull :	public cAction
{

public:
	/// 생성소멸
	cActionPushPull(void);
	~cActionPushPull(void);

	/// 액션 객체 초기화
	void			ActionInit( cMonster* pMonster, unsigned long accumTime );

	/// 이동상태 프로세스
	eACTION_CHANGE	Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime );

private:
};
