/* ==========================================================================
* 파    일 : ActionIdle.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2009-01-30
* 주의사항 :
*===========================================================================*/
#pragma once
#include "action.h"
#include "NiPoint2.h"

class cMonster;

class cActionIdle : public cAction
{

public:

	/// 생성 소멸
	cActionIdle(void);
	~cActionIdle(void);

	/// 액션 객체 초기화
	void			ActionInit( cMonster* pMonster, unsigned long accumTime );

	/// 도망상태 구현부
	eACTION_CHANGE	Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime );

};
