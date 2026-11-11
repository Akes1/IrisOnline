/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.17
* 내    용 : 키보드 중개자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "KeyCode.h"

/// 키보드 중개자
class cKeyAgent
{
	static cKeyAgent* mSingleton;

public:
	cKeyAgent();
	~cKeyAgent();

	/// 키 메시지 처리 함수
	/// 윈도우 메시지 처리 함수에 의해 자동으로 호출된다.
	void SetDown( eKeyCode code );
	void SetUp( eKeyCode code );

	/// 키가 눌렸는지를 리턴
	bool IsDown( eKeyCode code );

	/// 키가 떼였는지를 리턴
	bool IsUp( eKeyCode code );

public:
	/// 단일체를 리턴
	static cKeyAgent* GetSingleton();

protected:
	/// 키 상태 배열
	bool mKeyStates[256];
};

inline bool cKeyAgent::IsDown( eKeyCode code )
{
	return mKeyStates[code];
}

inline bool cKeyAgent::IsUp( eKeyCode code )
{
	return !mKeyStates[code];
}

inline cKeyAgent* cKeyAgent::GetSingleton()
{
	return mSingleton;
}

#define KEY	cKeyAgent::GetSingleton()
