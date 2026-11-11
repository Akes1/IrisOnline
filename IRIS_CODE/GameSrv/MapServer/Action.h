/* ==========================================================================
* 파    일 : Action.h
* 목    적 : 모든 액션의 베이스 - 몬스터에서 이 액션을 수행 할 경우
*			 현재 할당되어 있는 자식 액션이 수행됨
* 작 성 자 : 박경희
* 작 성 일 : 2006-09-20
* 주의사항 :
*===========================================================================*/

#pragma once
#include "stdafx.h"

class cMonster;

enum eACTION_ID;
enum eACTION_CHANGE;

class cAction
{
public:

	/// 생성소멸
	cAction(eACTION_ID actid);
	virtual ~cAction(void) {};

	/// 액션 객체 초기화
	virtual void	ActionInit( cMonster* pMonster, unsigned long accumTime );

	/// 현재 액션 수행 함수 - 리턴값에 따라 ai매니저에서 상태 전이
	virtual eACTION_CHANGE Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime ) = 0;

	/// 현재 수행중인 액션의 값을 가져옴
	eACTION_ID		GetActionID();

protected:

	/// 현재 액션 상태값
	eACTION_ID		mActionid;

};
