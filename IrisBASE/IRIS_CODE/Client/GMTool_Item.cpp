#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"

#include "GameResourceManager.h"
#include "ItemManager.h"
#include "Protocol_Cheat.h"
#include "Cheat_Common.h"

INT_PTR CALLBACK ItemNameEditSubProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );

void cGMTool::OnShowItem()
{
	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_ITEM];
	SetDlgItemInt( hWnd, IDC_EDIT_ITEMCOUNT, 1, false );
	SetDlgItemInt( hWnd, IDC_EDIT_MONEY, 0, false );
	SendDlgItemMessage( hWnd, IDC_EDIT_MONEY, EM_SETLIMITTEXT, (WPARAM)9, 0 );
	SendDlgItemMessage( hWnd, IDC_EDIT_ITEMCOUNT, EM_SETLIMITTEXT, (WPARAM)4, 0 );

	/// 위치세팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_ITEM].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_ITEM].y = GetSystemMetrics(SM_CYCAPTION);

}

/// 아이템 관련 버튼 이벤트
void cGMTool::OnCommandItem( int id, int msg )
{
	switch( id )
	{
	case IDC_COMBO_ITEMHIGH:
		if( msg == CBN_SELCHANGE )
		{
			EventItemHighClass();
		}
		break;
	case IDC_COMBO_ITEMMIDDLE:
		if( msg == CBN_SELCHANGE )
		{
			EventItemMiddleClass();
		}
		break;
	case IDC_COMBO_ITEMLOW:
		if( msg == CBN_SELCHANGE )
		{
			EventItemLowClass();
		}
		break;
	case IDC_COMBO_ITEMNAME:
		switch( msg )
		{
			case CBN_SELCHANGE :
				EventCardSlot();
				break;
			case CBN_EDITUPDATE:	/// 파일 검색 기능 구현
				{
					SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME, CB_SHOWDROPDOWN, (WPARAM)TRUE, 0 );

					TCHAR text[128] = { 0, };
					GetWindowText( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME ), text, 128 );

					int itemCount = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME, CB_GETCOUNT , 0, 0 );
					if( itemCount != CB_ERR )
					{
						TCHAR itemName[128] = { 0, };
						for( int index = 0; index < itemCount; ++index )
						{
							memset( itemName, 0, sizeof( itemName ) );
							if( CB_ERR != SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME, CB_GETLBTEXT , index, (LPARAM)itemName ) )
							{
								if( ::wcsstr( itemName, text ) != NULL )
								{
									SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME, CB_SETCURSEL, index, 0 );
									break;
								}
							}
						}
					}
				}
				break;
			case CBN_KILLFOCUS:
				{
 					if ( mItemNameSelected != -1 )
 						SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME, CB_SETCURSEL, mItemNameSelected, 0 );

					/// 아이템 인덱스 받아오기
					HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME );
					int sel = SendMessage( nameCombo, CB_GETCURSEL, 0, 0 );
					//unsigned long itemIndex = SendMessage( nameCombo, CB_GETITEMDATA, sel, 0 );
					SendMessage( nameCombo, CB_GETITEMDATA, sel, 0 );
				}
		}
		break;
	case IDC_BUTTON_ITEMCANCEL:
	case IDC_BUTTON_MONEYCANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_ITEM], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_ITEMCREATE: /// 아이템 생성
		if( NETWORK->IsConnected() == true )
		{
			/// 아이템 인덱스 받아오기
			HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME );
			int sel = SendMessage( nameCombo, CB_GETCURSEL, 0, 0 );
			unsigned long itemIndex = SendMessage( nameCombo, CB_GETITEMDATA, sel, 0 );
			if( sel < 0 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEM],IDC_EDIT_ITEM_NOTICE, GAMERESOURCEMAN->GetGameText( 8042 ) );
				return;
			}

			/// 강화 정보 받아오기
			HWND enhanceCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ENHANCE );
			int enhance = SendMessage( enhanceCombo, CB_GETCURSEL, 0, 0 );

			/// 수량 받아오기
			BOOL retcode;
			int count = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_EDIT_ITEMCOUNT, &retcode, false );
			if( retcode == FALSE )
				return;

			if( count > 9999 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEM],IDC_EDIT_ITEM_NOTICE, GAMERESOURCEMAN->GetGameText( 8041 ) );
				return;
			}
			if( count > 0 )
			{
				/// 아이템 잠금 검사
				if( ITEMMAN->IsLocked() )
					return;

				/// 아이템 생성 패킷
				MSG_REQ_CHEAT_MAKE msg;
				::memset( &msg, 0, sizeof( msg ) );

				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_MAKE_REQ;
				msg.mItemIndex = itemIndex;
				msg.mCount = (unsigned short)count;
				msg.mEnhance = (unsigned char)enhance;

				/// 카드장착 정보 가져오기
				unsigned int comboID[5] = { IDC_COMBO_PUTCARD1, IDC_COMBO_PUTCARD2, IDC_COMBO_PUTCARD3, IDC_COMBO_PUTCARD4, IDC_COMBO_PUTCARD5 };
				int cardSel[5];
				for( int i = 0; i < 5; ++i )
				{
					HWND combo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i] );
					cardSel[i] = SendMessage( combo, CB_GETCURSEL, 0, 0 );
					
					if( cardSel[i] > 0 ) 
					{
						msg.mCardSlot[i] = SendMessage( combo, CB_GETITEMDATA, cardSel[i], 0 );
					}
				}
				
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 초기화
				//SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_EDIT_ITEMCOUNT, 0, false );
			}
		}
		break;
	case IDC_BUTTON_MONEYCREATE:
		{
			/// 돈 받아오기
			BOOL retcode = FALSE;
			unsigned long money = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_EDIT_MONEY, &retcode, false );
			if( retcode == FALSE )
				return;

			if( money > 0 )
			{
				/// 돈 생성 패킷
				MSG_REQ_CHEAT_GOLD msg;
				::memset( &msg, 0, sizeof( msg ) );

				msg.Category = NM_CHEAT;
				msg.Protocol = NM_CHEAT_GOLD_REQ;
				msg.mValue = money;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 초기화
				//SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_EDIT_MONEY, 0, false );
			}
		}
		break;
	case IDC_EDIT_ITEMNAME:
		{
			if( msg == EN_CHANGE )
			{
				HWND itemFindList = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_LIST_ITEM_FIND_LIST );
				HWND itemFindName = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_EDIT_ITEMNAME );

				TCHAR inputText[128] = { 0, };
				GetWindowText( itemFindName, inputText, 128 );

				SendMessage( itemFindList, LB_RESETCONTENT, 0, 0 );

				if( _tcslen( inputText ) > 0 )
				{
					/// id 정보 담기
					typedef tHashMap<unsigned long, void*> cItemDefineMap;
					cItemDefineMap& itemMap = ITEMMAN->GetDefineMap();

					cItemDefineMap::cIterator mi = itemMap.Begin();
					cItemDefineMap::cIterator mend = itemMap.End();
					for( ; mi != mend; ++mi )
					{
						unsigned long itemIndex = (unsigned long)mi->mFirst;
						cItemDefine* pItemDefine = (cItemDefine*)mi->mSecond;

						if( pItemDefine )
						{

							cStringT itemName;
							itemName.Format(_T("[%04d]%s"), itemIndex, pItemDefine->GetName() );

							if( ::wcsstr( itemName.Cstr(), inputText ) != 0 )
							{
								int i = SendMessage( itemFindList, LB_ADDSTRING, 0, (LPARAM)(itemName.Cstr()));
								SendMessage( itemFindList, LB_SETITEMDATA, i, (LPARAM)itemIndex );
							}
						}
					}
					SendMessage( itemFindList, LB_SETCURSEL, 0, 0 );
				}

			}
		}
		break;
	case IDC_LIST_ITEM_FIND_LIST:
		{
			if( msg == LBN_DBLCLK )
			{
				OnItemNameSelected();
			}
		}
		break;
	default:
		return;
	}
}

bool cGMTool::LoadItemData()
{
	
	/// 대분류 콤보
	HWND highCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMHIGH );
	HWND itemSettinghighCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_HIGH );
	if( ::IsWindow( highCombo ) == false || ::IsWindow( itemSettinghighCombo ) == false )
	{
		assert(0);
		return false;
	}

	/// id 정보 담기
	typedef tHashMap<unsigned long, void*> cItemDefineMap;
	cItemDefineMap& itemMap = ITEMMAN->GetDefineMap();

	cItemDefineMap::cIterator mi = itemMap.Begin();
	cItemDefineMap::cIterator mend = itemMap.End();
	for( ; mi != mend; ++mi )
	{
		cItemDefine* pItemDefine = (cItemDefine*)mi->mSecond;

		if( pItemDefine )
		{
			tItemSubSet*  pItemSubSet = NULL;
			unsigned long itemHighType = pItemDefine->GetHighType();

			/// 대분류 설정 및 대분류-중분류 정보 연결하기
			tItemTypeMap::cIterator find = mItemTypeHighMiddleMap.Find( itemHighType );
			if( find == mItemTypeHighMiddleMap.End() )
			{
				LPCTSTR highName = GAMERESOURCEMAN->GetCategoryName( itemHighType );
				/// 아이템 생성 창
				int key = SendMessage( highCombo, CB_ADDSTRING, 0, (LPARAM)(highName));
				SendMessage( highCombo, CB_SETITEMDATA, key, (LPARAM)itemHighType );
				/// 아이템 셋팅 창
				key = SendMessage( itemSettinghighCombo, CB_ADDSTRING, 0, (LPARAM)(highName));
				SendMessage( itemSettinghighCombo, CB_SETITEMDATA, key, (LPARAM)itemHighType );

				pItemSubSet = new tItemSubSet;
				mItemTypeHighMiddleMap.Insert( itemHighType, pItemSubSet );				
			}
			else if( find != mItemTypeHighMiddleMap.End() )
				pItemSubSet = find->mSecond;

			if( pItemSubSet )
			{
				unsigned int middleKey = 0;
				unsigned int lowKey = 0;

				middleKey = pItemDefine->GetMiddleType();

				pItemSubSet->insert( middleKey );

				/// 중분류, 소분류 정보 연결하기
				lowKey = pItemDefine->GetLowType() + ( middleKey * 100000 ) ;

				tItemTypeMap::cIterator pos = mItemTypeMiddleLowMap.Find( middleKey );
				if( pos != mItemTypeMiddleLowMap.End() )
				{
					///기존의 소분류에 삽입.
					tItemSubSet* pItemLowSet = pos->mSecond; ///소분류 삽입
					if( pItemLowSet )
					{
						pItemLowSet->insert( lowKey );
					}
				}
				else
				{
					/// 새로운 소분류 생성.
					tItemSubSet*  pItemLowSet = new tItemSubSet;
					if( pItemLowSet )
					{
						pItemLowSet->insert( lowKey );
						mItemTypeMiddleLowMap.Insert( middleKey, pItemLowSet );
					}
				}

				/// 소분류, 아이템 인덱스 정보 연결하기
				pos = mItemTypeLowItemIndexMap.Find( lowKey );
				if( pos != mItemTypeLowItemIndexMap.End() )
				{
					///기존의 소분류에 삽입.
					tItemSubSet* pItemIndexSet = pos->mSecond; ///소분류 삽입
					if( pItemIndexSet )
					{
						pItemIndexSet->insert( mi->mFirst );
					}
				}
				else
				{
					/// 새로운 소분류 생성.
					tItemSubSet*  pItemIndexSet = new tItemSubSet;
					if( pItemIndexSet )
					{
						pItemIndexSet->insert( mi->mFirst );
						mItemTypeLowItemIndexMap.Insert( lowKey, pItemIndexSet );
					}
				}
			}
		}
	}
	
	/// 강화 리스트 삽입
	HWND enhanceCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ENHANCE );
	HWND ItemSettingEnhanceCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_ENHANCE );
	if( ::IsWindow( enhanceCombo ) == false || ::IsWindow( ItemSettingEnhanceCombo ) == false )
	{
		assert(0);
		return false;
	}
	
	cStringT str;
	for( int i = 0; i <= MAX_ITEM_ENHANCED; ++i )
	{
		str.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000087 ), i );
		SendMessage( enhanceCombo, CB_ADDSTRING, 0, (LPARAM)((LPCTSTR)str.Cstr()));
		SendMessage( ItemSettingEnhanceCombo, CB_ADDSTRING, 0, (LPARAM)((LPCTSTR)str.Cstr()));

	}
		
	/// 초기선택
	SendMessage( highCombo, CB_SETCURSEL, 0, 0 );
	SendMessage( itemSettinghighCombo, CB_SETCURSEL, 0, 0 );
	EventItemHighClass();
	EventItemSettingHighClass();

	return true;
}

void cGMTool::SetItemEditProc()
{
	/// 몬스터 이름 검색하기 위한 edit 박스 서브 클래싱
	HWND itemNameEidt = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_EDIT_ITEMNAME );
	mOldItemNameEditProc = (WNDPROC)SetWindowLongPtr( itemNameEidt, GWLP_WNDPROC, (LONG_PTR)ItemNameEditSubProc );
}

void cGMTool::OnDestroyItem()
{
	HWND itemNameEdit = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_EDIT_ITEMNAME );
	SetWindowLongPtr( itemNameEdit, GWLP_WNDPROC, (LONG_PTR)mOldItemNameEditProc );
}


void cGMTool::EventItemHighClass()
{
	/// 대분류 선택된 내용가져오기
	HWND highCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMHIGH);
	int i = SendMessage( highCombo, CB_GETCURSEL, 0, 0 );
	int highKey = SendMessage( highCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 초기화
	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMMIDDLE );
	SendMessage( midCombo, CB_RESETCONTENT, 0, 0 );

	/// 소분류 초기화
	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMLOW );
	SendMessage( lowCombo, CB_RESETCONTENT, 0, 0 );

	/// 이름 초기화
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME );
	SendMessage( nameCombo, CB_RESETCONTENT, 0, 0 );

	/// 중분류 세팅
	bool		 subTypeDisplay = false;
	tItemSubSet* pItemSubSet    = NULL;

	tItemTypeMap::cIterator find = mItemTypeHighMiddleMap.Find( highKey );

	if( find != mItemTypeHighMiddleMap.End() )
		pItemSubSet = find->mSecond;
	else
	{
		assert(0);
		return;
	}

	tItemSubSet::const_iterator pos;
	if( pItemSubSet )
	{
		for( pos = pItemSubSet->begin(); pos != pItemSubSet->end(); ++pos )
		{
			LPCTSTR midName = GAMERESOURCEMAN->GetCategoryName( (*pos) );
			if ( midName )
			{
				int key = SendMessage( midCombo, CB_ADDSTRING, 0, (LPARAM)(midName));
				SendMessage( midCombo, CB_SETITEMDATA, key, (LPARAM)(*pos) );
				subTypeDisplay = true;
			}
		}
	}
	/// 선택
	SendMessage( midCombo, CB_SETCURSEL, 0, 0 );

	if( subTypeDisplay )
		EventItemMiddleClass();
}

void cGMTool::EventItemMiddleClass()
{
	/// 대분류 선택된 내용가져오기
	HWND highCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMHIGH);
	int i = SendMessage( highCombo, CB_GETCURSEL, 0, 0 );
	//int highKey = SendMessage( highCombo, CB_GETITEMDATA, i, 0 );
	SendMessage( highCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 선택된 내용가져오기
	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMMIDDLE );
	i = SendMessage( midCombo, CB_GETCURSEL, 0, 0 );
	int midKey = SendMessage( midCombo, CB_GETITEMDATA, i, 0 );

	/// 소분류 초기화
	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMLOW );
	SendMessage( lowCombo, CB_RESETCONTENT, 0, 0 );

	/// 소분류 세팅
	bool		 lowTypeDisplay = false;
	tItemSubSet* pItemLowSet    = NULL;

	/// 키 계산
	tItemTypeMap::cIterator find = mItemTypeMiddleLowMap.Find( midKey );

	if( find != mItemTypeMiddleLowMap.End() )
		pItemLowSet = find->mSecond;
	else
	{
		assert(0);
		return;
	}

	tItemSubSet::const_iterator pos;
	if( pItemLowSet )
	{
		for( pos = pItemLowSet->begin(); pos != pItemLowSet->end(); ++pos )
		{

			unsigned long textIdx = (*pos) -  ( midKey * 100000 );
			LPCTSTR lowName = GAMERESOURCEMAN->GetCategoryName( textIdx );
			if ( lowName )
			{
				int key = SendMessage( lowCombo, CB_ADDSTRING, 0, (LPARAM)(lowName));
				SendMessage( lowCombo, CB_SETITEMDATA, key, (LPARAM)(*pos) );
				lowTypeDisplay = true;
			}
		}
	}
	/// 선택
	SendMessage( lowCombo, CB_SETCURSEL, 0, 0 );

 	if( lowTypeDisplay )
 		EventItemLowClass();
}

void cGMTool::EventItemLowClass()
{
	int i = 0;

	/// 대분류 선택된 내용가져오기
	HWND highCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMHIGH);
	i = SendMessage( highCombo, CB_GETCURSEL, 0, 0 );
	//int highKey = SendMessage( highCombo, CB_GETITEMDATA, i, 0 );
	SendMessage( highCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 선택된 내용가져오기
	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMMIDDLE );
	i = SendMessage( midCombo, CB_GETCURSEL, 0, 0 );
	//int midKey = SendMessage( midCombo, CB_GETITEMDATA, i, 0 );
	SendMessage( midCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 선택된 내용가져오기
	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMLOW );
	i = SendMessage( lowCombo, CB_GETCURSEL, 0, 0 );
	int lowKey = SendMessage( lowCombo, CB_GETITEMDATA, i, 0 );

	/// 아이템이름 초기화
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME );
	SendMessage( nameCombo, CB_RESETCONTENT, 0, 0 );

	//unsigned int totalCount = 0;

	///// 아이템이름 세팅
	tItemSubSet* pItemIndexSet    = NULL;
	/// 키 계산
	tItemTypeMap::cIterator find = mItemTypeLowItemIndexMap.Find( lowKey );

	if( find != mItemTypeLowItemIndexMap.End() )
		pItemIndexSet = find->mSecond;
	else
	{
		assert(0);
		return;
	}

	if( pItemIndexSet )
	{
		tItemSubSet::const_iterator pos = pItemIndexSet->begin();
		tItemSubSet::const_iterator iEnd   = pItemIndexSet->end();
		int textMaxLength = 0;

		for( ; pos != iEnd; ++pos )
		{
			cItemDefine* pItemDefine = NULL;
			pItemDefine = ITEMMAN->GetItemDefine( (*pos) );

			if( pItemDefine )
			{
				cStringT str;
				str.Format(_T("[%07d]%s"), (*pos), pItemDefine->GetName());
				textMaxLength = __max( textMaxLength, (int)str.GetLength() );
				int key = SendMessage( nameCombo, CB_ADDSTRING, 0, (LPARAM)(str.Cstr()));
				SendMessage( nameCombo, CB_SETITEMDATA, key, (LPARAM)(*pos) );
			}
		}
		/// 이름 폭 조정
		textMaxLength *= 10;
		SendMessage( nameCombo, CB_SETDROPPEDWIDTH, (WPARAM)textMaxLength, 0 );
		
	}

	SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ENHANCE), CB_SETCURSEL, 0, 0 );

	// 그외 세팅
	//int sel = SendMessage( nameCombo, CB_SETCURSEL, 0, 0 );
	SendMessage( nameCombo, CB_SETCURSEL, 0, 0 );
 	unsigned long setItemIndex = SendMessage( nameCombo, CB_GETITEMDATA, i, 0 );
 	cItemDefine* define = ITEMMAN->GetItemDefine( setItemIndex );
 	if( define )
 	{
 		unsigned char type = define->GetType();
 		bool enable = ( !(type == ITEM_WEAPON || type == ITEM_WEAR) ) ? false : true;
 		EnableWindow( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ENHANCE ), enable );
 	}
 
 	/// 카드장착 슬롯 세팅
 	EventCardSlot();

}

/// 카드장착 슬롯 세팅
void cGMTool::EventCardSlot()
{
	/// id 정보 담기
	typedef tHashMap<unsigned long, void*> cItemDefineMap;
	cItemDefineMap& itemMap = ITEMMAN->GetDefineMap();

	unsigned int comboID[5] = { IDC_COMBO_PUTCARD1, IDC_COMBO_PUTCARD2, IDC_COMBO_PUTCARD3, IDC_COMBO_PUTCARD4, IDC_COMBO_PUTCARD5 };
	unsigned int slotID[5] = { IDC_EDIT_SLOTTYPE1, IDC_EDIT_SLOTTYPE2, IDC_EDIT_SLOTTYPE3, IDC_EDIT_SLOTTYPE4, IDC_EDIT_SLOTTYPE5 };

	/// 카드콤보
	for( int i = 0; i < 5; ++i )
	{
		EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i] ), false );
		SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i] ), CB_RESETCONTENT, 0, 0 );
		SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i]), CB_ADDSTRING, 0, (LPARAM)_T("-------------------------"));
	}

	/// 슬롯 카드 타입 초기화
	for( int i = 0; i < 5; ++i )
	{
		SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEM], slotID[i], _T("") );
	}

	/// 아이템 인덱스 받아오기
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME );
	int sel = SendMessage( nameCombo, CB_GETCURSEL, 0, 0 );
	unsigned long selItemIdx = SendMessage( nameCombo, CB_GETITEMDATA, sel, 0 );
	
	cItemDefine* define = ITEMMAN->GetItemDefine( selItemIdx );
	if( !define )
	{
		assert(0);
		return;
	}

	/// 장착용 아이템이 아니면 리턴
	unsigned char type = define->GetType();
	if( !(type == ITEM_WEAPON || type == ITEM_WEAR) )
		return;

	/// 슬롯 정보 세팅
	sItemPutCardSlot* data = define->GetPutCard();
	int count = define->GetPutSlotCount();
	if( data && count > 0 )
	{
		LPCTSTR str = 0;
		for( int i = 0; i < count; ++i )
		{
			/// 1. 카드 슬롯 타입 세팅
			switch( data->mSlotType[i] )
			{
			case ITEM_CARD_SLOT_NONE:	continue;
			case ITEM_CARD_SLOT_A:		str = _T("A");	break;
			case ITEM_CARD_SLOT_B:		str = _T("B");	break;
			case ITEM_CARD_SLOT_AB:		str = _T("AB");	break;
			case ITEM_CARD_SLOT_O:		str = _T("O");	break;
			}

			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEM], slotID[i], str );
			EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i]), true );
			SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i] ), CB_SETCURSEL, 0, 0 );

			/// 2. 장착가능한 카드 콤보 세팅
			cItemDefineMap::cIterator mi = itemMap.Begin();
			cItemDefineMap::cIterator mend = itemMap.End();
			for( ; mi != mend; ++mi )
			{
				unsigned long cardIndex = (unsigned long)(mi->mFirst);
				cItemDefine* cardDefine = ITEMMAN->GetItemDefine( cardIndex );
				if( !cardDefine )
					continue;

				/// 장착용 카드 아니면 리턴
				if( cardDefine->GetType() != ITEM_CARD )
					continue;

				/// 혈액형 타입 카드 아니면 리턴
				if( !(cardDefine->GetSubType() == ITEM_CARD_EQUIP_A || 
					 cardDefine->GetSubType() == ITEM_CARD_EQUIP_B || 
					 cardDefine->GetSubType() == ITEM_CARD_EQUIP_AB ||
					 cardDefine->GetSubType() == ITEM_CARD_EQUIP_O ) )
					continue;
					
				/// 해당 슬롯에 들어갈수 있는지 판단
				short cardType = cardDefine->GetSubType();
				if( data->mSlotType[i] == ITEM_CARD_SLOT_A )
				{
					if( !(cardType == ITEM_CARD_EQUIP_A || cardType == ITEM_CARD_EQUIP_O) )
						continue;
				}
				else if( data->mSlotType[i] == ITEM_CARD_SLOT_B )
				{
					if( !(cardType == ITEM_CARD_EQUIP_B || cardType == ITEM_CARD_EQUIP_O) )
						continue;
				}
				else if( data->mSlotType[i] == ITEM_CARD_SLOT_O )
				{
					if( cardType != ITEM_CARD_EQUIP_O )
						continue;
				}
				
				/// 세팅
				LPCTSTR name = ITEMMAN->GetItemName( cardIndex );
				int key = SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i]), CB_ADDSTRING, 0, (LPARAM)(name));
				SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], comboID[i]), CB_SETITEMDATA, key, (LPARAM)cardIndex );
			}
		}
	}
}

void cGMTool::OnItemNameSelected()
{
	HWND hItemFindList = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_LIST_ITEM_FIND_LIST );
	int i = SendMessage( hItemFindList, LB_GETCURSEL, 0, 0 );
	long itemIndex = SendMessage( hItemFindList, LB_GETITEMDATA, i, 0 );

	if( i == LB_ERR || itemIndex == LB_ERR )
	{
		return;
	}

	cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( itemIndex );
	if( !pItemDefine )
	{
		assert(0);
		return;
	}

	HWND highCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMHIGH );
	int count = SendMessage( highCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long comboIndex = SendMessage( highCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == (long)pItemDefine->GetHighType() )
		{
			SendMessage( highCombo, CB_SETCURSEL, i, 0 );
			EventItemHighClass();
			break;
		}
	}
	if( i == count )
	{
		assert(0);
		return;
	}

	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMMIDDLE );
	count = SendMessage( midCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long comboIndex = SendMessage( midCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == (long)pItemDefine->GetMiddleType() )
		{
			SendMessage( midCombo, CB_SETCURSEL, i, 0 );
			EventItemMiddleClass();
			break;
		}
	}
	if( i == count )
	{
		assert(0);
		return;
	}

	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMLOW );
	count = SendMessage( lowCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long lowKey = pItemDefine->GetLowType() + ( pItemDefine->GetMiddleType() * 100000 ) ;
		long comboIndex = SendMessage( lowCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == lowKey )
		{
			SendMessage( lowCombo, CB_SETCURSEL, i, 0 );
			EventItemLowClass();
			break;
		}
	}
	if( i == count )
	{
		assert(0);
		return;
	}

	/// 이름 초기화
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_COMBO_ITEMNAME );
	count = SendMessage( nameCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long comboIndex = SendMessage( nameCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == itemIndex )
		{
			SendMessage( nameCombo, CB_SETCURSEL, i, 0 );
			EventCardSlot();
			break;
		}
	}
}

void cGMTool::SendMessageToItemList( UINT msg, WPARAM wparam, LPARAM lparam )
{
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEM], IDC_LIST_ITEM_FIND_LIST, msg, wparam, lparam );
}

/// 아이템 검색 에디트 박스 서브 클래싱
INT_PTR CALLBACK ItemNameEditSubProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_KEYDOWN:
		switch(wparam)	
		{
			/// 엔터키 입력시 아이템 콤보 셋팅.
		case VK_RETURN:
			{
				GMTOOL->OnItemNameSelected();	
			}
			return 0;

			/// 방향키 입력시 리스트 박스에 있는 요소 상/하 이동하기.
		case VK_UP:
		case VK_DOWN:
		case VK_PRIOR:
		case VK_NEXT:
			{
				GMTOOL->SendMessageToItemList( msg, wparam, lparam );
			}
			return 0;

		}
		break;
	}

	return CallWindowProc( GMTOOL->GetOldItemEditProc(), wnd, msg, wparam, lparam );
}