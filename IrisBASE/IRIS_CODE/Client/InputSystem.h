/* ====================================================================
* 파    일 :	InputSystem.h
* 목    적 :	I/O(마우스, 키보드)에 대한 관리
* 작 성 자 :	이법석 
* 작 성 일 :	2006.01
* 주의사항 :	
* =================================================================== */
#pragma once

#include "MouseAgent.h"
#include "KeyboardAgent.h"

class cInputSystem
{
	static cInputSystem* mpSingleton;

public:
	cInputSystem();
	virtual ~cInputSystem();

	/// 초기화 및 종료
	bool Init( NiInstanceRef hi, NiWindowRef hwnd );
	void Exit();

	/// 처리부
	void Process( unsigned long deltaTime );

	/// 스테이지 변경을 할 경우 기존 인풋 정보를 모두 제거한다.
	void InputInfoClearAll();

	void KeyInfoClear();

	/// Window Message Parser( 리턴값이 false인 경우 DefaultWndProc처리를 하지 않고 넘어간다 )
	bool WindowMessageParser( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );

	/// 마우스 관리자 획득
	inline cMouseAgent* GetMouse() { return &mMouse; }

	/// 키보드 관리자 획득
	inline cKeyboardAgent* GetKeyboard() { return &mKeyboard; }

protected:
	bool UseMouseEventByUIMgr();

public:
	/// 단일체를 리턴
	static cInputSystem* GetSingleton();

protected:

	/// DX Input 포인터
	LPDIRECTINPUT8		mpDI;

	/// 마우스 관리
	cMouseAgent			mMouse;

	/// 키보드 관리
	cKeyboardAgent		mKeyboard;
};

inline cInputSystem* cInputSystem::GetSingleton()
{
	return mpSingleton;
}

#define INPUTSYS	cInputSystem::GetSingleton()
#define MOUSE		INPUTSYS->GetMouse()
#define KEYBOARD	INPUTSYS->GetKeyboard()
