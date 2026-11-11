#pragma once

class cAppWindow;
class cAppTimer;
class cFilePackSystem;
class cGameResourceManager;
class cWorldManager;
class cItemManager;
class cDummyManager;

class cApplication
{
	static cApplication* mpSingleton;

public:
	cApplication();
	~cApplication();

	/// 단일체를 리턴
	static cApplication* GetSingleton();

	static cApplication* Create();
	static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	bool Init( HINSTANCE hi );
	void Run();
	void Exit();

	/// Application 시간값
	inline unsigned long GetWorldAccumTime() { return mAccumTime; } 
	inline unsigned long GetWorldDeltaTime() { return mDeltaTime; }

protected:
	HWND CreateMainWindow();

	void MainLoop();

private:
	HINSTANCE mInstance;

	/// 
	cAppWindow*	mpWindow;

	/// 타이머
	cAppTimer* mpTimer;
	unsigned long mDeltaTime;
	unsigned long mAccumTime;

	/// 파일 팩 시스템
	cFilePackSystem* mpFileSystem;


	/// game...
	cGameResourceManager* mpGameResourceManager;

	cItemManager*	mpItemManager;

	cWorldManager*	mpWorldManager;

	cDummyManager*	mpDummyManager;

	///
};

inline
cApplication* cApplication::GetSingleton()
{
	return mpSingleton;
}

#define THEAPP cApplication::GetSingleton()
