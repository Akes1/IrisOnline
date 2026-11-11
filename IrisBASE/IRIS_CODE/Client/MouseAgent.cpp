#include "StdAfx.h"
#include "mouseagent.h"

cMouseAgent::cMouseAgent()
: mpMouseInfo(NULL), mhWnd(NULL)
{
	mWheelMsg		= 0;
	mQueSize		= MAX_QUESIZE;

	mHead			= 0;
	mTail			= 0;

	for( int i=0; i<eME_COUNT; i++ )
	{
		mIsMouseEvent[i] = false;
	}

	mIsLBtnPress = false;
	mIsRBtnPress = false;
	mIsMBtnPress = false;

	mIsDrag = false;
	mIsLBtnDrag = false;
	mIsRBtnDrag = false;
	mIsMBtnDrag = false;

	mX = 0;
	mY = 0;
	mOldX = 0;
	mOldY = 0;
	mMouseEventX = 0;
	mMouseEventY = 0;

	mAxisX = 0;
	mAxisY = 0;

	mWheel = 0;
	mCombineKey	= 0;
}

cMouseAgent::~cMouseAgent()
{
	Release();
}

/* ------------------------------------------------------------------
 * 함수이름 :	Create( HWND hWnd ) 
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cMouseAgent::Create( HWND hWnd ) 
{
	if( hWnd == NULL )
	{
		return false;
	}

	mhWnd = hWnd;

	mQueSize		= MAX_QUESIZE;
	mpMouseInfo		= new sMOUSEINFO[mQueSize];
	mHead			= 0;
	mTail			= 0;

	POINT pt;
	GetCursorPos( &pt );
	ScreenToClient( mhWnd, &pt );
	mOldX		= pt.x;
	mX			= pt.x;
	mOldY		= pt.y;
	mY			= pt.y;
	mMouseEventX	= 0;
	mMouseEventY	= 0;

	mAxisX			= 0;
	mAxisY			= 0;

	mWheelMsg		= 0;
	mCombineKey	= 0;

	return true;
}

/* ------------------------------------------------------------------
 * 함수이름 :	Release()
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cMouseAgent::Release()
{
	SAFE_DELETE_ARRAY(mpMouseInfo);
}

/* ------------------------------------------------------------------
 * 함수이름 :	GetWindowMessage( HWND hWnd, unsigned long msg, WPARAM wParam, LPARAM lParam )
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cMouseAgent::GetWindowMessage( HWND hWnd, unsigned long msg, WPARAM wParam, LPARAM lParam )
{
	if( hWnd != mhWnd )
		return false;

	switch( msg )
	{
	case WM_LBUTTONDOWN:	SetCapture( mhWnd ); RecordMouseEvent( eME_LDOWN, wParam, lParam );	break;
	case WM_RBUTTONDOWN:	SetCapture( mhWnd ); RecordMouseEvent( eME_RDOWN, wParam, lParam );	break;
	case WM_MBUTTONDOWN:	SetCapture( mhWnd ); RecordMouseEvent( eME_MDOWN, wParam, lParam );	break;
	case WM_LBUTTONUP:		ReleaseCapture(); RecordMouseEvent( eME_LUP, wParam, lParam );	break;
	case WM_RBUTTONUP:		ReleaseCapture(); RecordMouseEvent( eME_RUP, wParam, lParam );	break;
	case WM_MBUTTONUP:		ReleaseCapture(); RecordMouseEvent( eME_MUP, wParam, lParam );	break;
	case WM_LBUTTONDBLCLK:	RecordMouseEvent( eME_LDBL, wParam, lParam );	break;
	case WM_RBUTTONDBLCLK:	RecordMouseEvent( eME_RDBL, wParam, lParam );	break;
	case WM_MBUTTONDBLCLK:	RecordMouseEvent( eME_MDBL, wParam, lParam );	break;
	/// for wheel
	case 0x020A/*WM_MOUSEWHEEL*/:		mWheelMsg = int(wParam)>>16;	break;
	default: return false;
	}	

	return true;
}

/* ------------------------------------------------------------------
 * 함수이름 :	RecordMouseEvent( int event, WPARAM wParam, LPARAM lParam )
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cMouseAgent::RecordMouseEvent( int event, WPARAM wParam, LPARAM lParam )
{
	sMOUSEINFO mouseInfo;

	mouseInfo.Event	= event;
	mouseInfo.X		= LOWORD(lParam);
	mouseInfo.Y		= HIWORD(lParam);
	mouseInfo.Key	= wParam;

	PushMouseInfo( &mouseInfo );
}

/* ------------------------------------------------------------------
 * 함수이름 :	GetNextMouseEvent()
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cMouseAgent::GetNextMouseEvent()
{
	POINT pt;
	GetCursorPos( &pt );
	ScreenToClient( mhWnd, &pt );
	mX	= pt.x;
	mY	= pt.y;

	mAxisX	= mOldX - mX;
	mAxisY	= mOldY - mY;

	mOldX	= mX;
	mOldY	= mY;

	mWheel		= mWheelMsg;
	mWheelMsg	= 0;

	for( int i = 0; i<eME_COUNT; ++i )
	{
		mIsMouseEvent[i] = false;
	}

	sMOUSEINFO mouseInfo;
	if( PopMouseInfo( &mouseInfo ) == true )
	{
		ParseCurMouseEvent( &mouseInfo );
	}


	if( mAxisX || mAxisY )
	{
		mIsDrag = true;
	}
	else
	{
		mIsDrag = false;

		if( LButtonPressed() )
		{
			mIsLBtnDrag = true;
		}

		if( RButtonPressed() )
		{
			mIsRBtnDrag = true;
		}

		if( MButtonPressed() )
		{
			mIsMBtnDrag = true;
		}
	}
}

/* ------------------------------------------------------------------
 * 함수이름 :	ParseCurMouseEvent( sMOUSEINFO* pMouseInfo )
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cMouseAgent::ParseCurMouseEvent( sMOUSEINFO* pMouseInfo )
{
	mIsMouseEvent[pMouseInfo->Event] = true;

	switch( pMouseInfo->Event )
	{
	case eME_LDOWN:		
		mIsLBtnPress	= true;		
		break;
	case eME_RDOWN:		
		mIsRBtnPress	= true;		
		break;
	case eME_MDOWN:		
		mIsMBtnPress	= true;		
		break;
	case eME_LUP:		
		mIsLBtnPress	= false;	
		mIsLBtnDrag		= false;	
		break;
	case eME_RUP:		
		mIsRBtnPress	= false;	
		mIsRBtnDrag		= false;	
		break;
	case eME_MUP:		
		mIsMBtnPress	= false;	
		mIsMBtnDrag		= false;	
		break;
	case eME_LDBL:		
		break;
	case eME_RDBL:		
		break;
	case eME_MDBL:		
		break;
	}

	mMouseEventX	= pMouseInfo->X;
	mMouseEventY	= pMouseInfo->Y;
	mCombineKey		= pMouseInfo->Key;
}

/* ------------------------------------------------------------------
 * 함수이름 :	PushMouseInfo( sMOUSEINFO* pIn )
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cMouseAgent::PushMouseInfo( sMOUSEINFO* pIn )
{
	int tail = ( mTail + 1 ) % mQueSize;

	if( tail == mHead )
	{
		return false;
	}

	mpMouseInfo[mTail] = *pIn;
	mTail = tail;

	return true;
}

/* ------------------------------------------------------------------
 * 함수이름 :	PopMouseInfo( sMOUSEINFO* pOut )
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cMouseAgent::PopMouseInfo( sMOUSEINFO* pOut )
{
	if( mTail == mHead )
	{
		return false;
	}

	*pOut = mpMouseInfo[mHead];

	mHead = ( mHead + 1 ) % mQueSize;

	return true;
}

void cMouseAgent::ClearAllInfo()
{ 
	mTail = mHead; 

	ZeroMemory( mIsMouseEvent, sizeof( mIsMouseEvent ) );
}
