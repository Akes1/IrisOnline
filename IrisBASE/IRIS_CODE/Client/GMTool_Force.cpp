#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"

#include "ObjectManager.h"
#include "Hero.h"
#include "UIManager.h"
#include "GameResourceManager.h"
#include "Protocol_Cheat.h"
#include "Cheat_Common.h"
#include "ChatManager.h"

void cGMTool::OnShowForce()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_FORCE];
	HWND hForceChangeCombo = GetDlgItem( hWnd, IDC_COMBO_FORCE_CHANGE_LIST );
	HWND hForcePointCombo = GetDlgItem( hWnd, IDC_COMBO_FORCE_POINT_LIST );

	SendMessage( hForceChangeCombo, CB_RESETCONTENT, 0, 0 );
	SendMessage( hForcePointCombo, CB_RESETCONTENT, 0, 0 );

	for( int cnt = eFORCETYPE_FIRE; cnt < eFORCETYPE_MAX; ++cnt )
	{
		unsigned long textIdx = 119 + cnt ;
		int retIndex = SendMessage( hForceChangeCombo, CB_ADDSTRING, 0, (LPARAM)UIMAN->GetUIText( textIdx ) );
		if( retIndex != CB_ERR && retIndex != CB_ERRSPACE )
		{
			SendMessage( hForceChangeCombo, CB_SETITEMDATA, retIndex, cnt );
		}

		retIndex = SendMessage( hForcePointCombo, CB_ADDSTRING, 0, (LPARAM)UIMAN->GetUIText( textIdx ) );
		if( retIndex != CB_ERR && retIndex != CB_ERRSPACE )
		{
			SendMessage( hForcePointCombo, CB_SETITEMDATA, retIndex, cnt );
		}

	}
	
	SendMessage( hForceChangeCombo, CB_SETCURSEL, 0, 0 );
	SendMessage( hForcePointCombo, CB_SETCURSEL, 0, 0 );

	SetDlgItemInt( hWnd, IDC_EDIT_FORCE_POINT, 0, FALSE );
	SendDlgItemMessage( hWnd, IDC_EDIT_FORCE_POINT, EM_SETLIMITTEXT, (WPARAM)5, 0 );

	/// 위치세팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_FORCE].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_FORCE].y = GetSystemMetrics(SM_CYCAPTION);

}

/// 아이템 관련 버튼 이벤트
void cGMTool::OnCommandForce( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_FORCE_CANCEL:
	case IDC_BUTTON_FORCE_CANCEL2:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_FORCE], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_FORCE_CHANGE:
		{
			HWND forceChangeCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_FORCE], IDC_COMBO_FORCE_CHANGE_LIST );
			int listIndex = SendMessage( forceChangeCombo, CB_GETCURSEL, 0, 0 );
			int  forceType = SendMessage( forceChangeCombo, CB_GETITEMDATA, listIndex, 0 );
			if( forceType == CB_ERR || forceType < eFORCETYPE_FIRE || forceType >= eFORCETYPE_MAX )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8090 ) );
				break;
			}

			if( HERO->GetForceType() == (unsigned char)forceType )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8091 ) );
				break;
			}

			MSG_REQ_CHEAT_GM_FORCE_CHANGE msg;
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_GM_FORCE_CHANGE_REQ;
			msg.mForceType = (unsigned char)forceType;

			if( NETWORK->IsConnected() )
			{
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_CHEAT_GM_FORCE_CHANGE ) );
			}

			::ShowWindow( mSubDlg[GMTOOL_SUBDLG_FORCE], SW_HIDE );
			::SetActiveWindow( mParentWnd );
		}
		break;
	case IDC_BUTTON_FORCE_ADDPOINT:
		{
			if( HERO->GetForceType() == eFORCETYPE_NONE || HERO->GetForceType() == eFORCETYPE_MAX )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8095 ) );
				break;
			}

			HWND forcePointCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_FORCE], IDC_COMBO_FORCE_POINT_LIST );
			int listIndex = SendMessage( forcePointCombo, CB_GETCURSEL, 0, 0 );
			int  forceType = SendMessage( forcePointCombo, CB_GETITEMDATA, listIndex, 0 );
			if( forceType == CB_ERR || forceType < eFORCETYPE_FIRE || forceType >= eFORCETYPE_MAX )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8090 ) );
				break;
			}

			int  point;
			BOOL ret = FALSE;
			point = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_FORCE], IDC_EDIT_FORCE_POINT, &ret, FALSE );
			if( ret == FALSE  || point < 1 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 8093 ) );
				break;
			}

			MSG_REQ_CHEAT_GM_FORCE_POINT msg;
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_GM_FORCE_ADDPOINT_REQ;
			msg.mForceType = (unsigned char)forceType;
			msg.mForcePoint = (unsigned long)point;

			if( NETWORK->IsConnected() )
			{
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_CHEAT_GM_FORCE_POINT ) );
			}
			//::ShowWindow( mSubDlg[GMTOOL_SUBDLG_FORCE], SW_HIDE );
			::SetActiveWindow( mParentWnd );
		}
		break;
	default:
		return;
	}
}
