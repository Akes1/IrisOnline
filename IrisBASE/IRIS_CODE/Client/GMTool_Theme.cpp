#include "stdafx.h"
#include "GMTool.h"
#include <CommCtrl.h>
#include "resource.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"
#include "GameResourceManager.h"
#include "UIManager.h"

void cGMTool::OnShowTheme()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_THEME];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_THEME].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_THEME].y = GetSystemMetrics(SM_CYCAPTION);

	/// 초기화
	SetDlgItemText( hWnd, IDC_EDIT_THEME_CHECK_ID, _T("") );
	SetDlgItemText( hWnd, IDC_EDIT_THEME_NOTICE, _T("") );
	SendDlgItemMessage( hWnd, IDC_EDIT_THEME_CHECK_ID, EM_SETLIMITTEXT, (WPARAM)MAX_NAME_SIZE, 0 );
}

void cGMTool::OnHideTheme()
{
	ClearThemeResetList();
}

void cGMTool::OnCommandTheme( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_THEME_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_THEME], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_THEME_CHECK:
		{
			/// 내용 받기
			TCHAR str[MAX_NAME_BUFFER_SIZE] = {0,};
			int retcode = GetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_EDIT_THEME_CHECK_ID, str, MAX_NAME_BUFFER_SIZE );
			if( retcode == false )
				return;

			cStringT temp = str;
			temp.Replace( _T("\r\n"), _T(" ") );
			temp.Replace( _T("\t"), _T(" ") );

			if( temp.GetLength() < 2 )
			{
				///글자 입력 부족
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_EDIT_THEME_NOTICE, GAMERESOURCEMAN->GetGameText( 8105 ) );
				return;
			}

			/// 메시지 전송
			if( NETWORK->IsConnected() )
			{
				MSG_REQ_CHEAT_THEME_INFO msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_GM_THEME_INFO_REQ;
				::Sstrncpy( msg.mCharName, MAX_NAME_BUFFER_SIZE, temp.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}

//			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_EDIT_THEME_CHECK_ID, _T("") );

			ClearThemeResetList();
		}
		break;
	case IDC_BUTTON_THEME_RESET:
		{
			HWND  listViewControl = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME );
			int   idx = -1;

			idx = ListView_GetNextItem( listViewControl, -1, LVNI_ALL | LVNI_SELECTED );
			if( idx != -1 )
			{
				TCHAR state[MAX_NAME_BUFFER_SIZE] = {0,};
				ListView_GetItemText( listViewControl, idx, 0, state, sizeof(state) );
				if( _wcsnicmp( state, _T("-"), 1 ) == 0 )
					return;

				/// 테마이름, 난이도, 방번호, 케릭터 인덱스 정보 전송
				sTheme_Info* pThemeInfo = NULL;
				LVITEM LvItem;
				LvItem.mask = LVIF_PARAM;
				LvItem.iItem = idx;
				LvItem.iSubItem = 0;
				SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_GETITEM, 0, (LPARAM)&LvItem );
				
				pThemeInfo = (sTheme_Info*)LvItem.lParam;

				if( pThemeInfo )
				{
					MSG_REQ_CHEAT_THEME_RESET msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_GM_THEME_RESET_REQ;
					msg.mCharacterIdx = pThemeInfo->mCharacterIdx;
					msg.mThemeIdx     = pThemeInfo->mThemeIdx;
					msg.mThemeMode    = pThemeInfo->mThemeMode;

					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}
			}
			else
			{
				/// 해제할 유저를 선택해주세요.
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_EDIT_THEME_NOTICE, GAMERESOURCEMAN->GetGameText( 8106 ) );
			}
		}
		break;
	}
}

void cGMTool::SetThemeResetInfo( MSG_RES_CHEAT_THEME_INFO* pResMsg )
{
	if( pResMsg == NULL )
	{
		assert(0);
		return;
	}

	ClearThemeResetList();

	if( pResMsg->mRowCount == 0 )
	{
		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = 0;
		LvItem.iSubItem = 0;
		LvItem.pszText = _T("-");
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTITEM, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

		LvItem.iSubItem = 1;
		LvItem.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8099 );
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_SETITEMTEXT, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

		return;
	}

	/// 테이터 입력
	sCheat_Theme_Info* table = pResMsg->mThemeInfo;

	for( int cnt = 0; cnt < pResMsg->mRowCount; cnt++, table++ )
	{
		if( table == NULL )
		{
			assert(0);
			continue;
		}

		sTheme_Info* themeInfoEx = new sTheme_Info;
		if( themeInfoEx )
		{
			themeInfoEx->mCharacterIdx = pResMsg->mCharacterIdx;
			themeInfoEx->mThemeIdx	   = table->themeIdx;
			themeInfoEx->mThemeMode	   = table->mode;
		}
		else
		{
			assert(0);
			continue;
		}

		cStringT tempText;
		tempText.Format(_T("%d"), cnt+1 );
		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.iItem = cnt;
		LvItem.iSubItem = 0;
		LvItem.lParam = (LPARAM)themeInfoEx;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTITEM, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		// 맵 이름
		LvItem.mask = LVIF_TEXT;
		LvItem.iSubItem = 1; 
		LvItem.pszText = (LPWSTR)GAMERESOURCEMAN->GetMapName( table->themeIdx );
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		// 귀속 고유 번호
		LvItem.iSubItem = 2;
		tempText.Format(_T("%d"), table->uniqueIdx );
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		//난이도
		LvItem.iSubItem = 3;
		LvItem.pszText = (LPWSTR)UIMAN->GetUIText( 26500 + table->mode );
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		//마지막 업데이트일
		struct tm date;
		localtime_s( &date, &table->lastDate );

		date.tm_year = date.tm_year+1900;
		date.tm_mon  = date.tm_mon+1;
		tempText.Format(_T("[%04d/%02d/%02d]%02d:%02d:%02d"), date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec );

		LvItem.iSubItem = 4;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		//유효 만기일
		localtime_s( &date, &table->validThru );

		date.tm_year = date.tm_year+1900;
		date.tm_mon  = date.tm_mon+1;
		tempText.Format(_T("[%04d/%02d/%02d]%02d:%02d:%02d"), date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec );

		LvItem.iSubItem = 5;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
	}
}

void cGMTool::ClearThemeResetList()
{
	HWND listViewControl = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME );
	int itemMax = ListView_GetItemCount( listViewControl );
	for( int cnt = 0; cnt < itemMax; ++cnt )
	{
		sTheme_Info* pMsg = NULL;
		LVITEM LvItem;
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = 0;
		LvItem.iSubItem = 0;
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_GETITEM, 0, (LPARAM)&LvItem );

		pMsg = (sTheme_Info*)LvItem.lParam;
		SAFE_DELETE(pMsg);
		
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_DELETEITEM, (WPARAM)(int)0, 0);
	}

	/// 리스트 박스 초기화
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_DELETEALLITEMS, 0, 0 );

	SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_EDIT_THEME_NOTICE, _T("") );
}