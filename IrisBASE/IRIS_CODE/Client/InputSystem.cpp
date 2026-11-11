#include "StdAfx.h"
#include "inputsystem.h"
#include "ShortcutManager.h"

#include "Application.h"
//#include "RenderSystem.h"
#include "StageManager.h"

#include "ObjectManager.h"
#include "Hero.h"

#include "CameraManager.h"
#include "UICursor.h"
#include "UINode.h"

#include "SkillExecuter.h"

cInputSystem* cInputSystem::mpSingleton = 0;

cInputSystem::cInputSystem()
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;
}

cInputSystem::~cInputSystem()
{
	Exit();

	mpSingleton = 0;
}

bool cInputSystem::Init( NiInstanceRef hi, NiWindowRef hwnd )
{
	HRESULT hr;

	/// 장치 생성
	hr = DirectInput8Create( hi, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&mpDI, NULL );
	if( FAILED( hr ) )
	{
		return false;
	}

	/// 키보드 생성
	hr |= mKeyboard.Create( hwnd, mpDI );
	if( FAILED( hr ) )
	{
		return false;
	}

	/// 마우스 생성
	if( !mMouse.Create( hwnd ) )
	{
		return false;
	}

	return true;
}

void cInputSystem::Exit()
{
	mMouse.Release();
	mKeyboard.Release();

	SAFE_RELEASE( mpDI );
}


/// 주의사항 : 키보드와 마우스의 처리 순서는 변경하지 말것!!
void cInputSystem::Process( unsigned long deltaTime )
{
	/// 
	OBJECTMAN->ClearPickObject();

	bool keyMoveUpdate = false;

	/// Keyboard 처리
	mKeyboard.UpdateKeyboardState();
	if( THEAPP->IsActive() == false )
	{
		mKeyboard.ClearKeyState();
	}
	else
	{
		/// 포커스 노드가 있거나, 모달 다이얼로그가 떠있으면 처리 안함.
		if( UIMAN->GetFocusedNode() || UIMAN->IsShowModalBox() )
		{
			mKeyboard.ClearKeyState();
		}
		else
		{
			/// 최상위 컨테이너에 이벤트를 전달한다..

			/// 이곳은 스테이지에 따라서 막아 놓자!!

			/// 게임상 키입력 처리들...
//			mKeyboard.StartGetKeyState();
//			while( mKeyboard.GetNextKeyState() )
			if( mKeyboard.GetDirectInput() )
			{
				mKeyboard.GetNextKeyState();

				/// 게임상 키입력 처리들...
				STAGEMAN->ProcessKeyUpDown( deltaTime, &mKeyboard );

				//////////////////////////////////////////////////////////////////////////
				/// key move check
				if( mKeyboard.GetKeyDown(KEY_MOVE_UP) || mKeyboard.GetKeyDown(KEY_MOVE_LEFT) || 
					mKeyboard.GetKeyDown(KEY_MOVE_RIGHT) || mKeyboard.GetKeyDown(KEY_MOVE_DOWN) || mKeyboard.GetKeyDown(KEY_AUTO_RUN) ||
					mKeyboard.GetKeyUp(KEY_MOVE_UP) || mKeyboard.GetKeyUp(KEY_MOVE_LEFT) || 
					mKeyboard.GetKeyUp(KEY_MOVE_RIGHT) || mKeyboard.GetKeyUp(KEY_MOVE_DOWN) )
				{
					keyMoveUpdate = true;
				}
				//////////////////////////////////////////////////////////////////////////
			}

			STAGEMAN->ProcessKeyPressed( &mKeyboard );
		}
	}

	/// Mouse 처리
	do
	{
		mMouse.GetNextMouseEvent();

		if( mMouse.LButtonDoubleClick() || mMouse.LButtonDown() )
		{
			/// 기본 커서였을 경우에만 클릭시 클릭 커서로 변경
			CURSOR->SetCursor( eCURSOR_CLICK );
		}
		else if( mMouse.LButtonUp() )
		{
			CURSOR->SetCursor( eCURSOR_UNCLICK );
		}

		/// UI 인풋 처리
		if( UseMouseEventByUIMgr() )
		{
			/// UI가 메세지를 처리했다면...
			if( mMouse.RButtonDown() || mMouse.LButtonDown() )
			{
				if( STAGEMAN->GetCurrentStage() == eStage_Game )
				{
					if( HERO )
					{
						if( SKILLEXECUTER->IsSkillCharge() ||
							SKILLEXECUTER->IsFieldSkillCharge() )
						{
							sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
							SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
						}
					}
				}
			}
			
			continue;
		}

		/// 카메라에 대한 인풋 처리 ( WM_MOUSEMOVE )
		if( STAGEMAN->GetCurrentStage() == eStage_Game )
		{
			CAMERAMAN->ProcessMouse( deltaTime );

			if( UIMAN->IsVisibleDrag() || UIMAN->IsVisibleUnionDrag() )
			{
				CURSOR->SetCursor( eCURSOR_CLICK );
				continue;
			}

 			cUINode* node = UIMAN->GetHoverNode();
 			if( node )
 			{
				if( node->IsSetCursorDefault() == true )
 					CURSOR->SetCursor( eCURSOR_DEFAULT );

 				continue;
 			}
		}

		/// 게임상 마우스 이벤트 처리들...
		STAGEMAN->ProcessMouseEvent( deltaTime, &mMouse );

		if( mMouse.RButtonDown() || mMouse.RButtonUp() )
		{
			if( HERO && HERO->IsReadyUseSkill() )
				keyMoveUpdate = true;
		}
	}
	while( !mMouse.IsEmptyMouseEvent() );

	if( keyMoveUpdate == true )
	{
		if( HERO && STAGEMAN->GetCurrentStage() == eStage_Game )
			HERO->UpdateKeyState();
	}
}

bool cInputSystem::UseMouseEventByUIMgr()
{
	cUIEvent event;
	event.mControl = mMouse.ControlPressed();
	event.mShift = mMouse.ShiftPressed();
	event.mAlt = mMouse.AltPressed();

	if( THEAPP->IsActive() )
	{
		if( mMouse.LButtonDown() )
		{
			event.mType = eUIEVENT_LBUTTON_DOWN;
			event.mPos.mX = mMouse.GetMouseEventX();//mMouse.GetMouseX();
			event.mPos.mY = mMouse.GetMouseEventY();//mMouse.GetMouseY();

			return UIMAN->PostEventFromUser( event );
		}
		else if( mMouse.LButtonUp() )
		{
			event.mType = eUIEVENT_LBUTTON_UP;
			event.mPos.mX = mMouse.GetMouseEventX();//mMouse.GetMouseX();
			event.mPos.mY = mMouse.GetMouseEventY();//mMouse.GetMouseY();

			return UIMAN->PostEventFromUser( event );
		}
		else if( mMouse.RButtonDown() )
		{
			event.mType = eUIEVENT_RBUTTON_DOWN;
			event.mPos.mX = mMouse.GetMouseEventX();//mMouse.GetMouseX();
			event.mPos.mY = mMouse.GetMouseEventY();//mMouse.GetMouseY();

			return UIMAN->PostEventFromUser( event );
		}
		else if( mMouse.RButtonUp() )
		{
			event.mType = eUIEVENT_RBUTTON_UP;
			event.mPos.mX = mMouse.GetMouseEventX();//mMouse.GetMouseX();
			event.mPos.mY = mMouse.GetMouseEventY();//mMouse.GetMouseY();

			return UIMAN->PostEventFromUser( event );
		}
		else if( mMouse.LButtonDoubleClick() )
		{
			event.mType = eUIEVENT_LBUTTON_DBLCLK;
			event.mPos.mX = mMouse.GetMouseEventX();//mMouse.GetMouseX();
			event.mPos.mY = mMouse.GetMouseEventY();//mMouse.GetMouseY();

			return UIMAN->PostEventFromUser( event );
		}
	}

	if( mMouse.GetWheel() )
	{
		event.mType = eUIEVENT_MOUSE_WHEEL;
		event.mPos.mX = mMouse.GetMouseX();
		event.mPos.mY = mMouse.GetMouseY();
		event.mWheel = mMouse.GetWheel();

		return UIMAN->PostEventFromUser( event );

	}

	/// WM_MOUSE_MOVE에 대한 처리는 윈도우 활성/비활성과 무관한다.
	if( mMouse.IsDrag() )
	{
		event.mType = eUIEVENT_MOUSE_MOVE;
		//event.mPos.mX = (float)mMouse.GetMouseX() / (float)RENDERSYS->GetScreenWidth();
		//event.mPos.mY = (float)mMouse.GetMouseY() / (float)RENDERSYS->GetScreenHeight();
		event.mPos.mX = mMouse.GetMouseX();
		event.mPos.mY = mMouse.GetMouseY();
		return UIMAN->PostEventFromUser( event );
	}
	return false;
}

bool cInputSystem::WindowMessageParser( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	/// 마우스에서 처리되는 이벤트냐!!
	if( mMouse.GetWindowMessage( wnd, msg, wparam, lparam ) )
		return false;

	/// UI 이벤트
	cUIEvent event;

	/// UI 에디트 계열에 포커스가 있는 경우 처리
	if( UIMAN == 0 )
		return false;

	if( UIMAN->IsShowModalBox() == false && UIMAN->IsShowModalessBox() == false )
	{
		if( !UIMAN->GetFocusedNode() )
			return false;
	}
	else
	{
		/// modal 이 띄워져 있을경우에도 처리해야 할 경우.
		mKeyboard.UpdateKeyboardState();
		if( mKeyboard.GetDirectInput() )
		{
			mKeyboard.GetNextKeyState();

			if( mKeyboard.GetKeyDown( KEY_SCREENSHOT ) == true )
				THEAPP->SetSaveScreenShot( true );
		}
	}
	
	/// 키보드 이벤트에 대한 처리!!
	switch( msg )
	{
		/*
		BYTE keys[256];
		WORD asc = 0;
		GetKeyboardState( keys );
		ToAscii( wparam, lparam, keys, &asc, 0 );

		event.mShift = (keys[VK_SHIFT] & 0x80) != 0;
		event.mControl = (keys[VK_CONTROL] & 0x80) != 0;
		event.mAscii = asc >= 0 ? asc : 0;
		*/
			/// 1. KEY DOWN/UP 처리
	case WM_KEYDOWN:
		{
			event.mType = eUIEVENT_KEY_DOWN;
			event.mCode = (eKeyCode)wparam;

			UIMAN->PostEventFromUser( event );
		}
		return true;
	case WM_KEYUP:
		{
			event.mType = eUIEVENT_KEY_UP;
			event.mCode = (eKeyCode)wparam;

			UIMAN->PostEventFromUser( event );

//			/// 스크린샷..
//			if( event.mCode  == eKEY_F12 )
//				THEAPP->SetSaveScreenShot( true );

		}
		return true;

		/// 2. WM_CHAR Message 처리
	case WM_CHAR:
		{
			event.mType = eUIEVENT_CHAR;
			event.mWparam = (TCHAR)wparam;

			UIMAN->PostEventFromUser( event );
		}
		//return true;
		break;
	
		/// 3. IME Message 처리
	//case WM_INPUTLANGCHANGE:
	//	{
	//		int a= 0;
	//		a=1;
	//	}
	//	//return true;
	//	break;
	case WM_IME_NOTIFY:
		{
			switch( wparam )
			{
			case IMN_OPENCANDIDATE:
				break;
			case IMN_SETCONVERSIONMODE:
				{
					event.mType = eUIEVENT_IME_NOTIFY;
					event.mWparam = wparam;
					UIMAN->PostEventFromUser( event );
				}
				break;
			}
		}
		break;
	case WM_IME_STARTCOMPOSITION:
		{
			event.mType = eUIEVENT_IME_STARTCOMPOSITION;
			event.mCode = (eKeyCode)wparam;

			UIMAN->PostEventFromUser( event );
		}
		//return true;
		break;
	case WM_IME_COMPOSITION:
		{
			/// 한글 조합중 
			event.mType = eUIEVENT_IME_COMPOSITION;
			event.mWparam = (BYTE)wparam;
			event.mLparam = lparam;

			UIMAN->PostEventFromUser( event );
		}
		//return true;
		break;
	case WM_IME_ENDCOMPOSITION:
		{
			event.mType = eUIEVENT_IME_ENDCOMPOSITION;

			UIMAN->PostEventFromUser( event );
		}
		//return true;
		break;
	}

	return false;
}

void cInputSystem::InputInfoClearAll()
{
	mKeyboard.UpdateKeyboardState();
	mKeyboard.ClearKeyState();
	mMouse.ClearAllInfo();
}

void cInputSystem::KeyInfoClear()
{
	mKeyboard.UpdateKeyboardState();
	mKeyboard.ClearKeyState();
}
