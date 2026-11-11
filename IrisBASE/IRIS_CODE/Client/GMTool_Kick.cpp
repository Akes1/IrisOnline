#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"

void cGMTool::OnShowKick()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_KICK];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
	
	mSubDlgPos[GMTOOL_SUBDLG_KICK].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_KICK].y = GetSystemMetrics(SM_CYCAPTION);

	/// 초기화
	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_KICK], IDC_EDIT_KICK_ID, _T("") );
}

void cGMTool::OnCommandKick( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_KICK_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_KICK], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_KICK_OK:
		{
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_KICK], IDC_EDIT_KICK_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;

			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_KICK msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_KICK_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_KICK], IDC_EDIT_KICK_ID, _T("") );
		}
		break;
	case IDC_EDIT_KICK_ID:
		{
		}
		break;
	}
}