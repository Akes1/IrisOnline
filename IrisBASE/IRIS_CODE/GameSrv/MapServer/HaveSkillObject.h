/* ==========================================================================
* 파    일 : HaveSkillObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-09-14
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Player_Common.h"

struct sInfluence;

class cHaveSkillObject
{

public:

	cHaveSkillObject();
	virtual ~cHaveSkillObject();

	/// Skillmanager의 메모리풀에서 스킬 공간을 할당받음
	void* operator new( size_t n ); 
	/// 메모리풀에서 할당 받은 효과 객체를을 제거해줌
	void operator delete( void* ptr, size_t n );

	void				Init( unsigned long skillIdx, unsigned char skillStep, unsigned long restCoolTime );
	void				RestoreInit( unsigned long skillIdx, unsigned char skillStep, unsigned long restCoolTime );
	void				Process( unsigned long elapsedTime, unsigned long accumTime );
	bool				StepUp( unsigned char skillStep, unsigned long restCoolTime );
	void				StepUpByCheat( unsigned char skillStep, unsigned long restCoolTime );
	void				StepDown()	{ --mSkillStep; }
	void				UpdateCoolTime( unsigned long restCoolTime, float speedFactor );
	void				ClearRestTime(){  mRestTime = 0; }


	unsigned long		GetSkillIdx()	{ return mSkillIdx; }
	unsigned char		GetSkillStep()	{ return mSkillStep; }
	unsigned long		GetEndCoolTime();
	unsigned long		GetRestTime();

	/// 적용 여부와 상관없는 DB저장용 쿨타임
	unsigned long		DbSaveDataCoolTime()	{ return mCreationTime + mRestTime; }

	
private:

	/// 스킬 인덱스
	unsigned long		mSkillIdx;
	/// 스킬 스텝
	unsigned char		mSkillStep;
	
	/// 쿨타임 적용 시작시간
	unsigned long		mCreationTime;

	/// 쿨타임 남은 시간 (적용 시작시간 이후로 부터)
	unsigned long		mRestTime;

};
