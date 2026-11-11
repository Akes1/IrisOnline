#include "stdafx.h"
#include "GMTool.h"
#include <CommCtrl.h>
#include "resource.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"
#include "GameResourceManager.h"

void cGMTool::OnShowStop()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_STOP];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_STOP].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_STOP].y = GetSystemMetrics(SM_CYCAPTION);

	/// 초기화
	SetDlgItemText( hWnd, IDC_EDIT_STOP_ID, _T("") );
	SendDlgItemMessage( hWnd, IDC_EDIT_STOP_TIME, EM_SETLIMITTEXT, (WPARAM)5, 0 );
	SendDlgItemMessage( hWnd, IDC_EDIT_STOP_ID, EM_SETLIMITTEXT, (WPARAM)MAX_NAME_SIZE, 0 );
	SetDlgItemInt ( hWnd, IDC_EDIT_STOP_TIME, 60, false );
	
}

void cGMTool::OnCommandStop( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_STOP_CANCEL:
	case IDC_BUTTON_STOP_CANCEL2:
	case IDC_BUTTON_STOP_CANCEL3:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_STOP], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_STOP_OK:
		{
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;

			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			BOOL ret = FALSE;
			int time = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_TIME, &ret, false );
			if( ret == FALSE )
				return;

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_STOP msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_STOP_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );
				msg.mDuration = time;

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_ID, _T("") );
			//SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_TIME, _T("") );
		}
		break;
	case IDC_BUTTON_NOCHAT_OK: /// 채팅 금지
		{
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;
			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			BOOL ret = FALSE;
			int time = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_TIME, &ret, false );
			if( ret == FALSE )
				return;

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_NO_CHAT msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_NO_CHAT_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );
				msg.mDuration = time;

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
		}
		break;
	case IDC_BUTTON_STOP_RELEASE_DIRECT: /// 행동 제한 이름 입력해서 해제
		{
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_RELEASE_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;
			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_CANCEL_STOP msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_CANCEL_STOP_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
		}
		break;
	case IDC_BUTTON_NOCHAT_RELEASE_DIRECT: /// 채팅 금지 이름 입력해서 해제
		{
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_RELEASE_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;
			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_ALLOW_CHAT msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_ALLOW_CHAT_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );
				
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
		}
		break;
	case IDC_BUTTON_STOP_RELEASE: ///리스트 컨트롤에서 선택된 유저 행동 제한 해제
		{

		}
		break;
	case IDC_BUTTON_NOCHAT_CHECK: /// 채팅이 금지된 유저 상태 체크
		{
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_EDIT_STOP_CHECK_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;
			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_NO_CHAT_LIST msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_NO_CHAT_LIST_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
		}
		break;
	case IDC_BUTTON_NOCHAT_RELEASE: ///리스트 컨트롤에서 선택된 유저 채팅 금지 해제
		{
			HWND  listViewControl = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP );
			int   idx = -1;

			idx = ListView_GetNextItem( listViewControl, -1, LVNI_ALL | LVNI_SELECTED );
			if( idx != -1 )
			{
				TCHAR state[MAX_NAME_BUFFER_SIZE] = {0,};
				ListView_GetItemText( listViewControl, idx, 0, state, sizeof(state) );
				if( _wcsnicmp( state, _T("-"), 1 ) == 0 )
					return;

				TCHAR charName[MAX_NAME_BUFFER_SIZE] = {0,};
				ListView_GetItemText( listViewControl, idx, 1, charName, sizeof(charName) );
				if( _tcslen( charName ) > 0 )
				{
					MSG_REQ_CHEAT_ALLOW_CHAT msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_ALLOW_CHAT_REQ;
					::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, charName, MAX_NAME_SIZE );

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}
			}
			else
			{
				/// 해제할 유저를 선택해주세요.
			}

		}
		break;
	}
}

void cGMTool::SetNoChatUser( int rowCount, NO_CHAT_CHARACTER_DUMMY* table )
{
	/// 리스트 박스 초기화
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_DELETEALLITEMS, 0, 0 );

	if( rowCount == 0 )
	{
		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.iItem = 0;
		LvItem.iSubItem = 0;
		LvItem.pszText = _T("-");
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_INSERTITEM, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

		LvItem.iSubItem = 1;
		LvItem.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8099 );
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_SETITEMTEXT, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

		return;
	}

	/// 테이터 입력
	for( int cnt = 0; cnt < rowCount; cnt++, table++ )
	{
		
		cStringT tempText;
		tempText.Format(_T("%d"), cnt+1 );
		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.iItem = cnt;
		LvItem.iSubItem = 0;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_INSERTITEM, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
				
		LvItem.iSubItem = 1;
		LvItem.pszText = (LPWSTR)table->characterName;
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));

		LvItem.iSubItem = 2;
		LvItem.pszText = (LPWSTR)table->adminName;
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));

		struct tm date;
		localtime_s( &date, &table->registDate );

		date.tm_year = date.tm_year+1900;
		date.tm_mon  = date.tm_mon+1;

		tempText.Format(_T("[%04d/%02d/%02d]%02d:%02d:%02d"), date.tm_year,
															  date.tm_mon,
															  date.tm_mday,
															  date.tm_hour,
															  date.tm_min,
															  date.tm_sec );

		LvItem.iSubItem = 3;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));

		localtime_s( &date, &table->validThru );

		date.tm_year = date.tm_year+1900;
		date.tm_mon  = date.tm_mon+1;

		if( difftime( table->validThru, table->registDate ) > 1 )
		{
			tempText.Format(_T("[%04d/%02d/%02d]%02d:%02d:%02d"), date.tm_year,
				date.tm_mon,
				date.tm_mday,
				date.tm_hour,
				date.tm_min,
				date.tm_sec );
		}
		else
			tempText.Format( GAMERESOURCEMAN->GetGameText( 8115 ) );

		LvItem.iSubItem = 4;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
	}

	/// 초기 선택.
}

void cGMTool::InitStopListBox()
{
	/// 리스트 박스 초기화
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_DELETEALLITEMS, 0, 0 );
}
