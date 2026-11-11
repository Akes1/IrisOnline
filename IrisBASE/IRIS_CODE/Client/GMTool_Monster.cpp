#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"

#include "Monster_Common.h"
#include "MonsterScript.h"
#include "Protocol_Cheat.h"
#include "Cheat_Common.h"

#include "ObjectManager.h"
#include "Hero.h"

INT_PTR CALLBACK MonsterNameEditSubProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );

void cGMTool::OnShowMonster()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_MONSTER];
		
	/// 위치세팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_MONSTER].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_MONSTER].y = GetSystemMetrics(SM_CYCAPTION);

	/// 에디트박스 기본 값 설정
	SetDlgItemInt( hWnd, IDC_EDIT_MONSTERCOUNT, 1, false );
	SetFocus( GetDlgItem( hWnd, IDC_EDIT_MONNAME ) );
}

/// 몬스터 관련 이벤트 
void cGMTool::OnCommandMonster( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_MONCANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_MONSTER], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_MONCREATE:	/// 몬스터 생성
		{
			OnCreateMonsterButtonDown();
		}
		break;
	case IDC_BUTTON_MONDELETE:	/// 몬스터 제거
		if( NETWORK->IsConnected() == true )
		{
			cBaseObject* pTarget = HERO->GetTargetObject();
			if( pTarget == NULL )
				return;

			if( pTarget->IsDie() == true )
				return;

			if( pTarget->GetObjectType() != eOBJECTTYPE_MONSTER )
				return;

			MSG_REQ_CHEAT_REMOVE msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_REMOVE_REQ;
			msg.mObjIndex = pTarget->GetObjectID();
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case IDC_BUTTON_MONKILL:	/// 몬스터 사냥 ( 드랍 & 경헙치 있음 )
		if( NETWORK->IsConnected() == true )
		{
			cBaseObject* pTarget = HERO->GetTargetObject();
			if( pTarget == NULL )
				return;

			if( pTarget->IsDie() == true )
				return;

			if( pTarget->GetObjectType() != eOBJECTTYPE_MONSTER )
				return;

			MSG_REQ_CHEAT_KILL msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_KILL_REQ;
			msg.mObjIndex = pTarget->GetObjectID();
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case IDC_BUTTON_MONDELETE_RANGE:	/// 몬스터 일정 반경 제거
		if( NETWORK->IsConnected() == true )
		{
			MSG_REQ_CHEAT_REMOVE_MONSTER msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_REMOVE_MONSTER_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case IDC_BUTTON_MONKILL_RANGE:
		if( NETWORK->IsConnected() == true )
		{
			MSGROOT msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_KILL_MONSTER_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case IDC_EDIT_MONNAME:
		{
			if( msg == EN_CHANGE )
			{
				HWND monList = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_LIST_MONLIST );

				TCHAR text[128] = { 0, };
				GetWindowText( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_EDIT_MONNAME ), text, 128 );

				SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_LIST_MONLIST , LB_RESETCONTENT, 0, 0 );
				
				cMonsterHashMap::cConstIterator i = mMonsterMap->Begin();
				cMonsterHashMap::cConstIterator end = mMonsterMap->End();

				for( int cnt= 0; i != end; ++cnt ,++i )
				{
					sMonsterScript* p = (sMonsterScript*)(i->mSecond);
					if( p )
					{
						cStringT monName;
						monName.Format(_T("[%04d]%s"), p->mMonsterClassIdx, p->mName );

						if( ::wcsstr( monName.Cstr(), text ) != 0 )
						{
							int i = SendMessage( monList, LB_ADDSTRING, 0, (LPARAM)(monName.Cstr()));
							SendMessage( monList, LB_SETITEMDATA, i, (LPARAM)p->mMonsterClassIdx );
						}
					}
				}

				SendMessage( monList, LB_SETCURSEL, 0, 0 );
			}
		}
		break;
	case IDC_LIST_MONLIST:
		{
			if( msg == LBN_DBLCLK )
			{
				OnCreateMonsterButtonDown();
			}
		}
		break;
	}
	
}

bool cGMTool::LoadMonsterData()
{
	/// 몬스터 리스트 받기
	HWND nameList = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_LIST_MONLIST );
	if( ::IsWindow( nameList ) == false )
	{
		assert(0);
		return false;
	}

	/// 몬스터 세팅
	mMonsterMap = MONSTERSCRIPT->GetMonsterMap();
	if( !mMonsterMap )
		return false;

	cMonsterHashMap::cConstIterator i = mMonsterMap->Begin();
	cMonsterHashMap::cConstIterator end = mMonsterMap->End();

	for( ; i != end; ++i )
	{
		sMonsterScript* p = (sMonsterScript*)(i->mSecond);
		if( p )
		{
			cStringT monName;
			monName.Format(_T("[%04d]%s"), p->mMonsterClassIdx, p->mName );
			int i = SendMessage( nameList, LB_ADDSTRING, 0, (LPARAM)(monName.Cstr()));
			SendMessage( nameList, LB_SETITEMDATA, i, (LPARAM)p->mMonsterClassIdx );
		}
	}

	/// 콤보 박스 기본 선택 셋팅
	SendMessage( nameList, LB_SETCURSEL, 0, 0 );

	return true;
}

void cGMTool::SetMonsterEditProc()
{
	/// 몬스터 이름 검색하기 위한 edit 박스 서브 클래싱
	HWND monsterNameEidt = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_EDIT_MONNAME );
	mOldMonsterNameEditProc = (WNDPROC)SetWindowLongPtr( monsterNameEidt, GWLP_WNDPROC, (LONG_PTR)MonsterNameEditSubProc );
}

void cGMTool::OnDestroyMonster()
{
	HWND monsterNameEdit = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_EDIT_MONNAME );
	SetWindowLongPtr( monsterNameEdit, GWLP_WNDPROC, (LONG_PTR)mOldMonsterNameEditProc );
}

void cGMTool::OnCreateMonsterButtonDown()
{
	/// 엔터키 입력시 선택된 몬스터 소환하기.
	if( NETWORK->IsConnected() == true )
	{
		/// 몬스터 인덱스 받기
		HWND hMonList = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_LIST_MONLIST );
		int i = SendMessage( hMonList, LB_GETCURSEL, 0, 0 );
		long monIdx = SendMessage( hMonList, LB_GETITEMDATA, i, 0 );

		if( i == LB_ERR || monIdx == LB_ERR )
			return;

		/// 수량 받기
		BOOL retcode;
		int count = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_EDIT_MONSTERCOUNT, &retcode, false );
		if( retcode == false )
			return;

		if( count == 0 )
			return;

		if( count > 128 )
		{
			SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_EDIT_MONSTERCOUNT, 128, false );
			count = 128;
		}

		/// 생성
		MSG_REQ_CHEAT_SUMMON msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_SUMMON_REQ;
		msg.mMonsterNum = monIdx;
		msg.mValue = (unsigned char)count;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 초기화
		//SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_EDIT_MONSTERCOUNT, 1, false );
	}
};

void cGMTool::SendMessageToMonsterList( UINT msg, WPARAM wparam, LPARAM lparam )
{
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MONSTER], IDC_LIST_MONLIST, msg, wparam, lparam );
}

/// 몬스터 에디트 박스 서브 클래싱
INT_PTR CALLBACK MonsterNameEditSubProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
		case WM_KEYDOWN:
			switch(wparam)	
			{
				/// 엔터키 입력시 몬스터 생성 버튼 클릭.
				case VK_RETURN:
					{
						GMTOOL->OnCreateMonsterButtonDown();	
					}
					return 0;

				/// 방향키 입력시 리스트 박스에 있는 요소 상/하 이동하기.
				case VK_UP:
				case VK_DOWN:
				case VK_PRIOR:
				case VK_NEXT:
					{
						GMTOOL->SendMessageToMonsterList( msg, wparam, lparam );
					}
					return 0;
				
			}
			break;
	}
	
	return CallWindowProc( GMTOOL->GetOldEditProc(), wnd, msg, wparam, lparam );
}