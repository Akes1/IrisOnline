/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 어플리케이션
* 주의사항 : 
*===========================================================================*/
#pragma once

class cAppWindow;
class cAppTimer;
class cFilePackSystem;
class cRenderSystem;
class cSoundSystem;
class cInputSystem;
class cResourceManager;
class cGameResourceManager;
class cCameraManager;
class cSceneManager;
class cSkillManager;
class cDramaturgyManager;

/// 어플리케이션
class cApplication
{
	static cApplication* mSingleton;

public:
	cApplication();
	~cApplication();

	/// 지움
	void Clear();

	/// 초기화
	bool Init( HWND wnd );

	/// 종료
	void Exit();

	/// 처리
	void Process();

	/// 렌더링
	void Render();

	/// 활성화 여부
	void SetActive( bool active );
	bool IsActive();

	/// 타이머
	cAppTimer* GetTimer();
	unsigned long GetDeltaTime() const;
	unsigned long GetAccumTime() const;

public:
	/// 단일체를 리턴
	static cApplication* GetSingleton();

	static cString mLangaugeFolder;

private:
	/// 메인 창
	cAppWindow* mWindow;

	/// 파일 시스템
	cFilePackSystem* mFileSystem;

	/// 렌더링 시스템
	cRenderSystem* mRenderSystem;

	/// 사운드 시스템
	cSoundSystem* mSoundSystem;

	/// 입력 시스템
	cInputSystem* mInputSystem;

	/// 자원 관리자
	cResourceManager* mResourceManager;
	cGameResourceManager* mGameResourceManager;

	/// 카메라 관리자
	cCameraManager* mCameraManager;

	/// 장면 관리자
	cSceneManager* mSceneManager;

	cSkillManager*	mSkillManager;

	cDramaturgyManager* mDramaManager;

	/// 타이머
	cAppTimer* mTimer;

	/// 누적 시간
	unsigned long mAccumTime;

	/// 경과 시간
	unsigned long mDeltaTime;

	/// 활성화 여부
	bool mActive;
};

inline void cApplication::SetActive( bool active )
{
	mActive = active;
}

inline bool cApplication::IsActive()
{
	return mActive;
}

inline cAppTimer* cApplication::GetTimer()
{
	return mTimer;
}

inline unsigned long cApplication::GetDeltaTime() const
{
	return mDeltaTime;
}

inline unsigned long cApplication::GetAccumTime() const
{
	return mAccumTime;
}


inline cApplication* cApplication::GetSingleton()
{
	return mSingleton;
}

#define THEAPP cApplication::GetSingleton()
