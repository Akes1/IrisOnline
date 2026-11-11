#include "StdAfx.h"
#include "Application.h"

#include "resource.h"
#include "AppWindow.h"
#include "AppTimer.h"

#include "..\Client\FilePackSystem.h"
#include "GameResourceManager.h"
#include "WorldManager.h"
#include "ItemManager.h"
#include "DummyManager.h"

cApplication* cApplication::mpSingleton = 0;


cApplication::cApplication()
: mpWindow(0)
, mpTimer(0)
, mpFileSystem(0)
, mpGameResourceManager(0)
, mpItemManager(0)
, mpWorldManager(0)
, mpDummyManager(0)
{
	mDeltaTime = 0;
	mAccumTime = 0;

	mpSingleton = this;
}

cApplication::~cApplication()
{
	SAFE_DELETE(mpTimer);

	SAFE_DELETE(mpDummyManager);
	SAFE_DELETE(mpWorldManager);
	SAFE_DELETE(mpItemManager);
	SAFE_DELETE(mpGameResourceManager);

	SAFE_DELETE(mpWindow);
	SAFE_DELETE(mpFileSystem);
}

cApplication* cApplication::Create()
{
	return new cApplication;
}

LRESULT CALLBACK cApplication::WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if( msg > WM_USER )
	{
		DUMMYMAN->NetWorkProc( msg, wparam, lparam );
		return 0;
	}

	HDC hdc;
	PAINTSTRUCT ps;

	switch( msg )
	{
	case WM_PAINT:
		{
			if( DUMMYMAN )
			{
				hdc=BeginPaint(wnd,&ps);
				SetTextAlign(hdc, TA_LEFT);
				cStringT str;
				str.Format( _T("Connect [%d]"), DUMMYMAN->checkCount );
				::TextOut(hdc, 0, 0, str.Cstr(), str.GetLength() );
				EndPaint(wnd,&ps);
			}
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	return ::DefWindowProc( wnd, msg, wparam, lparam );
}

bool cApplication::Init( HINSTANCE hi )
{
	mInstance = hi;

	///
	mpFileSystem = new cFilePackSystem;
	if( !mpFileSystem || mpFileSystem->Init() == false )
	{
		return false;
	}

	HWND wnd = CreateMainWindow();
	if( wnd == 0 )
	{
		NiMessageBox( "Failed create window.", "Error" );
		return false;
	}

	mpGameResourceManager = new cGameResourceManager;
	if( !mpGameResourceManager || mpGameResourceManager->Init() == false )
	{
		NiMessageBox( "Failed load resource.", "Error" );
		return false;
	}

	mpItemManager = new cItemManager;
	if( !mpItemManager || mpItemManager->Init() == false )
	{
		NiMessageBox( "Failed load item data.", "Error" );
		return false;
	}

	mpWorldManager = new cWorldManager;
	if( !mpWorldManager || mpWorldManager->Init() == false )
	{
		NiMessageBox( "Failed load world data.", "Error" );
		return false;
	}

	mpDummyManager = new cDummyManager;
	if( !mpDummyManager || mpDummyManager->Init( wnd ) == false )
	{
		NiMessageBox( "Failed dummymanager.", "Error" );
		return false;
	}

	///
	mpTimer = new cAppTimer;
	if( mpTimer == 0 )
	{
		return false;
	}
	else
	{
		mpTimer->Reset();
	}


	///
	mpTimer->Start();
	
	return true;
}

void cApplication::Exit()
{
	mInstance = 0;

	if( mpDummyManager )
		mpDummyManager->Exit();
}

void cApplication::Run()
{
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );

	while( msg.message != WM_QUIT )
	{
		if( ::PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
		else
		{
			Sleep(1);
			MainLoop();
		}
	}
}

HWND cApplication::CreateMainWindow()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= cApplication::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= mInstance;
	wcex.hIcon			= LoadIcon(mInstance, MAKEINTRESOURCE(IDI_DUMMYCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= cAppWindow::GetWindowClassName();
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if( RegisterClassEx(&wcex) == 0 )
		return 0;

	mpWindow = new cAppWindow;

	return mpWindow->Create( mInstance, 640, 480 );
}

void cApplication::MainLoop()
{
	mDeltaTime = mpTimer->GetDeltaTime();
	mAccumTime += mDeltaTime;


	mpDummyManager->Process( mDeltaTime, mAccumTime );
}