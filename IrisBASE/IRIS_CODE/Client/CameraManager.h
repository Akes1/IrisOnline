/* ==========================================================================
* 작 성 자 : 이법석
* 작 성 일 : 2006.12.07
* 내    용 : 카메라 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Camera.h"

enum
{
	eCamera_Hero = 0,
	eCamera_Free,
};

/// 카메라 관리자
/// 카메라를 생성, 제거하는 관리자이다.
class cCameraManager
{
	static cCameraManager* mpSingleton;

public:
	cCameraManager();
	~cCameraManager();

	/// 생성부
	bool Init();
	void Open();

	/// 키보드 및 마우스 입력에 대한 처리
	//bool ProcessInputKey( float time );
	//void ProcessInputMouse( float time );
	//bool ProcessKeyUpDown( float time );
	//bool ProcessKeyPressed( float time );
	void ProcessMouse( unsigned long time );

	/// 처리부( 절대적으로 입력처리후에 처리해야 한다.)
	void Process( unsigned long deltaTime, unsigned long accumTime );

	/// 재설정
	void Reset( unsigned int screenWidth, unsigned int screenHeight );

	/// 메인 카메라를 바꾼다.
	void ChangeCamera( unsigned int cameraIdx );

	/// 현재 카메라 정보를 획득
	NiCamera* GetCurrentNi();
	cCamera* GetCurrent();
	unsigned int GetCurrentIdx() { return mCurrentCameraIdx; }

	/// 현재 카메라의 위치를 얻는다.
	NiPoint3 GetCurrentCameraPos();

	/// LookAt이 있는 경우 카메라까지의 거리를 얻는다.
	float GetCurrentLookAtDistance();

	/// 화면 좌표에 대응하는 반직선을 얻음
	bool GetRayFromWindowPoint( NiPoint3* porigin, NiPoint3* pdir, int mouseX, int mouseY );

	void Shake();

public:
	/// 단일체를 리턴
	static cCameraManager* GetSingleton();

protected:
	typedef	tArray<void*> cCameraArray;
	cCameraArray	mCameraArray;

	cCamera*		mpCurrentCamera;
	unsigned int	mCurrentCameraIdx;
};

inline cCamera* cCameraManager::GetCurrent()
{
	return mpCurrentCamera;
}

inline NiCamera* cCameraManager::GetCurrentNi()
{
	if( mpCurrentCamera )
		return *mpCurrentCamera;
	else
		return 0;
}

inline cCameraManager* cCameraManager::GetSingleton()
{
	return mpSingleton;
}

#define CAMERAMAN	cCameraManager::GetSingleton()
