#include "stdafx.h"
#include "GMTool.h"
#include <CommCtrl.h>
#include "resource.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"

#include "GameResourceManager.h"
#include "SkillScript.h"
#include "AppTimer.h"

const int MAX_PERCENT =   999;
const int MIN_PERCENT =  -200;

void cGMTool::OnShowEvent()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_EVENT];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_EVENT].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_EVENT].y = GetSystemMetrics(SM_CYCAPTION);

	/// 초기화
	SetDlgItemTextW( hWnd, IDC_EDIT_EVENT_NOTICE, _T("") );
	SendDlgItemMessage( hWnd, IDC_COMBO_EVENT_TYPE, CB_SETCURSEL, 0, 0 );
	EventInfluenceClass();
	ClearGMEventList();
}


bool cGMTool::LoadEventData()
{

	HWND hEventCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_COMBO_EVENT_TYPE );
	if( hEventCombo == 0 )
	{
		assert(0);
		return false;
	}

	/// 미리 설정된 효과값을 가져온다.
	tArray<unsigned long>* pGMEventAry = SKILLSCRIPT->GMEventInfAry();
	if( pGMEventAry != NULL && pGMEventAry->GetSize() != 0 )
	{
		for( unsigned long i = 0 ; i < pGMEventAry->GetSize() ; ++i )
		{
			sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( (*pGMEventAry)[i] );
			if( pInfo )
			{
				int idx = SendMessage( hEventCombo, CB_ADDSTRING, 0, (LPARAM)GAMERESOURCEMAN->GetSkillNameText( pInfo->mNameidx ) );		
				SendMessage( hEventCombo, CB_SETITEMDATA, idx, (LPARAM)(*pGMEventAry)[i] );
			}
			else
			{
				assert(0);
				return false;
			}
		}
	}

	return true;
}

void cGMTool::OnCommandEvent( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_EVENT_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_EVENT], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_COMBO_EVENT_TYPE:
		{
			switch( msg )
			{
			case CBN_SELCHANGE:
				{
					EventInfluenceClass();
				}
				break;
			}
		}
		break;
	case IDC_BUTTON_EVENT_SEARCH:
		{
			SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, _T("") );
			ClearGMEventList();


			MSG_REQ_CHEAT_GM_EVENT_SEARCH msg;
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_GM_EVENT_SEARCH_REQ;

			if( NETWORK )
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSGROOT ) );
		}
		break;
	case IDC_BUTTON_EVENT_OK:
		{
			SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, _T("") );
			ClearGMEventList();

			//설정한 효과값 가져오기
			HWND hEventCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_COMBO_EVENT_TYPE );
			if( hEventCombo == 0 )
			{
				assert(0);
				return;
			}

			int idx = SendMessage( hEventCombo, CB_GETCURSEL, 0, 0 );
			int influenceIdx = SendMessage( hEventCombo, CB_GETITEMDATA, idx, 0 );
			if( idx != CB_ERR && influenceIdx != CB_ERR )
			{
				sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
				if( pInfo == NULL )
				{
					///해당 효과값이 유효하지 않습니다.
					assert(0);
					break;
				}
			}
			else
			{
				/// 효과선택을 제대로 선택하세요
				assert(0);
				break;
			}

			//설정한 시간 가져오기
			HWND hDay = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_DATETIMEPICKER_EVENT_STARTDAY );
			SYSTEMTIME startDay;
			if( DateTime_GetSystemtime( hDay, &startDay ) != GDT_VALID )
			{
				SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, GAMERESOURCEMAN->GetGameText( 8123 ) );
				assert(0);
				break;
			}
			HWND hTime = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_DATETIMEPICKER_EVENT_STARTTIME );
			SYSTEMTIME startTime;
			if( DateTime_GetSystemtime( hTime, &startTime ) != GDT_VALID )
			{
				SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, GAMERESOURCEMAN->GetGameText( 8124 ) );
				assert(0);
				break;
			}
			startTime.wYear		 = startDay.wYear;
			startTime.wMonth	 = startDay.wMonth;
			startTime.wDay		 = startDay.wDay;
			startTime.wDayOfWeek = startDay.wDayOfWeek;
			
			hDay = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_DATETIMEPICKER_EVENT_ENDDAY );
			SYSTEMTIME endDay;
			if( DateTime_GetSystemtime( hDay, &endDay ) != GDT_VALID )
			{
				SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, GAMERESOURCEMAN->GetGameText( 8123 ) );
				assert(0);
				break;
			}
			hTime = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_DATETIMEPICKER_EVENT_ENDTIME );
			SYSTEMTIME endTime;
			if( DateTime_GetSystemtime( hTime, &endTime ) != GDT_VALID )
			{
				SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, GAMERESOURCEMAN->GetGameText( 8124 ) );
				assert(0);
				break;
			}
			endTime.wYear	   = endDay.wYear;
			endTime.wMonth	   = endDay.wMonth;
			endTime.wDay	   = endDay.wDay;
			endTime.wDayOfWeek = endDay.wDayOfWeek;

			time_t currenttime;
			time( &currenttime );
			
			tm startTimeTM = TIMER->ConvertSTtoTM ( startTime );
			tm validthruTM = TIMER->ConvertSTtoTM ( endTime );

			time_t sendStartTime = mktime( &startTimeTM );
			time_t sendValidThru = mktime( &validthruTM );
			
			/// 이미 만기일이 지나간 것들이라면 에러 리턴
			if( difftime( sendValidThru, currenttime ) < 1 )
			{
				SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, GAMERESOURCEMAN->GetGameText( 8122 ) );
				break;
			}

			/// 시작 시간이 끝나는 시간보다 미래면 에러 리턴
			if( difftime( sendValidThru, sendStartTime ) < 1 )
			{
				SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, GAMERESOURCEMAN->GetGameText( 8128 ) );
				break;
			}

			/// 보내기
			MSG_REQ_CHEAT_GM_EVENT_APPLY msg;
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_GM_EVENT_APPLY_REQ;
			msg.mInfluenceIdx = influenceIdx;
			msg.mStartTime    = startTime;
			msg.mValidThru    =  endTime;

			if( NETWORK )
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_CHEAT_GM_EVENT_APPLY ) );
		}
		break;
	case IDC_BUTTON_EVENT_DELETE:
		{
			HWND  listViewControl = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT );
			int   idx = -1;

			idx = ListView_GetNextItem( listViewControl, -1, LVNI_ALL | LVNI_SELECTED );
			if( idx != -1 )
			{
				TCHAR state[MAX_NAME_BUFFER_SIZE] = {0,};
				ListView_GetItemText( listViewControl, idx, 0, state, sizeof(state) );
				if( _wcsnicmp( state, _T("-"), 1 ) == 0 )
					return;

				/// 효과 DB 인덱스 전송
				unsigned long DBIndex = 0;
				LVITEM LvItem;
				LvItem.mask = LVIF_PARAM;
				LvItem.iItem = idx;
				LvItem.iSubItem = 0;
				if( SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_GETITEM, 0, (LPARAM)&LvItem ) == TRUE )
					DBIndex = (unsigned long)LvItem.lParam;

				if( DBIndex != 0 )
				{
					MSG_REQ_CHEAT_GM_EVENT_RELEASE msg;
					::memset( &msg, 0, sizeof(msg) );
					msg.Category = NM_CHEAT;
					msg.Protocol = NM_CHEAT_GM_EVENT_RELEASE_REQ;
					msg.mDBIndex = DBIndex;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
				}

				SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, _T("") );
				ClearGMEventList();

			}
			else
			{
				/// 해제할 유저를 선택해주세요.
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_NOTICE, GAMERESOURCEMAN->GetGameText( 8127 ) );
			}
		}
		break;
	}
}

void cGMTool::EventInfluenceClass()
{
	/// 선택된 내용가져오기
	HWND hEventCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_COMBO_EVENT_TYPE );
	if( hEventCombo == 0 )
	{
		assert(0);
		return;
	}

	int idx = SendMessage( hEventCombo, CB_GETCURSEL, 0, 0 );
	int influenceIdx = SendMessage( hEventCombo, CB_GETITEMDATA, idx, 0 );
	if( idx != -1 && influenceIdx != -1 )
	{
		sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
		if( pInfo )
		{
			/// 설명
			cStringT temp;

			/// 효과값이 있으면 효과출력
			if( pInfo->mValue1 != 0 || pInfo->mValue2 != 0 || pInfo->mValue3 != 0 )
			{
				if( pInfo->mValue1 && pInfo->mValue2 && pInfo->mValue3 )
					temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2, pInfo->mValue3 );
				else if( pInfo->mValue1 && pInfo->mValue2 )
					temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1, pInfo->mValue2 );
				else if( pInfo->mValue1 )
					temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( pInfo->mToolTipIdx ), pInfo->mValue1 );
			}
			else
			{
				/// 일반 툴팁
				temp.Format( GAMERESOURCEMAN->GetSkillToolTipText( 100025 ), pInfo->mGroupOrder );
			}

			//설명 출력
			SetDlgItemTextW( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_EDIT_EVENT_TIP, temp.Cstr() );
		}
		else
		{
			assert(0);
			return;
		}
	}
}


void cGMTool::SetGMEventInfo( MSG_RES_CHEAT_GM_EVENT_SEARCH* pResMsg )
{
	if( pResMsg == NULL )
	{
		assert(0);
		return;
	}

	ClearGMEventList();

	if( pResMsg->mRowCount == 0 )
	{
		LVITEM LvItem;
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = 0;
		LvItem.iSubItem = 0;
		LvItem.pszText = _T("-");
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_INSERTITEM, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

		LvItem.iSubItem = 1;
		LvItem.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8099 );
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_SETITEMTEXT, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

		return;
	}

	/// 테이터 입력
	GM_EVENT_INFO* table = pResMsg->mTable;

	for( int cnt = 0; cnt < pResMsg->mRowCount; cnt++, table++ )
	{
		if( table == NULL )
		{
			assert(0);
			continue;
		}

		sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( table->influenceIdx );
		if( pInfo == NULL )
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
		//LvItem.lParam = (LPARAM)table->influenceIdx;
		LvItem.lParam = (LPARAM)table->index;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_INSERTITEM, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		// 효과 이름
		LvItem.mask = LVIF_TEXT;
		LvItem.iSubItem = 1; 
		LvItem.pszText = (LPWSTR)GAMERESOURCEMAN->GetSkillNameText( pInfo->mNameidx );
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		// 시작 시간
		struct tm date;
		localtime_s( &date, &table->startTime );

		date.tm_year = date.tm_year+1900;
		date.tm_mon  = date.tm_mon+1;
		tempText.Format(_T("[%04d/%02d/%02d]%02d:%02d:%02d"), date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec );
		LvItem.iSubItem = 2;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
		// 끝나는 시간
		localtime_s( &date, &table->validThru );

		date.tm_year = date.tm_year+1900;
		date.tm_mon  = date.tm_mon+1;
		tempText.Format(_T("[%04d/%02d/%02d]%02d:%02d:%02d"), date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec );

		LvItem.iSubItem = 3;
		LvItem.pszText = (LPWSTR)tempText.Cstr();
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_SETITEMTEXT, (WPARAM)(int)cnt, (LPARAM)(const LV_ITEM *)(&LvItem));
	}
}

void cGMTool::ClearGMEventList()
{
	/// 리스트 박스 초기화
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_DELETEALLITEMS, 0, 0 );
}