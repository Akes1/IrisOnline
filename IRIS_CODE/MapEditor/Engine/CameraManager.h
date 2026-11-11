/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 카메라 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Camera.h"

class cFreeCamera;
class cLookAtCamera;

/// 카메라 관리자
/// 카메라를 생성, 제거하는 관리자이다.
class cCameraManager
{
	static cCameraManager* mSingleton;

public:
	cCameraManager();
	~cCameraManager();

	/// 처리
	void Process( unsigned long deltaTime );

	/// 제거
	void DestroyAll();
	void Destroy( cCamera* cam );

	/// 생성
	cFreeCamera* CreateFree( bool makeCurrent = false );
	cLookAtCamera* CreateLookAt( bool makeCurrent = false );

	void SetCurrent( cCamera* cam );
	cCamera* GetCurrent();
	cCamera* GetCamera( unsigned int i );

	/// 화면 좌표에 대응하는 반직선을 얻음
	void GetRayFromWindowPoint( NiPoint3* porigin, NiPoint3* pdir, int mouseX, int mouseY );

	/// 속력
	void SetCameraTranslateSpeed( float unitsPerSec );
	void SetCameraRotateSpeed( float anglesPerSec );

public:
	/// 단일체를 리턴
	static cCameraManager* GetSingleton();

protected:
	/// 카메라 배열
	typedef tArray<cCamera*> cCameraArray;
	cCameraArray mCameraArray;

	/// 현재 카메라
	cCamera* mCurrentCamera;
};

inline
void cCameraManager::GetRayFromWindowPoint( NiPoint3* porigin, NiPoint3* pdir, int mouseX, int mouseY )
{
	if( mCurrentCamera )
	{
		mCurrentCamera->GetRayFromWindowPoint( mouseX, mouseY, *porigin, *pdir );
	}
	else
	{
		assert( 0 && "null current camera" );
	}
}

inline
cCamera* cCameraManager::GetCurrent()
{
	return mCurrentCamera;
}

inline
cCamera* cCameraManager::GetCamera( unsigned int i )
{
	if( i < mCameraArray.GetSize() )
		return mCameraArray[i];
	else
		return 0;
}

inline
cCameraManager* cCameraManager::GetSingleton()
{
	return mSingleton;
}

#define CAMERAMAN cCameraManager::GetSingleton()
