#include "stdafx.h"
#include <CommCtrl.h>
#include "GMTool.h"
#include "resource.h"
#include "GameResourceManager.h"
#include "UIManager.h"
#include "ItemManager.h"
#include "Protocol_Cheat.h"

INT_PTR CALLBACK SettingFindNameEditSubProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );

void cGMTool::OnShowItemSetting()
{
	/// 위치세팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
	mSubDlgPos[GMTOOL_SUBDLG_ITEMSETTING].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_ITEMSETTING].y = GetSystemMetrics(SM_CYCAPTION);

	/// 최대 값 제한
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_COUNT, EM_SETLIMITTEXT, (WPARAM)4, 0 );
	
 	SetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_COUNT, 1, false );

	SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_PATH, mCurrentPath );
}

void cGMTool::OnCommandItemSetting(int id, int msg)
{
	switch( id )
	{
	case IDC_BUTTON_ITEMSETTING_CANCEL:
	case IDC_BUTTON_ITEMSETTING_CANCEL2:
		{
			::ShowWindow( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], SW_HIDE );
			::SetActiveWindow( mParentWnd );
			///안내 메시지 초기화
			SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );
		}
		break;

	case IDC_COMBO_ITEMSETTING_HIGH:
		if( msg == CBN_SELCHANGE )
		{
			EventItemSettingHighClass();
		}
		break;
	case IDC_COMBO_ITEMSETTING_MIDDLE:
		if( msg == CBN_SELCHANGE )
		{
			EventItemSettingMiddleClass();
		}
		break;
	case IDC_COMBO_ITEMSETTING_LOW:
		if( msg == CBN_SELCHANGE )
		{
			EventItemSettingLowClass();
		}
		break;
	case IDC_COMBO_ITEMSETTING_NAME:
		switch( msg )
		{
		case CBN_SELCHANGE :
			EventItemSettingCardSlot();
			break;
		}
		break;
	case IDC_BUTTON_ITEMSETTING_MAKE:	///아이템 생성 버튼
		{
			///안내 메시지 초기화
			SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

			HWND listViewControl = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST );
			int itemMax = ListView_GetItemCount( listViewControl);
			
			if( itemMax < 1 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8047 ) );
				return;
			}

			char* pReqMsgBuff = new char[ sizeof( MSG_REQ_CHEAT_MAKE_ITEMS ) + (itemMax-1)*sizeof(sCheat_Make_Item_Info) ];
			if( !pReqMsgBuff )
			{
				assert( 0 && "Fail to memory Allocation" );
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8054 ) );
				return;
			}
			::memset( pReqMsgBuff, 0, sizeof( pReqMsgBuff ) );
			MSG_REQ_CHEAT_MAKE_ITEMS* pMsg = (MSG_REQ_CHEAT_MAKE_ITEMS*)pReqMsgBuff;
			sCheat_Make_Item_Info*    pItemInfo = pMsg->mItemInfo;
			pMsg->mCount = (unsigned char)itemMax;

			for( int cnt = 0; cnt < itemMax; ++cnt, pItemInfo++ )
			{
				sCheat_Make_Item_Info* pMsg = NULL;
				LVITEM LvItem;
				LvItem.mask = LVIF_PARAM;
				LvItem.iItem = cnt;
				ListView_GetItem( listViewControl, &LvItem );
				
				pMsg = (sCheat_Make_Item_Info*)LvItem.lParam;
				if( pMsg )
				{
					::memcpy_s( pItemInfo, sizeof( sCheat_Make_Item_Info ), pMsg, sizeof( sCheat_Make_Item_Info ) );
				}
			}

			if( NETWORK->IsConnected() )
			{
				pMsg->Category = NM_CHEAT;
				pMsg->Protocol = NM_CHEAT_MAKE_ITEMS_REQ;
				NETWORK->SendNetworkMsg( (char*)pMsg, (unsigned short)pMsg->GetMsgLength() );
			}
			
			delete pReqMsgBuff;

		}
		break;
	case IDC_BUTTON_ITEMSETTING_ADD:	///목록에 아이템 추가 버튼
		{
			///안내 메시지 초기화
			SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

			/// 아이템 종류 최대 갯수 제한하기
			LRESULT ret = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_GETITEMCOUNT, 0, 0 );
			if( ret > 99 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8055 ) );
				return;
			}
			
			/// 아이템 인덱스 받아오기
			HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_NAME );
			int sel = SendMessage( nameCombo, CB_GETCURSEL, 0, 0 );
			unsigned long itemIndex = SendMessage( nameCombo, CB_GETITEMDATA, sel, 0 );
			if( sel < 0 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8042 ) );
				return;
			}

			/// 강화 정보 받아오기
			HWND enhanceCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_ENHANCE );
			int enhance = SendMessage( enhanceCombo, CB_GETCURSEL, 0, 0 );

			/// 수량 받아오기
			BOOL retcode;
			int count = GetDlgItemInt( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_COUNT, &retcode, false );
			if( retcode == FALSE )
				return;

			if( count > 9999 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8041 ) );
				return;
			}
			if( count > 0 )
			{
				/// 아이템 정보
				sCheat_Make_Item_Info* pMsg;
				pMsg = new sCheat_Make_Item_Info;
				if( !pMsg )
				{
					assert( 0 && "fail to memory allocation ");
					return;
				}
				
				/// 카드장착 정보 가져오기
				unsigned int comboID[5] = { IDC_COMBO_PUTCARD1, IDC_COMBO_PUTCARD2, IDC_COMBO_PUTCARD3, IDC_COMBO_PUTCARD4, IDC_COMBO_PUTCARD5 };
				int cardSel[5];
				for( int i = 0; i < 5; ++i )
				{
					HWND combo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i] );
					cardSel[i] = SendMessage( combo, CB_GETCURSEL, 0, 0 );

					if( cardSel[i] > 0 ) 
					{
						pMsg->mCardSlot[i] = SendMessage( combo, CB_GETITEMDATA, cardSel[i], 0 );
					}
					else
						pMsg->mCardSlot[i] = 0;
				}
				pMsg->mItemIndex = itemIndex;
				pMsg->mCount	   = (unsigned short)count;
				pMsg->mEnhance   = (unsigned char)enhance;

				cItemDefine* pItemDefine;
				pItemDefine = ITEMMAN->GetItemDefine( itemIndex );
				if( !pItemDefine )	
				{
					assert( 0 && "bad item index" );
					delete pMsg;
					return;
				}
				cStringT itemName;
				if( enhance )
					itemName.Format(_T("+%d%s"), enhance, pItemDefine->GetName() );
				else
					itemName.Format(_T("%s"), pItemDefine->GetName() );

				LVITEM LvItem;
				LvItem.mask = LVIF_TEXT | LVIF_PARAM;
				LvItem.iItem = 0;
				LvItem.iSubItem = 0;
				LvItem.pszText = (LPWSTR)itemName.Cstr();
				LvItem.lParam = (LPARAM)pMsg;

				SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_INSERTITEM, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));
				
				cStringT countText;
				countText.Format(_T("%d"), count);
				LvItem.iSubItem = 1;
				LvItem.pszText = (LPWSTR)countText.Cstr();
				SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_SETITEMTEXT, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));
						
			}
		}
		break;
	case IDC_BUTTON_ITEMSETTING_DELETE:	/// 목록에 아이템 삭제
		{
			///안내 메시지 초기화
			SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

			int selectIdx = SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_GETNEXTITEM, (WPARAM)(int)-1, (LPARAM)( LVNI_ALL | LVNI_SELECTED ) );

			if( selectIdx == -1 )
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8045 ) );
			}
			else
			{
				//HWND wnd = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST );
				sCheat_Make_Item_Info* pMsg = NULL;
				LVITEM LvItem;
				LvItem.mask = LVIF_PARAM;
				LvItem.iItem = selectIdx;
				SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_GETITEM, 0, (LPARAM)&LvItem );

				pMsg = (sCheat_Make_Item_Info*)LvItem.lParam;
 				if( pMsg )
 				{
 					SAFE_DELETE(pMsg);
 				}

				SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_DELETEITEM, (WPARAM)(int)selectIdx, 0);
			}
		}
		break;
	case IDC_BUTTON_ITEMSETTING_DELETEALL: /// 목록에서 아이템 모두 삭제.
		{
			///안내 메시지 초기화
			SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

			DeleteAllItemList();
		}
		break;
	case IDC_BUTTON_ITEMSETTING_SAVE:	/// 파일 저장
		{
			///안내 메시지 초기화
			SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

			HWND listViewControl = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST );
			int itemMax = ListView_GetItemCount( listViewControl);

			char fileName[128] = {0,};
			if( GetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_FILENAME, fileName, sizeof(fileName) ) != 0 )
			{
				char fullPath[MAX_PATH] = {0,};
				sprintf_s( fullPath, "%s\\%s.item", mCurrentPath, fileName );

				cFileSaver fileSaver;
				if( fileSaver.Open( fullPath ) == true )
				{
					/// 아이템 정보 얻어오기.
					for( int cnt = 0; cnt < itemMax; ++cnt )
					{
						sCheat_Make_Item_Info* pMsg = NULL;
						LVITEM LvItem;
						LvItem.mask = LVIF_PARAM;
						LvItem.iItem = cnt;
						if( ListView_GetItem( listViewControl, &LvItem ) == TRUE )
						{
							pMsg = (sCheat_Make_Item_Info*)LvItem.lParam;
							if( fileSaver.Write( (char*)pMsg, sizeof(sCheat_Make_Item_Info) ) != sizeof(sCheat_Make_Item_Info ) )
								assert( 0 && "Bad file write" );
						}
					}
				}
				if( fileSaver.Close() == -1 )
					assert(0);
			}
			else
			{
				SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8048 ) );
			}
			
			/// 파일 리스트 화면 갱신
			{
				char findPath[ MAX_PATH ] = { 0, };
				sprintf_s( findPath, "%s\\*.item", mCurrentPath );

				DlgDirListA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], findPath, IDC_LIST_ITEMSETTING_FILELIST, 0, (DDL_ARCHIVE | DDL_DRIVES | DDL_DIRECTORY | DDL_READWRITE ) );
			}
		}
		break;
	case IDC_BUTTON_ITEMSETTING_FILESEARCH: ///파일 검색
		{
			///안내 메시지 초기화
			SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

			char findPath[ MAX_PATH ] = { 0, };
			sprintf_s( findPath, "%s\\*.item", mCurrentPath );

			DlgDirListA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], findPath, IDC_LIST_ITEMSETTING_FILELIST, 0, (DDL_ARCHIVE | DDL_DRIVES | DDL_DIRECTORY | DDL_READWRITE ) );
		}
		break;
	case IDC_LIST_ITEMSETTING_FILELIST:		///파일 리스트 더블 클릭
		{
			switch( msg )
			{
			case LBN_SELCHANGE:
				{
					BOOL isFolder;
					char selectedName[MAX_PATH] = { 0, };

					///안내 메시지 초기화
					SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

					isFolder = DlgDirSelectExA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], selectedName, MAX_PATH, IDC_LIST_ITEMSETTING_FILELIST );

					/// 폴더 선택 시 파일 이름 지우기.
					if( isFolder )
						SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_FILENAME, "");
					else
					{
						/// 파일 선택 시 파일 이름을 .item를 빼고 저장이름으로 셋팅
						char* pItemPos = NULL;
						pItemPos = strstr( selectedName, ".item" );
						if( pItemPos != 0 )
							*pItemPos = 0;
						
						SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_FILENAME, selectedName );
					}
						
				}
				break;
			case LBN_DBLCLK:
				{
					BOOL isFolder;
					char selectedName[MAX_PATH] = { 0, };
					char path[MAX_PATH] = { 0, };

					///안내 메시지 초기화
					SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, "" );

					isFolder = DlgDirSelectExA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], selectedName, MAX_PATH, IDC_LIST_ITEMSETTING_FILELIST );

					///폴더 선택
					if( isFolder )
					{
						if(lstrcmpA(selectedName, "..\\") == 0)
						{
							char *p;
							p = strrchr( mCurrentPath, '\\' );
							*p = 0;
						}
						else if( selectedName[1] == ':' )
						{
							sprintf_s( mCurrentPath, "%c:\\", selectedName[0] );
						}
						else
						{
							sprintf_s( path, "%s\\%s", mCurrentPath, selectedName );
							path[::lstrlenA(path)-1]=0;
							::lstrcpyA(mCurrentPath, path );
						}
						sprintf_s( path, "%s\\*.item", mCurrentPath );

						/// 비어있는 드라이브로 이동할 경우 DDL_DRIVES 옵션이 DDL_EXECLUSIVE를 호출하여 리스트가 생성되지 않으므로 2번 호출
						DlgDirListA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], path, IDC_LIST_ITEMSETTING_FILELIST, 0, ( DDL_DRIVES ) );
						DlgDirListA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], path, IDC_LIST_ITEMSETTING_FILELIST, 0, ( DDL_ARCHIVE | DDL_DRIVES | DDL_DIRECTORY | DDL_READWRITE ) );

						SetDlgItemTextA( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_PATH, mCurrentPath );
					}
					else
					{

						if( mIsItemSettingfileOpen == true )
						{
							if ( mItemSettingFileLoader.Close() == -1 )
							{
								SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8065 ) );
								break;
							}
						}

						/// 파일 입력시 .item파일을 읽어오기
 						sprintf_s( path, "%s\\%s", mCurrentPath, selectedName );
 
 						if( mItemSettingFileLoader.Open( path, false ) == false )
						{
							SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8065 ) );
 							break;
						}
 
 						mIsItemSettingfileOpen = true;
												
						/// 아이템 리스트 초기화
						DeleteAllItemList();

						while( mItemSettingFileLoader.IsEnd() == false )
						{
							/// 아이템 생성 패킷
							sCheat_Make_Item_Info* pMsg;
							pMsg = new sCheat_Make_Item_Info;
							if( !pMsg )
							{
								assert( 0 && "fail to allocation");
								return;
							}

							int msgSize = sizeof( sCheat_Make_Item_Info );
							int readByte = 0;
							
							readByte =  mItemSettingFileLoader.Read( pMsg, msgSize );
							if( readByte != msgSize )							
							{						
								if( readByte != 0 )
								{
									assert( 0 && "bad file read" );
									SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8046 ) );
								}

								delete pMsg;
								mItemSettingFileLoader.Close();
								mIsItemSettingfileOpen = false;
								return;
							}

							cItemDefine* pItemDefine;
							pItemDefine = ITEMMAN->GetItemDefine( pMsg->mItemIndex );
							if( !pItemDefine )
							{
								assert( 0 && "bad item index" );
								delete pMsg;
								mItemSettingFileLoader.Close();
								mIsItemSettingfileOpen = false;
								SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING],IDC_EDIT_ITEMSETTING_NOTICE, GAMERESOURCEMAN->GetGameText( 8046 ) );
								return;
							}
							cStringT itemName;
							if( pMsg->mEnhance )
								itemName.Format(_T("+%d%s"), pMsg->mEnhance, pItemDefine->GetName() );
							else
								itemName.Format(_T("%s"), pItemDefine->GetName() );

							LVITEM LvItem;
							LvItem.mask = LVIF_TEXT | LVIF_PARAM;
							LvItem.iItem = 0;
							LvItem.iSubItem = 0;
							LvItem.pszText = (LPWSTR)itemName.Cstr();
							LvItem.lParam = (LPARAM)pMsg;

							SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_INSERTITEM, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

							cStringT countText;
							countText.Format(_T("%d"), pMsg->mCount );
							LvItem.iSubItem = 1;
							LvItem.pszText = (LPWSTR)countText.Cstr();
							SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_SETITEMTEXT, (WPARAM)(int)0, (LPARAM)(const LV_ITEM *)(&LvItem));

						}

						/// 파일 닫기
						mItemSettingFileLoader.Close();
						mIsItemSettingfileOpen = false;
					}

				}
				break;
			}
		}
		break;
	case IDC_EDIT_ITEMSETTING_FINDNAME:
		{
			if( msg == EN_CHANGE )
			{
				HWND itemFindList = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LIST_ITEMSETTING_FIND_LIST );
				HWND itemFindName = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_FINDNAME );

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
	case IDC_LIST_ITEMSETTING_FIND_LIST:
		{
			if( msg == LBN_DBLCLK )
			{
				OnSettingFindNameSelected();
			}
		}
		break;
	}
}

void cGMTool::SetItemSettingEditProc()
{
	/// 몬스터 이름 검색하기 위한 edit 박스 서브 클래싱
	HWND itemNameEidt = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_FINDNAME );
	mOldSettingFindNameEditProc = (WNDPROC)SetWindowLongPtr( itemNameEidt, GWLP_WNDPROC, (LONG_PTR)SettingFindNameEditSubProc );
}

void cGMTool::OnDestroyItemSetting()
{
	HWND itemNameEdit = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_EDIT_ITEMSETTING_FINDNAME );
	SetWindowLongPtr( itemNameEdit, GWLP_WNDPROC, (LONG_PTR)mOldSettingFindNameEditProc );
}

void cGMTool::EventItemSettingHighClass()
{
	/// 대분류 선택된 내용가져오기
	HWND highCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_HIGH);
	int i = SendMessage( highCombo, CB_GETCURSEL, 0, 0 );
	int highKey = SendMessage( highCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 초기화
	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_MIDDLE );
	SendMessage( midCombo, CB_RESETCONTENT, 0, 0 );

	/// 소분류 초기화
	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_LOW );
	SendMessage( lowCombo, CB_RESETCONTENT, 0, 0 );

	/// 이름 초기화
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_NAME );
	SendMessage( nameCombo , CB_RESETCONTENT, 0, 0 );

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
		EventItemSettingMiddleClass();
}

void cGMTool::EventItemSettingMiddleClass()
{
	/// 대분류 선택된 내용가져오기
	HWND highCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_HIGH);
	int i = SendMessage( highCombo, CB_GETCURSEL, 0, 0 );
	//int highKey = SendMessage( highCombo, CB_GETITEMDATA, i, 0 );
	SendMessage( highCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 선택된 내용가져오기
	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_MIDDLE );
	i = SendMessage( midCombo, CB_GETCURSEL, 0, 0 );
	int midKey = SendMessage( midCombo, CB_GETITEMDATA, i, 0 );

	/// 소분류 초기화
	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_LOW );
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

			unsigned long textIdx = (*pos)  -  ( midKey * 100000 );
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
		EventItemSettingLowClass();
}

void cGMTool::EventItemSettingLowClass()
{
	int i = 0;

	/// 대분류 선택된 내용가져오기
	HWND highCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_HIGH);
	i = SendMessage( highCombo, CB_GETCURSEL, 0, 0 );
	//int highKey = SendMessage( highCombo, CB_GETITEMDATA, i, 0 );
	SendMessage( highCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 선택된 내용가져오기
	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_MIDDLE );
	i = SendMessage( midCombo, CB_GETCURSEL, 0, 0 );
	//int midKey = SendMessage( midCombo, CB_GETITEMDATA, i, 0 );
	SendMessage( midCombo, CB_GETITEMDATA, i, 0 );

	/// 중분류 선택된 내용가져오기
	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_LOW );
	i = SendMessage( lowCombo, CB_GETCURSEL, 0, 0 );
	int lowKey = SendMessage( lowCombo, CB_GETITEMDATA, i, 0 );

	/// 아이템이름 초기화
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_NAME );
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

	SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_ENHANCE), CB_SETCURSEL, 0, 0 );

	// 그외 세팅
	//int sel = SendMessage( nameCombo, CB_SETCURSEL, 0, 0 );
	SendMessage( nameCombo, CB_SETCURSEL, 0, 0 );
	unsigned long setItemIndex = SendMessage( nameCombo, CB_GETITEMDATA, i, 0 );
	cItemDefine* define = ITEMMAN->GetItemDefine( setItemIndex );
	if( define )
	{
		unsigned char type = define->GetType();
		bool enable = ( !(type == ITEM_WEAPON || type == ITEM_WEAR) ) ? false : true;
		EnableWindow( GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_ENHANCE ), enable );
	}

	/// 카드장착 슬롯 세팅
	EventItemSettingCardSlot();

}

/// 카드장착 슬롯 세팅
void cGMTool::EventItemSettingCardSlot()
{
	/// id 정보 담기
	typedef tHashMap<unsigned long, void*> cItemDefineMap;
	cItemDefineMap& itemMap = ITEMMAN->GetDefineMap();

	unsigned int comboID[5] = { IDC_COMBO_PUTCARD1, IDC_COMBO_PUTCARD2, IDC_COMBO_PUTCARD3, IDC_COMBO_PUTCARD4, IDC_COMBO_PUTCARD5 };
	unsigned int slotID[5] = { IDC_EDIT_SLOTTYPE1, IDC_EDIT_SLOTTYPE2, IDC_EDIT_SLOTTYPE3, IDC_EDIT_SLOTTYPE4, IDC_EDIT_SLOTTYPE5 };

	/// 카드콤보
	for( int i = 0; i < 5; ++i )
	{
		EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i] ), false );
		SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i] ), CB_RESETCONTENT, 0, 0 );
		SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i]), CB_ADDSTRING, 0, (LPARAM)_T("-------------------------"));
	}

	/// 슬롯 카드 타입 초기화
	for( int i = 0; i < 5; ++i )
	{
		SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], slotID[i], _T("") );
	}

	/// 아이템 인덱스 받아오기
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_NAME );
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

			SetDlgItemText( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], slotID[i], str );
			EnableWindow( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i]), true );
			SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i] ), CB_SETCURSEL, 0, 0 );

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
				int key = SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i]), CB_ADDSTRING, 0, (LPARAM)(name));
				SendMessage( GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], comboID[i]), CB_SETITEMDATA, key, (LPARAM)cardIndex );
			}
		}
	}
}

void cGMTool::DeleteAllItemList()
{
	HWND listViewControl = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST );
	int itemMax = ListView_GetItemCount( listViewControl);
	for( int cnt = 0; cnt < itemMax; ++cnt )
	{
		sCheat_Make_Item_Info* pMsg = NULL;
		LVITEM LvItem;
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = 0;
		LvItem.iSubItem = 0;
		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_GETITEM, 0, (LPARAM)&LvItem );

		pMsg = (sCheat_Make_Item_Info*)LvItem.lParam;
		if( pMsg )
		{
			SAFE_DELETE(pMsg);
		}

		SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_DELETEITEM, (WPARAM)(int)0, 0);
	}
}

void cGMTool::OnSettingFindNameSelected()
{
	HWND hItemFindList = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LIST_ITEMSETTING_FIND_LIST );
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

	HWND highCombo = GetDlgItem( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_HIGH );
	int count = SendMessage( highCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long comboIndex = SendMessage( highCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == (long)pItemDefine->GetHighType() )
		{
			SendMessage( highCombo, CB_SETCURSEL, i, 0 );
			EventItemSettingHighClass();
			break;
		}
	}
	if( i == count )
	{
		assert(0);
		return;
	}

	HWND midCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_MIDDLE );
	count = SendMessage( midCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long comboIndex = SendMessage( midCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == (long)pItemDefine->GetMiddleType() )
		{
			SendMessage( midCombo, CB_SETCURSEL, i, 0 );
			EventItemSettingMiddleClass();
			break;
		}
	}
	if( i == count )
	{
		assert(0);
		return;
	}

	HWND lowCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_LOW );
	count = SendMessage( lowCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long lowKey = pItemDefine->GetLowType() + ( pItemDefine->GetMiddleType() * 100000 ) ;
		long comboIndex = SendMessage( lowCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == lowKey )
		{
			SendMessage( lowCombo, CB_SETCURSEL, i, 0 );
			EventItemSettingLowClass();
			break;
		}
	}
	if( i == count )
	{
		assert(0);
		return;
	}

	/// 이름 초기화
	HWND nameCombo = GetDlgItem(mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_COMBO_ITEMSETTING_NAME );
	count = SendMessage( nameCombo, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < count; ++i )
	{
		long comboIndex = SendMessage( nameCombo, CB_GETITEMDATA, (WPARAM)i, 0 );
		if( comboIndex == itemIndex )
		{
			SendMessage( nameCombo, CB_SETCURSEL, i, 0 );
			EventItemSettingCardSlot();
			break;
		}
	}
}


void cGMTool::SendMessageToSettingFindList( UINT msg, WPARAM wparam, LPARAM lparam )
{
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LIST_ITEMSETTING_FIND_LIST, msg, wparam, lparam );
}


/// 아이템 검색 에디트 박스 서브 클래싱
INT_PTR CALLBACK SettingFindNameEditSubProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_KEYDOWN:
		switch(wparam)	
		{
			/// 엔터키 입력시 아이템 콤보 셋팅.
		case VK_RETURN:
			{
				GMTOOL->OnSettingFindNameSelected();	
			}
			return 0;

			/// 방향키 입력시 리스트 박스에 있는 요소 상/하 이동하기.
		case VK_UP:
		case VK_DOWN:
		case VK_PRIOR:
		case VK_NEXT:
			{
				GMTOOL->SendMessageToSettingFindList( msg, wparam, lparam );
			}
			return 0;

		}
		break;
	}

	return CallWindowProc( GMTOOL->GetOldItemSettingEditProc(), wnd, msg, wparam, lparam );
}