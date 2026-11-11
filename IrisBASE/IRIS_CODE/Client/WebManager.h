/* ==========================================================================
* 작 성 자 : 
* 작 성 일 : 
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include <SHLGUID.h>

const unsigned int WEB_WIDTH = 1024;
const unsigned int WEB_HEIGHT = 768;

class cWebManager
{
	static cWebManager* mpSingleton;

public:
	cWebManager();
	~cWebManager();

	bool Init( HINSTANCE hinst, HWND hwnd );
	void Release();

	void Show();
	void Hide();
	void UpdateRect( unsigned int x, unsigned int y, unsigned int w, unsigned int h );

public:
	static cWebManager* GetSingleton();

	static INT_PTR CALLBACK DlgProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );

private:
	HWND mParentWnd;
	HWND mMainDlg;
	HINSTANCE mInstance;

	//
	IWebBrowser2* mpWebBrowser;
};

inline cWebManager* cWebManager::GetSingleton()
{
	return mpSingleton;
}

#define WEBMAN	cWebManager::GetSingleton()