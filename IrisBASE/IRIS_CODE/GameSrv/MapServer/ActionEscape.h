/* ==========================================================================
* 파    일 : ActionEscape.h
* 목    적 : 몬스터 도망 상태 클래스
* 작 성 자 : 박경희
* 작 성 일 : 2006-10-11
* 주의사항 :
*===========================================================================*/

#pragma once
#include "action.h"
#include "NiPoint2.h"

class cMonster;

const long ESCAPE_MAXIMUM_TIME = 10000;
const long ESCAPE_MINIMUM_TIME = 3000;
const long ESCAPE_CHANGE_TIME = 5000;

class cActionEscape : public cAction
{

public:

	/// 생성 소멸
	cActionEscape(void);
	~cActionEscape(void);

	/// 액션 객체 초기화
	void			ActionInit( cMonster* pMonster, unsigned long accumTime );

	/// 도망상태 구현부
	eACTION_CHANGE	Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime );

private:

	/// 도망 목적지 선정
	bool			EscapeTargetPos( cMonster* pMonster, unsigned long escapeMinRange, unsigned long escapeMaxRange, float centerX, float centerY );

private:

	/// 도망 시작 시간
	unsigned long	mEscapeStartTime;
	/// 중간 경로 변경
	bool			mCourseChange;
	/// 경로 설정 여부
	bool			mIsSetPath;

};
