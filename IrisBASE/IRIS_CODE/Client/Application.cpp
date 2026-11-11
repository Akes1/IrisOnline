#include "StdAfx.h"
#include "resource.h"
#include "Application.h"

#include "commondefines.h"
#include "number.ini"

#include "DXVersion.h"
#include "AppWindow.h"
#include "AppTimer.h"

#include "RenderSystem.h"
#include "InputSystem.h"
#include "FilePackSystem.h"
#include "SoundSystem.h"

#include "OptionManager.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "UIManager.h"
#include "StageManager.h"

#include "UICursor.h"

#ifdef _GMTOOL
	#include "GMTool.h"
#endif


#if (defined(_DEVSYS) || defined(_DEBUG))
#include "DevSystem.h"
#endif

#pragma data_seg("shared")
long mUsageCnt = 0;
#pragma data_seg()
#pragma comment(linker, " /section:shared,rws")

cApplication* cApplication::mpSingleton = 0;
cString cApplication::mLangaugeFolder = "";
cStringW cApplication::mLangaugeFolderW = L"";

/// directx version data size
const unsigned int BUFSIZE = 80;

cApplication::cApplication()
: mpFileSystem( 0 )
, mpOptionManager( 0 )
, mpRenderSystem( 0 )
, mpSoundSystem( 0 )
, mpInputSystem( 0 )
, mpResourceManager( 0 )
, mpCameraManager( 0 )
, mpSceneManager( 0 )
, mpInstance( 0 )
, mpStageManager( 0 )
, mpCursor(0)
#ifdef _GMTOOL
, mGMTool( 0 )
#endif
#if (defined(_DEVSYS) || defined(_DEBUG))
, mpDevSystem( 0 )
#endif
, mSaveScreenShot( false )
, mResetFlag( false )
, mAuthCode(-1)
{

	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	mIsActive = false;
	mIsMinimze = false;

	mpWindow = new cAppWindow;		
	
	mpTimer = NULL;
	/// 070110 PKH 동기화용 타이머
	mAccumTime = 0;
	mDeltaTime = 0;
}

cApplication::~cApplication()
{
#ifdef _GMTOOL
	SAFE_DELETE( mGMTool );
#endif
#if (defined(_DEVSYS) || defined(_DEBUG))
	SAFE_DELETE( mpDevSystem );
#endif

	SAFE_DELETE( mpCursor );
	if( mpStageManager )
	{
		mpStageManager->Exit();
		delete mpStageManager;
		mpStageManager = 0;
	}
	SAFE_DELETE( mpSceneManager );
	SAFE_DELETE( mpCameraManager );
	SAFE_DELETE( mpTimer );
	SAFE_DELETE( mpInputSystem );
	SAFE_DELETE( mpResourceManager );
	SAFE_DELETE( mpRenderSystem );
	SAFE_DELETE( mpOptionManager );
	SAFE_DELETE( mpSoundSystem );
	SAFE_DELETE( mpFileSystem );

	/// 윈도우의 제거는 임의로 먼저 한다.
	SAFE_DELETE( mpWindow );

	mpSingleton = 0;
}

HWND cApplication::GetHWND()
{ 
	return (mpWindow)? mpWindow->GetHWND():NULL; 
}

HINSTANCE cApplication::GetWindowInstance()
{
	return mpInstance;
}

cApplication* cApplication::Create()
{
	return new cApplication;
}

bool cApplication::Init( NiInstanceRef hi, cString ip, long authcode )
{
	mpInstance = hi;
	mIP = ip;
	mAuthCode = authcode;

	/// 파일 팩 시스템을 생성
	mpFileSystem = new cFilePackSystem;
	if( !mpFileSystem || !mpFileSystem->Init() )
	{
		NiMessageBox( "Failed to Init FileSystem", "Error" );
		return false;
	}

	/// 사운드 시스템 생성 및 초기화
	mpSoundSystem = new cSoundSystem;
	if( mpSoundSystem == 0 )
	{
		assert( 0 && "failed to init sound system" );
		NiMessageBox( "Failed to Init SoundSystem", "Error" );
		return false;
	}
	else
	{
		mpSoundSystem->Init();
	}

	/// 윈도우를 생성
	if( CreateMainWindow() == false )
	{
		NiMessageBox( "Failed to create window.", "Error" );
		return false;
	}

	mpSoundSystem->SetHWND( mpWindow->GetHWND() );

	/// option file load
	mpOptionManager = new cOptionManager;
	if( !mpOptionManager || !mpOptionManager->Init() )
	{
		NiMessageBox( "Failed option file load/save.", "Error" );
		return false;
	}

	/// 렌더링 시스템을 생성 및 초기화
	mpRenderSystem = new cRenderSystem( mpWindow );
	if( !mpRenderSystem || !mpRenderSystem->Init() )
	{
		assert( 0 && "failed to init render system" );
		NiMessageBox( "Failed to Init RenderSystem", "Error" );
		return false;
	}

	/// 자원 관리자를 생성 및 초기화
	mpResourceManager = new cResourceManager;
	if( !mpResourceManager || !mpResourceManager->Init() )
	{
		assert( 0 && "failed to init resource manager" );
		NiMessageBox( "Failed to Init Resource manager", "Error" );
		return false;
	}

	/// 커서
	mpCursor = new cUICursor;
	if( !mpCursor || !mpCursor->Init() )
	{
		assert( 0 && "failed to Init mouse cursor" );
		return false;
	}

	/// 입력 시스템을 생성 및 초기화
	mpInputSystem = new cInputSystem;
	if( !mpInputSystem || !mpInputSystem->Init( mpInstance, mpWindow->GetHWND() ) )
	{
		assert( 0 && "failed to init input system" );
		NiMessageBox( "Failed to Init InputSystem", "Error" );
		return false;
	}

	/// 타이머를 생성 및 초기화
	mpTimer = new cAppTimer;
	if( !mpTimer )
	{
		return false;
	}
	mpTimer->Reset();

	/// 카메라 관리자를 생성 및 초기화
	mpCameraManager = new cCameraManager;
	if( !mpCameraManager || !mpCameraManager->Init() )
	{
		assert( 0 && "failed to init camera manager" );
		NiMessageBox( "Failed to Init Camera manager", "Error" );
		return false;
	}

	/// 장면 관리자를 생성
	mpSceneManager = new cSceneManager;
	if( !mpSceneManager || !mpSceneManager->Init() )
	{
		assert( 0 && "failed to init scene manager");
		NiMessageBox( "Failed to Init Scene manager", "Error" );
		return false;
	}

	/// stage manager 생성 및 초기화
	mpStageManager = new cStageManager;
	if( !mpStageManager || !mpStageManager->Init( mpWindow->GetHWND(), mpRenderSystem->GetScreenWidth(), mpRenderSystem->GetScreenHeight() ) )
	{
		assert( 0 && "failed to init stage manager");
		NiMessageBox( "Failed to Init Stage manager", "Error" );
		return false;
	}

#if (defined(_DEVSYS) || defined(_DEBUG))
	mpDevSystem = new cDevSystem();
	if( !mpDevSystem || !mpDevSystem->Init() )
	{
		return false;
	}
#endif

	/// System Timer Start..
	mpTimer->Start();

	mpWindow->AdjustWindowForChange( mpRenderSystem->IsFullScreenMode() );
	mpWindow->AdjustWindowRect( mpRenderSystem->GetScreenWidth(), mpRenderSystem->GetScreenHeight(), true );

	UpdateScreenSize( mpRenderSystem->GetScreenWidth(), mpRenderSystem->GetScreenHeight(), mpRenderSystem->GetScreenBpp(), !mpRenderSystem->IsFullScreenMode() );

	//
	::ShowWindow( THEAPP->GetHWND(), SW_SHOW );

	DWORD topID = GetWindowThreadProcessId( GetForegroundWindow(), NULL );
	DWORD curID = GetCurrentThreadId();

	if( topID != curID )
	{
		if( AttachThreadInput( topID, curID, TRUE ) )
		{
			BringWindowToTop( THEAPP->GetHWND() );
			AttachThreadInput( topID, curID, FALSE );
		}
	}
	else
	{
		BringWindowToTop( THEAPP->GetHWND() );
	}


	return true;
}

void cApplication::Run()
{
	MSG  msg;
	ZeroMemory( &msg, sizeof(msg) );

	while( msg.message != WM_QUIT )
	{
		if( ::PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			/// alt + f4 키 기능을 막는다.
 			if( msg.message == WM_SYSCOMMAND && msg.wParam == SC_CLOSE )
 				continue;

			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
		else
		{
			if ( NETWORK != NULL )
				NETWORK->Process( (WPARAM)ICT_RECV, (LPARAM)NULL );
			MainLoop();
		}
	}
}

/// 목적: 각종 시스템 및 관리자들을 제거
/// 주의: 생성의 역순으로 제거하는 규칙을 지켜주세요.
void cApplication::Exit()
{
	if(mpStageManager)
	{
		mpStageManager->Exit();
		delete mpStageManager;
		mpStageManager = 0;
	}

	if( mpSceneManager )
		mpSceneManager->Exit();


	if( mpResourceManager )
		mpResourceManager->Exit();


#if (defined(_DEVSYS) || defined(_DEBUG))
	if( mpDevSystem )
		mpDevSystem->Exit();
#endif

	if( mpInputSystem )
		mpInputSystem->Exit();

	if( mpRenderSystem )
		mpRenderSystem->Exit();

#ifdef _GMTOOL
	if( mGMTool )
		mGMTool->Exit();
#endif

	/// 타이머를 정지
	if( mpTimer )
		mpTimer->Stop();
}

/// 목적: 모드 전환시 그래픽 메모리의 초기화
/// 주의: 게임브리오 내부 루틴에서 지원하는 부분이지만,
/// 문제 발생시 루틴수정없이 제어하기 위한 함수
void cApplication::Invalidate()
{
	if( mpRenderSystem )
		mpRenderSystem->Invalidate();

#if (defined(_DEVSYS) || defined(_DEBUG))
	if(mpDevSystem)
		mpDevSystem->Invalidate();
#endif
}

/// 목적: 모드 전환시 그래픽 메모리의 재생산
/// 주의: 게임브리오 내부 루틴에서 지원하는 부분이지만,
/// 문제 발생시 루틴수정없이 제어하기 위한 함수
void cApplication::Restore()
{
#if (defined(_DEVSYS) || defined(_DEBUG))
	if(mpDevSystem)
		mpDevSystem->Restore();
#endif

	if( mpRenderSystem )
		mpRenderSystem->Restore();
}

void cApplication::UpdateScreenSize( unsigned int width, unsigned int height, unsigned char bpp, bool windowed )
{
	mpRenderSystem->Reset( width, height, bpp, windowed );
}

#include "XPerformanceGuard.h"

void cApplication::MainLoop()
{

	/// 윈도우가 비활성화 되었을때 다른 Application에 CPU 타임을 할당
	if( mIsActive == false )
		NiSleep( 1 );

	/// 경과 시간 및 누적 시간을 계산
	mDeltaTime = mpTimer->GetDeltaTime();
	mAccumTime += mDeltaTime;

	XPG_SET( "Performance Test", 200 );

	/// 프로세싱
	mpInputSystem->Process( mDeltaTime );
	XPG_DO( "mpInputSystem" );

	mpResourceManager->Process();
	XPG_DO( "mpResourceManager" );

	mpStageManager->Process( mDeltaTime, mAccumTime );
	XPG_DO( "mpStageManager(Process)" );

	mpCameraManager->Process( mDeltaTime, mAccumTime );
	XPG_DO( "mpCameraManager" );

	mpSceneManager->Process( mDeltaTime, mAccumTime );
	XPG_DO( "mpSceneManager" );

	mpSoundSystem->Process( mDeltaTime, mAccumTime );
	XPG_DO( "mpSoundSystem" );

	mpStageManager->ProcessWorld( mDeltaTime, mAccumTime );
	XPG_DO( "mpStageManager(ProcessWorld)" );

#if (defined(_DEVSYS) || defined(_DEBUG))
	mpDevSystem->Process( mAccumTime );
	XPG_DO( "mpDevSystem" );
#endif

	/// 렌더링
	mpRenderSystem->Render();
	XPG_DO( "mpRenderSystem" );

	/// 스크린샷
	if( mSaveScreenShot )
	{
		mSaveScreenShot = false;
		mpRenderSystem->SaveScreenShot();
//		SaveScreenShot();
	}

}

void cApplication::Process()
{
	
}

void cApplication::Render()
{

}

bool cApplication::CreateMainWindow()
{
	/// 윈도우를 등록
	WNDCLASS wndclass;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = cApplication::WinProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = mpInstance;
	wndclass.hIcon         = ::LoadIcon(mpInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	wndclass.hCursor       = ::LoadCursor(mpInstance, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH);
	wndclass.lpszClassName = cAppWindow::GetWindowClassName();
	wndclass.lpszMenuName  = 0;

	if( ::RegisterClass(&wndclass) == 0 )
	{
		NiMessageBox( "Can't Create Window!", "Error!!" );
		return false;
	}

	NiWindowRef hWnd = mpWindow->Create( mpInstance, 0, 0 );

	mVersion.Format( _T("v%d.%.2d.%d"), LOBYTE(VERSION_HEADER), HIBYTE(VERSION_HEADER), BUILD_NUM );

	cStringT str;
	str.Format( _T("IRIS ONLINE %s"), mVersion.Cstr() );
	::SetWindowText( hWnd, str.Cstr() );

	return true;
}

bool cApplication::CheckEnoughMemory()
{
	return true;
}

bool cApplication::CheckEnoughHardDisk()
{
	return true;
}

bool cApplication::CheckDirectXVersion()
{
	/// 실제 GetDXVersion() 함수의 속도가 느린부분 때문에, 레지스트리 정보를 믿어본다.
	/// 이부분은 처음 실행시 한번 검사를 하고, 그 값을 파일에 저장시키고,
	/// 다음 실행시는 레지스트리를 먼저 검사하는 방식으로 변경한다.
	HKEY hkey;
	char producttype[BUFSIZE];
	unsigned long buflen = BUFSIZE;
	long ret;

	/// 레지스트리에 있는 정보를 신뢰한다.
	ret = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DirectX"), 0, KEY_QUERY_VALUE, &hkey );
	if( ret == ERROR_SUCCESS )
	{
		ret = ::RegQueryValueEx( hkey, _T("Version"), NULL, NULL, (LPBYTE) producttype, &buflen);
		if( (ret == ERROR_SUCCESS) && (buflen <= BUFSIZE) )
		{
			if( producttype[2] == '0' && producttype[3] < '9' )
			{
				/// Version Check Fail!
				NiMessageBox( "최신의 DirectX 버젼을 설치하세요.\n\n문제가 계속될 경우 재설치 하시기 바랍니다.", "Error Info" );
				::RegCloseKey( hkey );
				return false;
			}
			else
			{
				/// Version Check Succeed!
				::RegCloseKey( hkey );
				return true;
			}
		}

		::RegCloseKey( hkey );
	}

	/// DxDialog와 File버젼을 체크한다.
	HRESULT hr = E_FAIL;
	unsigned long dxver = 0;
	TCHAR dxvertext[10];

	CDXVersion kVer;
	hr = kVer.GetDXVersion( &dxver, dxvertext, 10 );

	if( SUCCEEDED(hr) )
	{
		/// 9.0~ 9.0c까지 통과.
		if( HIWORD(dxver) < 9 )
		{
			NiMessageBox( "최신의 DirectX 버젼을 설치하세요.", "Error Info" );
			return false;
		}
	}
	else
	{
		/// Dx 버젼에 대한 체크 실패.
		NiMessageBox( "DirectX설치에 문제가 있습니다.\n\n문제가 계속될 경우 재설치 하시기 바랍니다.", "Error Check" );
		return false;
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	CheckExecutionApplication()
* 목    적 :	뮤텍스를 통해서 Application의 중복 실행을 제한한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cApplication::CheckExecutionApplication()
{
	cAppWindow::mMutexHandle = ::CreateMutex( NULL, TRUE, cAppWindow::GetWindowClassName() );
	unsigned long err = ::GetLastError();
	if( err != ERROR_SUCCESS )
	{
		NiMessageBox( "Application already running...", "Error Info" );
		return false;
	}

	mUsageCnt++;
	if( mUsageCnt > 1 )
	{
		NiMessageBox( "Application already running...", "Error Info" );
		return false;
	}

	if( ::FindWindow( cAppWindow::mWindowClassName, NULL ) )
	{
		NiMessageBox( "Application already running...", "Error Info" );
		return false;
	}

	return true;
}

void cApplication::SetActive( bool active )
{
	if( mIsMinimze == true )
		mIsActive = false;
	else
		mIsActive = active;

	if( SOUNDSYS )
		SOUNDSYS->ActiveSound( mIsActive );

	if( mpWindow )
	{
		if( mIsActive )
			mpWindow->ChangeWindowOrder( true );
		else
			mpWindow->ChangeWindowOrder( false );
	}
}

/* ------------------------------------------------------------------
* 함수이름 :	WinProc()
* 목    적 :	생성된 System및 함수를 일괄 제거한다.
* 주의사항 :	생성의 역순으로 제거하는 규칙을 지켜주세요.
* ------------------------------------------------------------------ */
LRESULT CALLBACK cApplication::WinProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	/// window message parsing !!
	if( INPUTSYS && INPUTSYS->WindowMessageParser( wnd, msg, wparam, lparam ) )
	{
		/// 포커스객체에서 사용하는 경우
		/// DX_INPUT 키보드 정보 비운다..
		INPUTSYS->KeyInfoClear();
		return 0;
	}

	switch( msg ) 
	{
	case WM_CREATE:
		{
			::SetTimer( wnd, 0, 60000, NULL );

	/*		HIMC hImc = ImmGetContext( wnd );
			DWORD dwConversion, dwSentence;
			ImmGetConversionStatus( hImc, &dwConversion, &dwSentence );
			dwSentence |= IME_SMODE_SINGLECONVERT;
			ImmSetConversionStatus( hImc, dwConversion, dwSentence );
			ImmReleaseContext( wnd, hImc );
			*/
		}
		break;
	case WM_CHAR:
	case WM_IME_CHAR:
		return 0;
	case WM_IME_STARTCOMPOSITION:
		{
			if( UIMAN && !UIMAN->GetFocusedNode() )
			{
				HIMC hImc = ImmGetContext( wnd );
				ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
				ImmReleaseContext( wnd, hImc );
			}
		}
		return 0;
	case WM_TIMER:
		{
			///
			NETWORK->SendHeartbeat();
		}
		break;
	case WM_SOCKEVENTMSG2:
		NETWORK->Process( wparam, lparam );
		break;
	case WM_ACTIVATEAPP:
		{
			if( THEAPP )
			{
				if( wparam == TRUE )
				{
					THEAPP->SetActive( true );
				}
				else 
				{
					THEAPP->SetActive( false );
#if !(defined(_DEVSYS) || defined(_DEBUG))
					if( GetForegroundWindow() != wnd )
						::SetWindowPos( wnd, GetForegroundWindow(), 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
#endif
				}
			}
		}
		break;
	case WM_ACTIVATE:
		{
			if( THEAPP )
			{
				if( wparam == WA_INACTIVE )
				{
					THEAPP->SetActive( false );

#if !(defined(_DEVSYS) || defined(_DEBUG))
					if( GetForegroundWindow() != wnd )
						::SetWindowPos( wnd, GetForegroundWindow(), 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
#endif
				}
				else 
				{
					THEAPP->SetActive( true );
				}
			}
		}
		break;
	case WM_SETCURSOR:
		{ 
			if( CURSOR != NULL )
			{
				CURSOR->Process(0);
			}
		}
		break;
	//case WM_IME_SETCONTEXT:
	//	{
	////		// 반드시 DefWindowProc로 보내야한다 ( 후보윈도우 출력을 위해 )
	////		/*if( wparam == TRUE )
	////		{
	//			lparam &= ~( ISC_SHOWUICOMPOSITIONWINDOW );
	////		}

	//		::DefWindowProc( wnd, WM_IME_SETCONTEXT, wparam, lparam);

	//		//::DefWindowProc( wnd, WM_IME_SETCONTEXT, true, ISC_SHOWUICANDIDATEWINDOW );
	//		return 0;
	//	}
	//	break;
	case WM_SYSCOMMAND:
		switch( wparam )
		{
		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_MONITORPOWER:
			{
				if( RENDERSYS && RENDERSYS->IsFullScreenMode() )
				{
					return 1;
				}
			}
			return 0;
		case SC_KEYMENU:
			{
				return 1;
			}
		case SC_MINIMIZE:
			{
				THEAPP->SetMinimize( true );
			}
			break;
		case SC_RESTORE:
			{
				THEAPP->SetMinimize( false );
			}
			break;
		case SC_PREVWINDOW: /// alt + f6 기능을 막는다.
			return 0;
		}
		break;
		/// 휴대용 PC에서 게임을 하는 경우에 대한 처리
	case WM_POWERBROADCAST:	
		{
			switch( wparam )
			{
#ifndef	PBT_APMQUERYSUSPEND
#define	PBT_APMQUERYSUSPEND 0x0000
#endif
	case PBT_APMQUERYSUSPEND:
		return true;
#ifndef	PBT_APMRESUMESUSPEND
#define PBT_APMRESUMESUSPEND 0x0007
#endif
	case PBT_APMRESUMESUSPEND:
		return true;
			}
		}
		break;
	case WM_MOVE:
		{
#ifdef _GMTOOL
			if( GMTOOL )
				GMTOOL->OnMove();
#endif
		}
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		{
			::PostQuitMessage( 0 );
		}
		return 0;
	case WM_DISPLAYCHANGE:
		{
			/// 실행중 해상도 변경 체크
			unsigned int width = LOWORD(lparam);
			unsigned int height = HIWORD(lparam);
			unsigned int bpp = wparam;

			if( RENDERSYS )
				RENDERSYS->SetMonitorDisplay( width, height, bpp );
		}
		break;
	}

	return ::DefWindowProc( wnd, msg, wparam, lparam );
}

/*
bool cApplication::SaveScreenShot()
{
	NiDX9Renderer* renderer = NiDX9Renderer::GetRenderer();
	if( renderer == 0 )
		return false;

	/// 백버퍼를 파일로 저장
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();
	if( device == 0 )
		return false;

	IDirect3DSurface9* buffer = 0;
	if( device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &buffer ) != D3D_OK )
		return false;

	if( buffer == 0 )
		return false;

	///
	if( ::FileExist( "ScreenShot" ) == false )
	{
		::CreateDirectory( L"ScreenShot", 0 );
	}

	///
	SYSTEMTIME tm;
	::GetLocalTime( &tm );

	cStringT pathName;
	pathName.Format( L"ScreenShot/iris_screenshot_%d_%02d_%02d_%02d_%02d_%02d.jpg", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond );

	HRESULT ret = D3DXSaveSurfaceToFile( pathName.Cstr(), D3DXIFF_JPG, buffer, 0, 0 );
	buffer->Release();

	if( ret == D3D_OK )
		mpStageManager->AddScreenShotMsg( pathName.Cstr() );

	return ret == D3D_OK;
}
*/

#ifdef _GMTOOL
bool cApplication::InitGMTool()
{
	mGMTool = new cGMTool();
	if( mGMTool == 0 || mGMTool->Init( mpInstance, mpWindow->GetHWND() ) == false )
		return false;

	return true;
}
#endif
