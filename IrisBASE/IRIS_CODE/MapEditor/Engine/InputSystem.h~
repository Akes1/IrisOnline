/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 입력 시스템
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "MouseAgent.h"
#include "KeyAgent.h"

/// 입력 시스템
class cInputSystem
{
	friend class Application;
	static cInputSystem* mSingleton;

public:
	cInputSystem();
	~cInputSystem();

	/// 처리
	void Process();

	/// 마우스 중개자를 리턴
	cMouseAgent* GetMouseAgent();

	/// 키보드 중개자를 리턴
	cKeyAgent* GetKeyAgent();

public:
	/// 단일체를 리턴
	static cInputSystem* GetSingleton();

protected:
	/// 마우스 중개자
	cMouseAgent mMouseAgent;

	/// 키보드 중개자
	cKeyAgent mKeyAgent;
};

inline cMouseAgent* cInputSystem::GetMouseAgent()
{
	return &mMouseAgent;
}

inline cKeyAgent* cInputSystem::GetKeyAgent()
{
	return &mKeyAgent;
}

inline cInputSystem* cInputSystem::GetSingleton()
{
	return mSingleton;
}
