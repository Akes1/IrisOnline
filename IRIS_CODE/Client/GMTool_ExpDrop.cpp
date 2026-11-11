#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"

const int MAX_PERCENT =   999;
const int MIN_PERCENT =  -200;

void cGMTool::OnShowExpDrop()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_EXPDROP];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_EXPDROP].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_EXPDROP].y = GetSystemMetrics(SM_CYCAPTION);

	/// 초기화
	SendDlgItemMessage( hWnd, IDC_EDIT_EXPDROP_EXP, EM_SETLIMITTEXT, (WPARAM)4, 0 );
	SendDlgItemMessage( hWnd, IDC_EDIT_EXPDROP_DROP, EM_SETLIMITTEXT, (WPARAM)4, 0 );
	SetDlgItemInt( hWnd, IDC_EDIT_EXPDROP_EXP, mExpPercent, true );
	SetDlgItemInt( hWnd, IDC_EDIT_EXPDROP_EXP, mSxpPercent, true );
	SetDlgItemInt( hWnd, IDC_EDIT_EXPDROP_DROP, mDropPercent, true );

}

void cGMTool::OnCommandExpDrop( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_EXPDROP_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_EXPDROP], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_EXPDROP_OK:
		{
			BOOL ret = FALSE;
			short percent = 0;
			percent = (short)GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_EXPDROP], IDC_EDIT_EXPDROP_EXP, &ret, true );
			if( ret == TRUE && percent >= MIN_PERCENT && percent <= MAX_PERCENT )
			{
				/// 메시지 전송
				if( NETWORK->IsConnected() )
				{
					MSG_REQ_CHEAT_EXP msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_EXP_REQ;
					msg.mExpUpPercent = percent;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
					mExpPercent = percent;
				}
			}

			ret = FALSE;
			percent = 0;
			percent = (short)GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_EXPDROP], IDC_EDIT_EXPDROP_SXP, &ret, true );
			if( ret == TRUE && percent >= MIN_PERCENT && percent <= MAX_PERCENT )
			{
				/// 메시지 전송
				if( NETWORK->IsConnected() )
				{
					MSG_REQ_CHEAT_SXP msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_SXP_REQ;
					msg.mSxpUpPercent = percent;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
					mSxpPercent = percent;
				}
			}

			ret = FALSE;
			percent = 0;
			percent = (short)GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_EXPDROP], IDC_EDIT_EXPDROP_DROP, &ret, true );
			if( ret == TRUE && percent >= MIN_PERCENT && percent <= MAX_PERCENT )
			{
				/// 메시지 전송
				if( NETWORK->IsConnected() )
				{
					MSG_REQ_CHEAT_DROP msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_DROP_REQ;
					msg.mDropUpPercent = percent;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
					mDropPercent = percent;
				}
			}
		}
		break;

	}
}
