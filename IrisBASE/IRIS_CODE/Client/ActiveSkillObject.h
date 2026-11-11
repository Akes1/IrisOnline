/* =====================================================================
* 파    일 :	ActiveSkillObject.h , ActiveSkillObject.cpp
* 목    적 :	사용중인 Active 스킬 객체
* 작 성 자 :	정원주
* 작 성 일 :    2006년 9월 28일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* =======================================================================*/

#pragma once

/*
#include "BaseObject_Common.h"
#include "SkillObject.h"

class cBaseObject;


class cActiveSkillObject : public cSkillObject
{

public:
	void			SetActiveTime(float totallifetime, unsigned int ratio);
	void			SetTarget(sTarget target) { mTarget = target; }
	void			SetLinkName(cString linkname) { mLinkName = linkname; }

	bool			GetSkillLife() { return mbLife; }

	void			Process();

	void*			operator new( size_t n ); 
	void			operator delete( void* ptr );

	cActiveSkillObject();
	~cActiveSkillObject();

private:
	float			mTotallifetime;
	float			mCurrenttime;
	unsigned	int	mRatio;
	bool			mbLife;

	sTarget			mTarget;

	cString			mLinkName;

};
*/