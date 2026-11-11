/* =====================================================================
* 파    일 :	PassiveSkillObject.h , PassiveSkillObject.cpp
* 목    적 :	사용중인 Passive 스킬 객체
* 작 성 자 :	정원주
* 작 성 일 :    2006년 9월 28일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* =======================================================================*/

#pragma once

#include "SkillObject.h"

class cPassiveSkillObject : public cSkillObject
{
private:

public:

	void	Process();

	static void*	operator new( size_t n ); 
	static void		operator delete( void* ptr, size_t n );

	cPassiveSkillObject();
	~cPassiveSkillObject();
};