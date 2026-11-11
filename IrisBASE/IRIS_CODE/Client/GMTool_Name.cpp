#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"

void cGMTool::OnShowGMName()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_GMNAME];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_GMNAME].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_GMNAME].y = GetSystemMetrics(SM_CYCAPTION);

	/// 초기화
	SendDlgItemMessage( hWnd, IDC_EDIT_GMNAME_ID, EM_SETLIMITTEXT, (WPARAM)8, 0 );
	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_GMNAME], IDC_EDIT_GMNAME_ID, _T("") );
	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_GMNAME], IDC_EDIT_GMNAME_NOTICE, _T("") );
}

void cGMTool::OnCommandGMName( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_GMNAME_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_GMNAME], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_GMNAME_OK:
		{
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_GMNAME], IDC_EDIT_GMNAME_NOTICE, _T("") );

			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_GMNAME], IDC_EDIT_GMNAME_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;

			if( HERO == NULL )
				return;

			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			if( temp.GetLength() < 2 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_GMNAME], IDC_EDIT_GMNAME_NOTICE, GAMERESOURCEMAN->GetGameText( 8105 ) );
				return;
			}

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_GM_NAME msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_GM_NAME_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			//SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_GMNAME], IDC_EDIT_GMNAME_ID, _T("") );
		}
		break;
	case IDC_EDIT_GMNAME_ID:
		{
		}
		break;
	}
}