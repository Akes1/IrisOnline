#include "stdafx.h"
#include <CommCtrl.h>
#include "GMTool.h"
#include "resource.h"
#include "CheatManager.h"
#include "Cheat_Common.h"
#include "Protocol_Cheat.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "GameUIManager.h"

#include "SkillManager.h"
#include "SkillScript.h"
#include "Hero.h"
#include "SkillWindow.h"

cGMTool* cGMTool::mInstance = 0;

cGMTool::cGMTool()
: mParentWnd( 0 )
, mMainDlg( 0 )
, mRichMod( 0 )
, mInst ( 0 )
, mIsNoticefileOpen( false )
, mIsNoticeTimerRun( false )
, mIsCounterTimerRun( false )
, mTimeInterval( 1 )
, mSpeed( 0 )
, mExpPercent( 0 )
, mSxpPercent( 0 )
, mDropPercent( 0 )
, mItemNameSelected( -1 )
{ 
	assert( mInstance == 0 && "bad singleton" );
	mInstance = this;

	::memset( mSubDlg, 0, sizeof(HWND)*MAX_GMTOOL_SUBDLG );
	::memset( mCurrentPath, 0, sizeof( mCurrentPath ) );
	::memset( mSubDlgPos, 0, sizeof( mSubDlgPos ) );

	mNumDownCounter.mCount      = 0;
	mNumDownCounter.mNoticeType = 0;

	GetCurrentDirectoryA( MAX_PATH, mCurrentPath);

	mChasePosImg.mIsShow = false;
	mChasePosImg.mPosX = 0;
	mChasePosImg.mPosY = 0;
	mChasePosImg.mMapIndex  = 0;
	mChasePosImg.mRoomIndex = 0;
}

cGMTool::~cGMTool()
{
	FreeLibrary( mRichMod );

	assert( mInstance );
	mInstance = 0;

	tItemTypeMap::cIterator i = mItemTypeLowItemIndexMap.Begin();
	tItemTypeMap::cIterator end = mItemTypeLowItemIndexMap.End();
	for( ; i != end; ++i )
	{
		tItemSubSet* pLowSet = i->mSecond;
		SAFE_DELETE( pLowSet );
	}
	mItemTypeLowItemIndexMap.Clear(); 

	i = mItemTypeMiddleLowMap.Begin();
	end = mItemTypeMiddleLowMap.End();
	for( ; i != end; ++i )
	{
		tItemSubSet* pLowSet = i->mSecond;
		SAFE_DELETE( pLowSet );
	}
	mItemTypeMiddleLowMap.Clear();

	i = mItemTypeHighMiddleMap.Begin();
	end = mItemTypeHighMiddleMap.End();
	for( ; i != end; ++i )
	{
		tItemSubSet* pSubSet = i->mSecond;
		SAFE_DELETE( pSubSet );
	}
	mItemTypeHighMiddleMap.Clear();

	tAutoNoticeList::cConstIterator iBegin = mAutoNoticeList.Begin();
	tAutoNoticeList::cConstIterator iEnd   = mAutoNoticeList.End();
	for( ; iBegin != iEnd; ++iBegin )
	{
		sAutoNotice* pAutoNotice = NULL;
		pAutoNotice = *iBegin;
		SAFE_DELETE( pAutoNotice );
	}
	mAutoNoticeList.Clear();
	
	DeleteObject( mHeroTargetPosImg );
	cMapImageHashMap::cConstIterator pos2 = mMapImgHashMap.Begin();
	cMapImageHashMap::cConstIterator end2 = mMapImgHashMap.End();
	for( ; pos2 != end2; ++pos2 )
	{
		HBITMAP bitmap = (HBITMAP)(*pos2).mSecond;
		DeleteObject( bitmap );
	}

	tSkillTypeMap::cConstIterator pos3 = mSkillTypeIndexMap.Begin();
	tSkillTypeMap::cConstIterator end3 = mSkillTypeIndexMap.End();
	for( ; pos3 != end3; ++pos3 )
	{
		tSkillSubArr* skillArr = (*pos3).mSecond;
		skillArr->Clear();
		SAFE_DELETE( skillArr );
	}
	mSkillTypeIndexMap.Clear();

	DeleteAllItemList();
	ClearThemeResetList();

	if( mIsNoticefileOpen == true )
	{
		mNoticeFileLoader.Close();
		mIsNoticefileOpen = false;
	}

	if( mIsItemSettingfileOpen == true )
	{
		mItemSettingFileLoader.Close();
		mIsItemSettingfileOpen = false;
	}

	if( mIsNoticeTimerRun == true )
	{
		KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND );
		mIsNoticeTimerRun = false;
	}
	if( mIsCounterTimerRun == true )
	{
		KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT );
		mIsCounterTimerRun = false;
	}
}

void cGMTool::Clear( )
{
	ClearThemeResetList();

	if( mIsNoticefileOpen == true )
	{
		mNoticeFileLoader.Close();
		mIsNoticefileOpen = false;
	}

	if( mIsItemSettingfileOpen == true )
	{
		mItemSettingFileLoader.Close();
		mIsItemSettingfileOpen = false;
	}

	if( mIsNoticeTimerRun == true )
	{
		KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_NOTICE_SEND );
		mIsNoticeTimerRun = false;
	}
	if( mIsCounterTimerRun == true )
	{
		KillTimer( mSubDlg[GMTOOL_SUBDLG_NOTICE], eGMTOOL_TIMER_DOWN_COUNT );
		mIsCounterTimerRun = false;
	}

	mSpeed = 0 ;

	GMTOOL->SetHideModeText( false );
	GMTOOL->SetUndeadModeText( false );

	/// 초기 트랙바 설정
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_SPEED], IDC_SLIDER_SPEED_MOVESPEED, TBM_SETPOS, WPARAM( TRUE ), LPARAM( mSpeed ) );

#ifdef _GMTOOL
	CHEATMAN->SetHideMode( false );
	CHEATMAN->SetUndeadMode( false );
#endif	 

}

void cGMTool::InitChannelChange()
{
	if( mIsNoticefileOpen == true )
	{
		mNoticeFileLoader.Close();
		mIsNoticefileOpen = false;
	}

	if( mIsItemSettingfileOpen == true )
	{
		mItemSettingFileLoader.Close();
		mIsItemSettingfileOpen = false;
	}

	mSpeed = 0 ;

	GMTOOL->SetHideModeText( false );
	GMTOOL->SetUndeadModeText( false );

	/// 초기 트랙바 설정
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_SPEED], IDC_SLIDER_SPEED_MOVESPEED, TBM_SETPOS, WPARAM( TRUE ), LPARAM( mSpeed ) );

#ifdef _GMTOOL
	CHEATMAN->SetHideMode( false );
	CHEATMAN->SetUndeadMode( false );
#endif

}

bool cGMTool::Init( HINSTANCE hinst, HWND hwnd )
{
	mInst      = hinst;
	mParentWnd = hwnd;
	
	mRichMod = LoadLibrary( _T("Riched20.dll") );

	/// dialog 생성
	mMainDlg = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL), mParentWnd, DlgProc );
	mSubDlg[GMTOOL_SUBDLG_ITEM]		= CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_ITEM),		mMainDlg, DlgItemProc );
	mSubDlg[GMTOOL_SUBDLG_SKILL]	= CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_SKILL),	mMainDlg, DlgSkillProc );
	mSubDlg[GMTOOL_SUBDLG_MONSTER]	= CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_MONSTER),	mMainDlg, DlgMonsterProc );
	mSubDlg[GMTOOL_SUBDLG_NOTICE]	= CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_NOTICE),	mMainDlg, DlgNoticeProc );
	mSubDlg[GMTOOL_SUBDLG_WEBSITE]	= CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_WEBSITE),	mMainDlg, DlgWebSiteProc );
	mSubDlg[GMTOOL_SUBDLG_MAP]	    = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_MAP),	    mMainDlg, DlgMapProc );
	mSubDlg[GMTOOL_SUBDLG_SPEED]    = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_SPEED),	mMainDlg, DlgSpeedProc );
 	mSubDlg[GMTOOL_SUBDLG_STOP]	    = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_STOP),	    mMainDlg, DlgStopProc );
 	mSubDlg[GMTOOL_SUBDLG_KICK]	    = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_KICK),	    mMainDlg, DlgKickProc );
	mSubDlg[GMTOOL_SUBDLG_EVENT]	= CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_EVENT),	mMainDlg, DlgEventProc );
	mSubDlg[GMTOOL_SUBDLG_GMNAME]   = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_GMNAME ),  mMainDlg, DlgGMNameProc );
	mSubDlg[GMTOOL_SUBDLG_ITEMSETTING]= CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_ITEMSETTING),	mMainDlg, DlgItemSettingProc );
	mSubDlg[GMTOOL_SUBDLG_JOB]		  = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_JOB),	mMainDlg, DlgJobProc );
	mSubDlg[GMTOOL_SUBDLG_FORCE]    = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_FORCE), mMainDlg, DlgForceProc );
	mSubDlg[GMTOOL_SUBDLG_THEME]    = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_GMTOOL_THEME), mMainDlg, DlgThemeProc );

	/// enable 걸기
 	//EnableWindow( GetDlgItem( mMainDlg, IDC_BUTTON_GMT_EVENT ), false );

	/// 아이템 셋팅 란의 List View Control column 설정
	LVCOLUMNW col;
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
	col.fmt = LVCFMT_LEFT;
	col.cx  = 212;
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8043 );
	col.iSubItem = 0;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_INSERTCOLUMN, (WPARAM)(int)0, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8044 );
	col.cx  = 42;
	col.iSubItem = 1;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_INSERTCOLUMN, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)(&col));
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING], IDC_LISTCONTROL_ITEMLIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT) );

	/// 사용 제한 란의  List View Control column 설정
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8070 );
	col.cx  = 25;
	col.iSubItem = 0;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_INSERTCOLUMN, (WPARAM)(int)0, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8071 );
	col.cx  = 110;
	col.iSubItem = 1;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_INSERTCOLUMN, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8072 );
	col.cx  = 110;
	col.iSubItem = 2;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_INSERTCOLUMN, (WPARAM)(int)2, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8073 );
	col.cx  = 125;
	col.iSubItem = 3;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_INSERTCOLUMN, (WPARAM)(int)3, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8074 );
	col.cx  = 125;
	col.iSubItem = 4;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_INSERTCOLUMN, (WPARAM)(int)4, (LPARAM)(const LV_COLUMN *)(&col));
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_STOP], IDC_LISTCONTROL_STOP, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT) );

	/// Map 이동 란의 List View Control column 설정
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8070 );
	col.cx  = 25;
	col.iSubItem = 0;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_INSERTCOLUMN, (WPARAM)(int)0, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8083 );
	col.cx  = 110;
	col.iSubItem = 1;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_INSERTCOLUMN, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8084 );
	col.cx  = 87;
	col.iSubItem = 2;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_INSERTCOLUMN, (WPARAM)(int)2, (LPARAM)(const LV_COLUMN *)(&col));
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_MAP], IDC_LISTCONTROL_MAP_ROOM_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT) );
	
	/// 테마 초기화 란의 List View Control column 설정
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8070 ); //"-"
	col.cx  = 25;
	col.iSubItem = 0;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTCOLUMN, (WPARAM)(int)0, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8083 ); // "맵 이름"
	col.cx  = 110;
	col.iSubItem = 1;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTCOLUMN, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8101 ); // "귀속번호"
	col.cx  = 70;
	col.iSubItem = 2;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTCOLUMN, (WPARAM)(int)2, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8102 ); // "난이도"
	col.cx  = 70;
	col.iSubItem = 3;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTCOLUMN, (WPARAM)(int)3, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8103 ); // "마지막 업데이트일"
	col.cx  = 120;
	col.iSubItem = 4;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTCOLUMN, (WPARAM)(int)4, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8104 ); // "유효만기일"
	col.cx  = 120;
	col.iSubItem = 5;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_INSERTCOLUMN, (WPARAM)(int)5, (LPARAM)(const LV_COLUMN *)(&col));
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_THEME], IDC_LISTCONTROL_THEME, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT) );

	/// GM 이벤트 List View Control column 설정
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8070 ); //"-"
	col.cx  = 25;
	col.iSubItem = 0;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_INSERTCOLUMN, (WPARAM)(int)0, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8116 ); // "효과 이름"
	col.cx  = 150;
	col.iSubItem = 1;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_INSERTCOLUMN, (WPARAM)(int)1, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8117 ); // "시작 시간"
	col.cx  = 120;
	col.iSubItem = 2;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_INSERTCOLUMN, (WPARAM)(int)2, (LPARAM)(const LV_COLUMN *)(&col));
	col.pszText = (LPWSTR)GAMERESOURCEMAN->GetGameText( 8104 ); // "유효만기일"
	col.cx  = 120;
	col.iSubItem = 3;
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_INSERTCOLUMN, (WPARAM)(int)3, (LPARAM)(const LV_COLUMN *)(&col));
	SendDlgItemMessage( mSubDlg[GMTOOL_SUBDLG_EVENT], IDC_LISTCONTROL_EVENT, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT) );

	///서브 클래싱 설정
	SetMonsterEditProc(); ///몬스터 검색
	SetItemEditProc();    ///아이템 이름 검색
	SetItemSettingEditProc(); ///아이템 셋팅에서 이름 검색

	return true;
}

void cGMTool::Exit()
{
	DeleteAllItemList();
	ClearThemeResetList();

	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_NOTICE] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_MONSTER] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_SKILL] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_ITEM] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_WEBSITE] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_MAP] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_SPEED] );
 	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_STOP] );
 	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_KICK] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_EVENT] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_GMNAME] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_ITEMSETTING] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_JOB] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_FORCE] );
	DestroyWindow( mSubDlg[GMTOOL_SUBDLG_THEME] );

	DestroyWindow( mMainDlg );
}

bool cGMTool::LoadData()
{
	/// 아이템 데이터 로딩
	if( LoadItemData() == false )
	{
		assert(0);
		return false;
	}

	/// 몬스터 데이터 로딩
	if( LoadMonsterData() == false )
	{
		assert(0);
		return false;
	}
	
	/// 공지 데이터 로딩
	if( LoadNoticeData() == false )
	{
		assert(0);
		return false;
	}

	/// 스킬 데이터 로딩
	if( LoadSkillData() == false )
	{
		assert(0);
		return false;
	}

	/// 스킬 데이터 로딩
	if( LoadMapData() == false )
	{
		assert(0);
		return false;
	}

	/// 이벤트 데이터 로딩
	if( LoadEventData() == false )
	{
		assert(0);
		return false;
	}

	//::ShowWindow( mMainDlg, SW_SHOW );
//	::SetActiveWindow( mParentWnd );

	return true;
}

void cGMTool::OnMove()
{
	RECT rc0, rc1;
	::GetWindowRect( mParentWnd, &rc0 );
	::GetWindowRect( mMainDlg, &rc1 );
	int width = rc1.right - rc1.left;
	int x = rc0.left - width;
	if( x < 0 )
		x = 0;
	int y = rc0.top;

	::SetWindowPos( mMainDlg, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	for( int cnt = 0; cnt < MAX_GMTOOL_SUBDLG; ++cnt )
	{
		if ( ::IsWindowVisible( mSubDlg[cnt] ) == TRUE )
		{
			//int screenWid   = GetSystemMetrics( SM_CXSCREEN );;
			//int ScreenHeight= GetSystemMetrics( SM_CYSCREEN );;
			
			RECT childWndPos;
			::GetWindowRect( mSubDlg[cnt], &childWndPos );
			
			int posX      = rc0.left + mSubDlgPos[cnt].x;
			int posY      = rc0.top  + mSubDlgPos[cnt].y;;
			//int wndWidth  = childWndPos.right - childWndPos.left;
			//int wndHeight = childWndPos.bottom- childWndPos.top;

			::SetWindowPos( mSubDlg[cnt], 0, posX, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
		}
	}
}

void cGMTool::OnCommand( int id )
{
	HWND hwnd = NULL;
	switch( id )
	{
	case IDC_BUTTON_GMT_ITEM:	hwnd = mSubDlg[GMTOOL_SUBDLG_ITEM];		break;
	case IDC_BUTTON_GMT_SKILL:	hwnd = mSubDlg[GMTOOL_SUBDLG_SKILL];	break;
	case IDC_BUTTON_GMT_MONSTER:hwnd = mSubDlg[GMTOOL_SUBDLG_MONSTER];	break;
	case IDC_BUTTON_GMT_NOTICE:	hwnd = mSubDlg[GMTOOL_SUBDLG_NOTICE];	break;
    case IDC_BUTTON_GMT_WEBSITE:hwnd = mSubDlg[GMTOOL_SUBDLG_WEBSITE];	break;
	case IDC_BUTTON_GMT_MAP    :hwnd = mSubDlg[GMTOOL_SUBDLG_MAP];   	break;
	case IDC_BUTTON_GMT_SPEED  :hwnd = mSubDlg[GMTOOL_SUBDLG_SPEED];   	break;
 	case IDC_BUTTON_GMT_STOP   :hwnd = mSubDlg[GMTOOL_SUBDLG_STOP];   	break;	
 	case IDC_BUTTON_GMT_KICK   :hwnd = mSubDlg[GMTOOL_SUBDLG_KICK];   	break;
	case IDC_BUTTON_GMT_EVENT  :hwnd = mSubDlg[GMTOOL_SUBDLG_EVENT];	break;
	case IDC_BUTTON_GMT_GMNAME: hwnd = mSubDlg[GMTOOL_SUBDLG_GMNAME];   break;
	case IDC_BUTTON_GMT_ITEMSETTING:hwnd = mSubDlg[GMTOOL_SUBDLG_ITEMSETTING]; break;
	case IDC_BUTTON_GMT_JOB:	hwnd = mSubDlg[GMTOOL_SUBDLG_JOB];		break;
	case IDC_BUTTON_GMT_FORCE:  hwnd = mSubDlg[GMTOOL_SUBDLG_FORCE];	break;
	case IDC_BUTTON_GMT_THEME:  hwnd = mSubDlg[GMTOOL_SUBDLG_THEME];	break;

#ifdef _GMTOOL
	case IDC_BUTTON_GMT_UNDEAD :
		/// 메시지 전송
		if( NETWORK->IsConnected() )
		{
			MSG_REQ_CHEAT_UNDEAD msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_UNDEAD_REQ;
			msg.mMode = !(CHEATMAN->GetUndeadMode());

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}

		::SetActiveWindow( mParentWnd );
		return;
	
	case IDC_BUTTON_GMT_HIDE   :
		/// 메시지 전송
		if( NETWORK->IsConnected() )
		{
			MSG_REQ_CHEAT_HIDE msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_CHEAT;
			msg.Protocol = NM_CHEAT_HIDE_REQ;
			msg.mMode = !(CHEATMAN->GetHideMode());

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		::SetActiveWindow( mParentWnd );
		return;
	case IDC_BUTTON_GMT_TODAY_WORD:
		{
			GAMEUI->ShowTodayWordWindow();
		}
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_GMT_SKILL_GET:
		{

			/// 유저가 배워야 할 스킬 리스트 얻기
			GetHeroSkillList();
			
			cSkillWindow* win = GAMEUI->GetSkillWindow();
			if( win )
				win->NetUnLock();

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				/// GM_Tool 스킬 습득 중
				eMBEVENT_GM_TOOL_GET_SKILL, eSKIN_NONE,
				GAMERESOURCEMAN->GetGameText( 8069 ), 
				GAMERESOURCEMAN->GetGameText( 301 )
				);

			GetSkill();
		}
		::SetActiveWindow( mParentWnd );
		break;
#endif
	default:					assert( 0 && "error gmtool button" );	return;
	}

	::ShowWindow( hwnd, IsWindowVisible( hwnd ) ? SW_HIDE : SW_SHOW );
}

BOOL cGMTool::GetMapTeleportState()
{
	if( mMainDlg )
		return IsWindowEnabled( GetDlgItem( mMainDlg, IDC_BUTTON_GMT_MAP ) );
	
	return false;
}

void cGMTool::SetUndeadModeText( bool mode )
{
	charT str[32] = {0,};
	long setStringTableIdx = IDS_GMTOOL_UNDEAD_ON;

	if( mode == false )
		setStringTableIdx = IDS_GMTOOL_UNDEAD;
	
	if( LoadString( mInst, setStringTableIdx, str, MAX_LOADSTRING ) == 0 )
		return;
	SetDlgItemText( mMainDlg, IDC_BUTTON_GMT_UNDEAD, str );
}

void cGMTool::SetHideModeText( bool mode )
{
	charT str[32] = {0,};
	long setStringTableIdx = IDS_GMTOOL_HIDE_ON;

	if( mode == false )
		setStringTableIdx = IDS_GMTOOL_HIDE;

	if( LoadString( mInst, setStringTableIdx, str, MAX_LOADSTRING ) == 0 )
		return;
	SetDlgItemText( mMainDlg, IDC_BUTTON_GMT_HIDE, str );
}

void cGMTool::GetHeroSkillList()
{
	mSkillCheatAddList.Clear();

	/// 스크립트 로딩
	unsigned long		   parentJobMax = 0;
	tArray<unsigned long>* pArray = GAMERESOURCEMAN->GetParentJob( HERO->GetJob() );
	if( pArray )
	{
		if( pArray->IsEmpty() == false )
			parentJobMax = pArray->GetSize();
	}

	unsigned long jobIndex		 = HERO->GetJob();
	unsigned long parentJobCount = 0;

	do 
	{
		cSkillTree* pTree = SKILLSCRIPT->GetSkillTree( jobIndex );
		assert(pTree);
		if( pTree )
		{
			for( unsigned int i=0; i<pTree->mIconCount; ++i )
			{
				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( pTree->mIconList[i].skillIdx );
				if( pInfo )
				{
					/// 기본 제공 스킬이면서 단계가 1인 스킬은 버튼 생성을 제외한다.
					if( pInfo->mStepCount == 1 && pInfo->mpSetpInfoArray[0].mPlayerLevel == 0 )
						continue;

					bool isSkillUp = false;
					sKeepSkill* pKeepSkill = SKILLMAN->GetKeepInfo( pTree->mIconList[i].skillIdx );
					if( pKeepSkill )
					{
						/// 마스터 검사	
						if( pKeepSkill->mStep + 1 == pInfo->mStepCount )
							continue;
						else if( pKeepSkill->mStep < pInfo->mStepCount )
							isSkillUp = true;
					}

					/// 현재 자기 레벨에 맞는 최대 스킬 래밸 검사
					unsigned char stepCount = 0;
					for( stepCount = 0; stepCount < pInfo->mStepCount; ++stepCount )
					{
						if( pInfo->mpSetpInfoArray[stepCount].mPlayerLevel > HERO->GetLevel() )
							break;
					}
					/// 스킬1 단계 조차 배울 수 없다면 다음 스킬.
					if( stepCount == 0 )
						continue;

					
					/// 리스트에 추가
					sSkillCheatAdd temp;
					temp.mSkillClassIdx = pTree->mIconList[i].skillIdx;
					temp.mSkillStep     = stepCount-1;
					temp.mIsSkillUp		= isSkillUp;
					mSkillCheatAddList.PushBack( temp );
				}
			}
		}
		else
			break;

		if( parentJobCount < parentJobMax )
		{
			jobIndex = (*pArray)[parentJobCount];
			parentJobCount++;
		}
		else
			break;

	} while( 1 );
}

void cGMTool::GetSkill()
{

	/// 리스트가 빌 때까지 서버에 스킬 배우기 치트 전송
	if( mSkillCheatAddList.IsEmpty() == false )
	{
		sSkillCheatAdd temp = mSkillCheatAddList.Front();
		MSG_REQ_CHEAT_SKILL_ADD msg;
		msg.Category = NM_CHEAT;
		msg.Protocol = NM_CHEAT_SKILL_GET_REQ;
		msg.mSkillClassIdx = temp.mSkillClassIdx;
		msg.mSkillStep     = temp.mSkillStep;
		msg.mIsSkillUp     = temp.mIsSkillUp;
		if( NETWORK->IsConnected() )
		{
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}

		mSkillCheatAddList.PopFront();
	}
	else
	{

		/// GM_Tool 스킬 습득 메시지 창 닫기
		UIMAN->DestroyMsgBox( eMBEVENT_GM_TOOL_GET_SKILL );

		cSkillWindow* win = GAMEUI->GetSkillWindow();
		if( win )
		{
			win->NetUnLock();
			win->ShowSkillUpButton();
		}
	}
}

void cGMTool::OnHideDlg()
{
	::SetActiveWindow( mParentWnd );
}

INT_PTR CALLBACK cGMTool::DlgProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnMove();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommand( LOWORD( wparam ) );
		}
		break;
	case WM_SYSCOMMAND:
		{
			if( wparam == SC_CLOSE ) 
			{
				PostMessage( ::GetParent( wnd ), msg, wparam, lparam );
			}
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam == FALSE )
				GMTOOL->OnHideDlg();
		}
	}
	return FALSE;
}

/// 아이템 
INT_PTR CALLBACK cGMTool::DlgItemProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
		    GMTOOL->OnChildMove( GMTOOL_SUBDLG_ITEM );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowItem();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandItem( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	case WM_DESTROY:
		{
			GMTOOL->OnDestroyItem();
		}
	}
	return FALSE;
}

/// 스킬
INT_PTR CALLBACK cGMTool::DlgSkillProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_SKILL );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowSkill();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandSkill( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 몬스터
INT_PTR CALLBACK cGMTool::DlgMonsterProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_MONSTER );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowMonster();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandMonster( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	case WM_DESTROY:
		{
			GMTOOL->OnDestroyMonster();
		}
	}
	return FALSE;
}

/// 공지
INT_PTR CALLBACK cGMTool::DlgNoticeProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_NOTICE );	
		}
		break;
	case WM_INITDIALOG:
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowNotice();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandNotice( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	case WM_TIMER:
		{
			GMTOOL->OnTimerNotice( (int)wparam );	
		}
		break;
	}
	return FALSE;
}

/// 웹 사이트
INT_PTR CALLBACK cGMTool::DlgWebSiteProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_WEBSITE );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowWebSite();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandWebSite( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 맵 이동 
INT_PTR CALLBACK cGMTool::DlgMapProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_MAP );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowMap();
			else
				GMTOOL->OnHideMap();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandMap( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	case WM_LBUTTONDOWN:
		{
			GMTOOL->OnLButtonDownMap( LOWORD(lparam), HIWORD(lparam) );
		}
		break;
	case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint( wnd, &ps );
			GMTOOL->OnPaintMap( hdc );
			EndPaint( wnd, &ps );
			ReleaseDC( wnd, hdc );
		}
		break;
	case WM_MOUSEMOVE:
		{
			GMTOOL->OnMouseMoveMap( wnd, LOWORD(lparam), HIWORD(lparam) );
		}
		break;
	case WM_MOUSELEAVE:
		{
			GMTOOL->OnMouseLeaveMap();
		}
		break;
	case WM_TIMER:
		{
			GMTOOL->OnTimerMap( (int)wparam );	
		}
		break;
	}
	return FALSE;
}

/// 이동 속도
INT_PTR CALLBACK cGMTool::DlgSpeedProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_SPEED );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowSpeed();
		}
		break;
	case WM_HSCROLL:
		{
			GMTOOL->OnCommandSpeed( IDC_SLIDER_SPEED_MOVESPEED, LOWORD( wparam ));
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandSpeed( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 유저 이동, 스킬 사용, 공격, 거래, 아이템 습득, 창고 이용 금지
INT_PTR CALLBACK cGMTool::DlgStopProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_STOP );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowStop();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandStop( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 유저 강제 퇴장
INT_PTR CALLBACK cGMTool::DlgKickProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_KICK );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowKick();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandKick( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 드롭율 / 경험치 증가율 조정
INT_PTR CALLBACK cGMTool::DlgEventProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_EVENT );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowEvent();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandEvent( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// GM 이름 등록
INT_PTR CALLBACK cGMTool::DlgGMNameProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_GMNAME );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowGMName();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandGMName( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 아이템 셋팅
INT_PTR CALLBACK cGMTool::DlgItemSettingProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_ITEMSETTING );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowItemSetting();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandItemSetting( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	case WM_DESTROY:
		{
			GMTOOL->OnDestroyItemSetting();
		}
	}
	return FALSE;
}

/// 직업 셋팅
INT_PTR CALLBACK cGMTool::DlgJobProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_JOB );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowJob();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandJob( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 세력 셋팅
INT_PTR CALLBACK cGMTool::DlgForceProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_FORCE );	
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowForce();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandForce( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}

/// 테마 초기화 셋팅
INT_PTR CALLBACK cGMTool::DlgThemeProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
			GMTOOL->OnChildMove( GMTOOL_SUBDLG_THEME );
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( (BOOL)wparam )
				GMTOOL->OnShowTheme();
			else
				GMTOOL->OnHideTheme();
		}
		break;
	case WM_COMMAND:
		{
			GMTOOL->OnCommandTheme( LOWORD( wparam ), HIWORD( wparam ) );
		}
		break;
	}
	return FALSE;
}
