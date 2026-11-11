/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.06.25
* 내    용 : 운영툴
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <set>
#include "CommonDefines.h"
#include "TargaImage.h"

struct NO_CHAT_CHARACTER_DUMMY;

enum eGMTOOL_SUBDLG_INDEX
{
	GMTOOL_SUBDLG_ITEM = 0, /// 아이템
	GMTOOL_SUBDLG_MONSTER,  /// 몬스터 
	GMTOOL_SUBDLG_NOTICE,   /// 공지
	GMTOOL_SUBDLG_WEBSITE,  /// 웹사이트
	GMTOOL_SUBDLG_MAP,      /// 맵 이동/소환
	GMTOOL_SUBDLG_SPEED,    /// 이동 속도 
	GMTOOL_SUBDLG_STOP,     /// 행동제한
	GMTOOL_SUBDLG_KICK,     /// 강제퇴장
	GMTOOL_SUBDLG_EVENT,	/// 경험치/드롭율
	GMTOOL_SUBDLG_GMNAME,	/// GM 이름등록
	GMTOOL_SUBDLG_ITEMSETTING, /// GM아이템 셋팅
	GMTOOL_SUBDLG_SKILL,    /// 버프 적용
	GMTOOL_SUBDLG_JOB,		/// 전직
	GMTOOL_SUBDLG_FORCE,    /// 세력 
	GMTOOL_SUBDLG_THEME,	/// 테마

	MAX_GMTOOL_SUBDLG
};

enum eGMTOOL_TIMER_ID
{
	eGMTOOL_TIMER_NOTICE_SEND = 1,
	eGMTOOL_TIMER_DOWN_COUNT,
	eGMTOOL_TIMER_HERO_POS_UPDATE,

	eGMTOOL_TIMER_MAX,
};

enum eGMTOOL_NOTICE_TYPE
{
	eGMTOOL_NOTICE_SERVER = 0,
	eGMTOOL_NOTICE_CHANNEL,
	eGMTOOL_NOTICE_MAP,

	eGMTOOL_NOTICE_MAX,
};

/// 운영자 스킬 정보
enum eGM_SKILL_TYPE
{
	eGM_SKILL_TYPE_BUILD = 26100,
	eGM_SKILL_TYPE_MAGIC,

	eGM_SKILL_TYPE_MAX,
};

/// 자동 공지 구조체
struct sAutoNotice
{
	int   mTimeDelay;
	WCHAR mNoticeMsg[MAX_CHAT_BUFFER_SIZE];
};

/// 숫자 카운터 구조체
struct sNumDownCounter
{
	short		   mNoticeType;
	unsigned short mCount;
};

/// 스킬 한번에 배우기
struct sSkillCheatAdd
{
	unsigned long	mSkillClassIdx;
	unsigned char	mSkillStep;
	bool			mIsSkillUp;
};

/// 맵 이미지 그리는 위치
enum MAP_IMAGE
{
	MAP_IMG_POS_X = 260,
	MAP_IMG_POS_Y =  18,
	MAP_IMG_WIDTH = 512,
	MAP_IMG_HEIGHT= 512,
};

/// search 한 유저 정보
struct sChaseUserInfo
{
	bool   mIsShow;
	int    mPosX;
	int    mPosY;
	unsigned short  mMapIndex;
	unsigned short  mRoomIndex;
};

/// search 이미지 파일 정보
struct sImageInfo
{
	POINT	mImgWidHeight;
	POINT	mTexturePos;
};

/// 테마 정보
struct sTheme_Info
{
	unsigned long  mCharacterIdx;	/// 플레이어인덱스
	unsigned short mThemeIdx;		/// 테마종류
	unsigned char  mThemeMode;		/// 테마모드
};

struct MSG_RES_USED_ROOM_ID;
struct MSG_RES_CHEAT_THEME_INFO;
struct MSG_RES_CHEAT_GM_EVENT_SEARCH;

/// 운영툴
class cGMTool
{
	static cGMTool* mInstance;

public:
	cGMTool();
	virtual ~cGMTool();

	bool Init( HINSTANCE hinst, HWND hwnd );
	void Clear();
	void InitChannelChange();
	bool LoadData();
	void Exit();
	void OnMove();
	void OnHideDlg();

public:
	static cGMTool* GetSingleton();
	
	/// 메시지 처리 함수
	static INT_PTR CALLBACK DlgProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgItemProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgSkillProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgMonsterProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgNoticeProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgWebSiteProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgMapProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgSpeedProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgStopProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgKickProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgEventProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgGMNameProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgItemSettingProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgJobProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgForceProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	static INT_PTR CALLBACK DlgThemeProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
	
	void SetGMToolVisible( bool visible );
	void SetPvpThemeList( MSG_RES_USED_ROOM_ID* pMsg );

	BOOL GetMapTeleportState();

	void SetUndeadModeText( bool mode );
	void SetHideModeText( bool mode );

	/// 몬스터 소환 윈도우 관련
	void	SetMonsterEditProc(); /// 서브클래싱 설정
	void	OnDestroyMonster();   /// 서브클래싱 해제
	void	OnCreateMonsterButtonDown(); /// 몬스터 생성
	void	SendMessageToMonsterList( UINT msg, WPARAM wparam, LPARAM lparam );
	WNDPROC GetOldEditProc(){ return mOldMonsterNameEditProc; }

	/// 아이템 생성 서브 클래싱 관련
	void	SetItemEditProc(); /// 서브클래싱 설정
	void	OnDestroyItem();   /// 서브클래싱 해제
	void	OnItemNameSelected(); /// 검색된 아이템 선택
	void	SendMessageToItemList( UINT msg, WPARAM wparam, LPARAM lparam );
	WNDPROC GetOldItemEditProc(){ return mOldItemNameEditProc; }

	/// 아이템 셋팅 서브 클래싱 관련
	void	SetItemSettingEditProc(); /// 서브클래싱 설정
	void	OnDestroyItemSetting();   /// 서브클래싱 해제
	void	OnSettingFindNameSelected(); /// 검색된 아이템 선택
	void	SendMessageToSettingFindList( UINT msg, WPARAM wparam, LPARAM lparam );
	WNDPROC GetOldItemSettingEditProc(){ return mOldSettingFindNameEditProc; }


	/// 스킬 습득 하기
	void GetSkill();

	/// 채팅 금지 리스트 받기
	void SetNoChatUser( int rowCount, NO_CHAT_CHARACTER_DUMMY* table );
	void InitStopListBox();

	/// 맵이동/추적/소환 관련.
	void ShowSearchPlayer( unsigned short mapIndex, float posX, float posY );
	void ShowSearchChannelPlayer( short channelNum );
	
	/// 운영자 전용 스킬 썼는지 마우스 체크
	bool ProcessGMSkill( int mouseX, int mouseY );

	void HideChasePosImg(){ mChasePosImg.mIsShow = false; }

	/// 테마 초기화
	void SetThemeResetInfo( MSG_RES_CHEAT_THEME_INFO* pResMsg );
	void ClearThemeResetList();

	void SetGMEventInfo( MSG_RES_CHEAT_GM_EVENT_SEARCH* pResMsg );
	void ClearGMEventList();

protected:
	void OnCommand( int id );
	void OnCommandItem( int id, int msg );
	void OnCommandMonster( int id, int msg );
	void OnCommandNotice( int id, int msg );
	void OnCommandSkill( int id, int msg );
	void OnCommandWebSite( int id, int msg );
	void OnCommandMap( int id, int msg );
	void OnCommandSpeed( int id, int msg );
 	void OnCommandStop( int id, int msg );
 	void OnCommandKick( int id, int msg );
	void OnCommandEvent( int id, int msg );
	void OnCommandGMName( int id, int msg );
	void OnCommandItemSetting( int id, int msg );
	void OnCommandJob( int id, int msg );
	void OnCommandForce( int id, int msg );
	void OnCommandTheme( int id, int msg );

	void OnTimerNotice( int timerIndex );
	void OnTimerMap( int timerIndex );

	void OnShowItem();
	void OnShowMonster();
	void OnShowNotice();
	void OnShowSkill();
	void OnShowWebSite();
	void OnShowMap();
	void OnShowSpeed();
 	void OnShowStop();
 	void OnShowKick();
	void OnShowEvent();
	void OnShowGMName();
	void OnShowItemSetting();
	void OnShowJob();
	void OnShowForce();
	void OnShowTheme();
	
	void OnHideMap();
	void OnHideTheme();

	bool LoadItemData();
	bool LoadMonsterData();
	bool LoadNoticeData();
	bool LoadSkillData();
	bool LoadMapData();
	bool LoadEventData();

	void OnChildMove( eGMTOOL_SUBDLG_INDEX index );

	/// 공지 파일 읽기
	bool ParsingNoticeFile( char* filePath );
	/// 공지 보내기
	void SendNoticeMsg( int noticeType, WCHAR* str );
	/// 자동 공지 해야 하는 글들 표시
	void ShowAutoNoticeMsg();

	/// 아이템 세팅 관련
	void EventItemHighClass();
	void EventItemMiddleClass();
	void EventItemLowClass();
	void EventCardSlot();

	void EventItemSettingHighClass();
	void EventItemSettingMiddleClass();
	void EventItemSettingLowClass();
	void EventItemSettingCardSlot();

	/// GM 이벤트 효과 세팅 관련
	void EventInfluenceClass();

	void DeleteAllItemList();

	/// 배워야 할 스킬 리스트 작성
	void GetHeroSkillList();

	/// 맵 출력
	void DrawMap( int mapIndex );
	void OnLButtonDownMap( int x, int y );
	void OnPaintMap( HDC hdc );
	void OnMouseMoveMap(HWND wnd, int x, int y );
	void OnMouseLeaveMap();
	
	/// 운영자 스킬 사용
	void EventSkillType();
	void EventSkillName();

protected:
	HWND mParentWnd;
	HWND mMainDlg;
	HWND mSubDlg[MAX_GMTOOL_SUBDLG];
	HMODULE mRichMod;
	HINSTANCE mInst;

	/// 자동 공지
	bool        mIsNoticefileOpen;
	bool        mIsItemSettingfileOpen;
	bool        mIsNoticeTimerRun;   //자동 공지   타이머
	bool        mIsCounterTimerRun;  //자동 카운터 타이머
	char        mCurrentPath[ MAX_PATH ];
	cFileLoader mNoticeFileLoader;
	int         mTimeInterval;
	sNumDownCounter mNumDownCounter;

	/// 이동속도 조정
	int         mSpeed;
	/// 언데드 모드
	bool        mIsInOn;
	/// 경험치, 드랍률 조정
	short       mExpPercent;
	short       mSxpPercent;
	short       mDropPercent;
	
	/// 맵 이동에서 맵 이미지, hero위치, chase 위치
	typedef tPointerHashMap< unsigned long, void* > cMapImageHashMap;
	cMapImageHashMap mMapImgHashMap;
	HBITMAP		   mHeroTargetPosImg;
	sChaseUserInfo mChasePosImg;
	sImageInfo	   mHeroImg;
	sImageInfo	   mSearchImg;
	bool		   mIsLeaveEventTracking;
	

	/// 아이템 셋팅
	cFileLoader mItemSettingFileLoader;

	POINT       mSubDlgPos[MAX_GMTOOL_SUBDLG];

	typedef std::set< unsigned int > tItemSubSet;
	typedef tHashMap< unsigned int, tItemSubSet* > tItemTypeMap;
	tItemTypeMap mItemTypeHighMiddleMap;
	tItemTypeMap mItemTypeMiddleLowMap;
	tItemTypeMap mItemTypeLowItemIndexMap;

	/// 아이템에서 아이템 검색 기능.(추후 구현)
	int			 mItemNameSelected;
	cStringT	 mItemNameText;

	/// 몬스터에서 몬스터 이름 검색
	typedef const tPointerHashMap<unsigned long, void*> cMonsterHashMap;
	cMonsterHashMap* mMonsterMap;

	
	WNDPROC		 mOldMonsterNameEditProc; /// 몬스터에서 몬스터 이름 검색 에디트 서브 클래싱
	WNDPROC		 mOldItemNameEditProc;    /// 아이템 생성에서 아이템 이름 검색 에디트 서브 클래싱
	WNDPROC		 mOldSettingFindNameEditProc;    /// 아이템 생성에서 아이템 이름 검색 에디트 서브 클래싱


	/// 자동 공지 기능
	typedef tPointerList< sAutoNotice* > tAutoNoticeList;
	tAutoNoticeList mAutoNoticeList;

	/// 스킬 한꺼번에 습득하기
	typedef tList< sSkillCheatAdd > tSkillCheatAddList;
	tSkillCheatAddList mSkillCheatAddList;

	/// 운영자 스킬 정보
	typedef tArray< unsigned int > tSkillSubArr;
	typedef tHashMap< int, tSkillSubArr* > tSkillTypeMap;
	tSkillTypeMap mSkillTypeIndexMap;


public:
	static const int MAX_PERCENT =   999;
	static const int MIN_PERCENT =  -100;
};

inline void cGMTool::SetGMToolVisible( bool visible )
{
	ShowWindow( mMainDlg, visible==true ? SW_SHOWNOACTIVATE : SW_HIDE );
	if( visible == false )
	{
		for( int idx = 0; idx < MAX_GMTOOL_SUBDLG; ++idx )
			ShowWindow( mSubDlg[idx] , SW_HIDE );
	}
}

inline void cGMTool::OnChildMove( eGMTOOL_SUBDLG_INDEX index )
{
	RECT rc0, rc1;
	::GetWindowRect( mParentWnd, &rc0 );
	::GetWindowRect( mSubDlg[index], &rc1 );
	mSubDlgPos[index].x = rc1.left - rc0.left;
	mSubDlgPos[index].y = rc1.top - rc0.top;
}


inline cGMTool* cGMTool::GetSingleton()
{
	return mInstance;
}

#define GMTOOL	cGMTool::GetSingleton()
