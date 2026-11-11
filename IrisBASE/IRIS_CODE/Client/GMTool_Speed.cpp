#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"
#include <commctrl.h>


const unsigned int MIN_SPEED = 0;
const unsigned int MAX_SPEED = 5;


void cGMTool::OnShowSpeed()
{

	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_SPEED];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_SPEED].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_SPEED].y = GetSystemMetrics(SM_CYCAPTION);

	/// 초기 트랙바 설정
	SendDlgItemMessage( hWnd, IDC_SLIDER_SPEED_MOVESPEED, TBM_SETRANGE, WPARAM( FALSE ), LPARAM( MAKELONG( MIN_SPEED, MAX_SPEED )) );
	SendDlgItemMessage( hWnd, IDC_SLIDER_SPEED_MOVESPEED, TBM_SETPOS, WPARAM( FALSE ), LPARAM( mSpeed ) );
}

void cGMTool::OnCommandSpeed( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_SPEED_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_SPEED], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_SLIDER_SPEED_MOVESPEED:
		{
			if ( msg == TB_ENDTRACK )
			{
				int speed = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_SPEED], IDC_SLIDER_SPEED_MOVESPEED, TBM_GETPOS, 0,0 );
				if( speed < MIN_SPEED  || speed > MAX_SPEED  || speed == mSpeed )
					return;
				mSpeed = speed;
				/// 메시지 전송
				if( NETWORK->IsConnected() )
				{
					MSG_REQ_CHEAT_SPEEDUP msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_SPEEDUP_REQ;
					msg.mSpeedUp = (eCHEAT_SPEEDUP)speed;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}
			}
		}
		break;
	}
}