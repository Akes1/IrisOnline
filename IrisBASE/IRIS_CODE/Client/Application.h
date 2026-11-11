/* ====================================================================
* 파    일 :	Application.h
* 목    적 :	윈도우 및 게임의 기반 클래스들을 관리
* 작 성 자 :	이법석
* 작 성 일 :	2006.12
* 주의사항 :	
* =================================================================== */
#pragma once

class cAppWindow;
class cAppTimer;
class cFilePackSystem;
class cOptionManager;
class cRenderSystem;
class cSoundSystem;
class cInputSystem;
class cResourceManager;
class cCameraManager;
class cSceneManager;
class cStageManager;
//class cSyncTimer;
class cUICursor;

#ifdef _GMTOOL
class cGMTool;
#endif

#if (defined(_DEVSYS) || defined(_DEBUG))
class cDevSystem;
#endif

/// 어플리케이션
class cApplication
{
	static cApplication* mpSingleton;

public:
	cApplication();
	~cApplication();

	/// 어플리케이션을 생성
	static cApplication* Create();

	/// 메시지 처리 함수
	static LRESULT CALLBACK	WinProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );

	/// 가상 메모리 체크
	static bool CheckEnoughMemory();

	/// 물리 메모리 체크
	static bool	CheckEnoughHardDisk();

	/// Dx버젼 체크
	static bool	CheckDirectXVersion();

	/// 중복 실행에 대한 체크
	static bool	CheckExecutionApplication();

	/// 윈도우 활성화 체크용
	inline bool IsActive()	{ return mIsActive; }
	void SetActive( bool active );

	void SetMinimize( bool mini ) { mIsMinimze = mini; }

	/// 초기화
	bool Init( NiInstanceRef hi, cString ip, long authcode );

#ifdef _GMTOOL
	bool InitGMTool();
#endif

	/// 메인 루프를 실행
	void Run();

	/// 종료
	void Exit();

	/// 스크린 모드 토글
	void UpdateScreenSize( unsigned int width, unsigned int height, unsigned char bpp, bool windowed );

	/// 스크린샷
	void SetSaveScreenShot( bool save );

	/// 모드 전환시 자원 메모리 초기화
	void Invalidate();

	/// 모드 전화시 자원 메모리 재생산
	void Restore();

	inline bool IsReset() { return mResetFlag; } 

	/// Application 시간값
	inline unsigned long GetWorldAccumTime() { return mAccumTime; } 
	inline unsigned long GetWorldDeltaTime() { return mDeltaTime; }

	/// 어플리케이션 윈도우를 리턴
	inline cAppWindow* GetAppWindow() const { return mpWindow; }

	/// 윈도우 핸들을 리턴
	HWND GetHWND();
	HINSTANCE GetWindowInstance();

	//unsigned long GetIP() { return mIP; }
	char* GetIP() { return (char*)mIP.Cstr(); }

	LPCTSTR GetStrVer() { return mVersion.Cstr(); }

	long GetAuthCode() { return mAuthCode; }

private:
	/// 윈도우를 생성
	bool CreateMainWindow();

	/// 메인 루프
	void MainLoop();

	/// 스크린샷
//	bool SaveScreenShot();

	void Process();
	void Render();

public:
	/// 단일체를 리턴
	static cApplication* GetSingleton();

	static cString  mLangaugeFolder;
	static cStringW mLangaugeFolderW;

private:
	/// 인스턴스
	NiInstanceRef mpInstance;

	/// 윈도우
	cAppWindow* mpWindow;

	/// 타이머
	cAppTimer* mpTimer;

	/// 활성화 여부
	bool mIsActive;
	bool mIsMinimze;

	/// 누적 시간
	unsigned long mAccumTime;

	/// 프레임 경과 시간
	unsigned long mDeltaTime;

	/// 파일 팩 시스템
	cFilePackSystem* mpFileSystem;

	cOptionManager* mpOptionManager;

	/// 랜더링 시스템
	cRenderSystem* mpRenderSystem;

	/// 사운드 시스템
	cSoundSystem* mpSoundSystem;

	/// 입력 시스템
	cInputSystem* mpInputSystem;

	/// 자원 관리자
	cResourceManager* mpResourceManager;

	/// 카메라 관리자
	cCameraManager* mpCameraManager;

	/// 장면 관리자
	cSceneManager* mpSceneManager;

	/// 게임 스테이지 관리자
	cStageManager* mpStageManager;

	/// 마우스 커서
	cUICursor* mpCursor;

#ifdef _GMTOOL
private:
	/// 운영툴
	cGMTool* mGMTool;
#endif

#if (defined(_DEVSYS) || defined(_DEBUG))
private:
	/// 개발자를 위한 시스템
	cDevSystem* mpDevSystem;
#endif

	/// 스크린샷
	bool mSaveScreenShot;

	bool mResetFlag;

	cString mIP;

	cStringT mVersion;

	// 인증코드
	long mAuthCode;
};

inline
void cApplication::SetSaveScreenShot( bool save )
{
	mSaveScreenShot = save;
}

inline
cApplication* cApplication::GetSingleton()
{
	return mpSingleton;
}

#define THEAPP cApplication::GetSingleton()
