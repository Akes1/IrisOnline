/* ==========================================================================
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Chat_Common.h"

enum eWINPOSIDX
{
	CHATWIN_POS = 0,	/// hold
	ECHATWIN1_POS,
	ECHATWIN2_POS,
	ECHATWIN3_POS,

	HEROSTATWIN_POS,	/// hold
	HEROINFOWIN_POS,
	TARGETWIN_POS,

	INVENWIN_POS,
	BAGWIN1_POS,
	BAGWIN2_POS,
	BAGWIN3_POS,

	SKILLWIN_POS,
	OPTWIN_POS,
	MINIMAPWIN_POS,

	CQUICK_POS,			/// hold or hide
	RQUICK_POS,			/// hold or hide
	FSCQUICK_POS,		/// hold or hide
	FSRQUICK_POS,		/// hold or hide

	PARTYWIN_POS,
	PARTYSTATWIN1_POS,
	PARTYSTATWIN2_POS,
	PARTYSTATWIN3_POS,
	PARTYSTATWIN4_POS,
	PARTYSTATWIN5_POS,
	
	PARTYSTATMINIWIN1_POS,
	PARTYSTATMINIWIN2_POS,
	PARTYSTATMINIWIN3_POS,
	PARTYSTATMINIWIN4_POS,
	PARTYSTATMINIWIN5_POS,

	FRIENDWIN_POS,
	TAROTCON_POS,
	SELLWIN_POS,
	LOOTWIN_POS,

	QUESTWIN_POS,
	SQUESTWIN_POS,

	DISJOINT_POS,
	ENHANCEDWIN_POS,
	PUTCARD_POS,
	SOCIALWIN_POS,
	TITLEWIN_POS,
	GUILDWIN_POS,
	THEMESTATEWIN_POS,
	TODAYWORDWIN_POS,
	ITEMMIXWIN_POS,
	MAILWIN_POS,
	PARTYUNIONWIN_POS,
	PARTYUNIONSMALLWIN_POS,
	
	CASHFREEMOVE_POS,
	TWITTER_POS,

 	CQUICK_EXTRA1_POS,			/// hold or hide
 	RQUICK_EXTRA1_POS,			/// hold or hide
 	CQUICK_EXTRA2_POS,			/// hold or hide
 	RQUICK_EXTRA2_POS,			/// hold or hide

	SUMMON_POS,		

	POS_END,
};

class cBaseObject;
enum eMSGBox_Event;

class cUIWindow;
class cNpcBackWindow;
class cChatManager;
class cChatWindow;
class cTargetWindow;
class cHeroInfoWindow;
class cCastingBar;
class cDestroyWarningWindow;
class cDropWarningWindow;
class cHeroStateWindow;
class cNpcWindow;
class cNpcStoreWindow;
class cInventoryWindow;
class cBagWindow;
class cWareHouseWindow;
class cWareHouseBagWindow;
class cItemCountWindow;
class cItemMsgWindow;
class cTradeWindow;
class cSkillWindow;
class cBuffWindow;
class cDragWindow;
class cMenuButtonWindow;
class cMiniMapWindow;
class cOptionWindow;
class cSmallMiniMapWindow;
class cColumnQuickSlot;
class cRowQuickSlot;
class cFriendWindow;
class cTarotWindow;
class cTarotControlWindow;
class cUserSellWindow;
class cItemTipWindow;
class cSkillTipWindow;
class cBuffToolTipWindow;
class cSimpleTipWindow;
class cExitWindow;
class cExitMsgWindow;
class cLootingWindow;
class cRevivalWindow;
class cPartyWindow;
class cPartyStateWindow;
class cPartyStateMiniWindow;
class cPartyMenu;
class cPartyDivideMenu;
class cMoneyWindow;
class cQuestWindow;
class cNpcRewardWindow;
class cNpcDisjointWindow;
class cEnhancedWindow;
class cQuestSimpleWindow;
class cMoneyWindow;
class cPutCardWindow;
class cDuelMsgWindow;
class cDuelWindow;
class cCardBookWindow;
class cChatExtraWindow;
class cFSColumnQuickSlot;
class cFSRowQuickSlot;
class cSocialWindow;
class cDMScoreWindow;
class cDMResultWindow;
class cDMMsgWindow;
class cDMRevivalWindow;
class cDMRankingListWindow;
class cChatButtonWindow;
class cTitleWindow;
class cMenuHeroWindow;
class cMenuCommuityWindow;
class cMenuOptionWindow;
class cMenuLifeWindow;
class cGuildWindow;
class cChatTipWindow;
class cRButtonMenuWindow;
class cGuildButtonWindow;
class cGuildMarkWindow;
class cTodayWordWindow;
class cItemMixWindow;
class cNpcDealWindow;
class cMailWindow;
class cPartyUnionWindow;
class cPartyUnionSmallWindow;
class cUnionDragWindow;
class cThemeEnterWindow;
class cThemeStateWindow;
class cQuestTipWindow;
class cSkillPreviewWindow;
class cShortcutWindow;
class cBoardWindow;
class cDiceWindow;
class cTutorialExitWindow;
class cNpcTutorialWindow;
class cMoveMsgWindow;
class cItemReceiptWindow;
class cItemMallWindow;
class cTwitterWindow;
class cTwitterRegisterWindow;
class cCamaelWindow;
class cMegaphoneWindow;
class cCashFreeMoveWindow;
class cThemeResetWindow;
class cItemDivideWindow;
class cLookWindow;
//class cCertifyWindow;
class cSummonWindow;

enum eItemTry;
enum eMoneyType;

/// save window pos struct
struct sWindowPosHeader
{
	unsigned short	version;
	unsigned int	dataCount;
};

struct sWindowPosData
{
	unsigned long	index;
	NiPoint2		pos;
	unsigned char	showFlag; // 0: hide, 1: show, 2: holdView
};

struct sChatWinOption
{
	bool	checkNormal;
	bool	checkParty;
	bool	checkGuild;
	bool	checkShout;
	bool	checkTrade;
	bool	checkWhisper;
	bool	checkSystem;
	bool	checkMegaPhone;
};

struct sInvenWinOption
{
	bool showBag1;
	bool showBag2;
	bool showBag3;
};

struct sSkillWinOption
{
	bool showStepListWin;

	sSkillWinOption()
	{
		showStepListWin = true;
	}
};

struct sMiniMapWinOption
{
	bool showHelper;
	bool showQuestView;
};

struct sPartyStateOption
{
	unsigned char showFlag; // 0 : stateWindow, 1 : stateMiniWindow, 2 : hide

	sPartyStateOption()
	{
		showFlag = 0;
	}
};

struct sQuestOption
{
	bool infoViewFlag;	// 0 : 꺼짐 1: 켜짐

	sQuestOption()
	{
		infoViewFlag = false;
	}
};

struct sMoveLockOption
{
	bool chatLock;
	bool partyLock;

	sMoveLockOption()
	{
		chatLock = false;
		partyLock = false;
	}
};

struct sTitleWinOption
{
	long titleIdxArr[5];

	sTitleWinOption()
	{	
		for( int i = 0; i < 5; i++ )
			titleIdxArr[i] = -1;
	}
};

struct sWindowOptCharacter
{
	unsigned short version;
	unsigned int questCount;
};

///

class cGameUIManager
{
	static cGameUIManager* mpSingleton;

public:
	cGameUIManager();
	~cGameUIManager();

	void Init();
	void Exit();
	void Open();
	void Close();
	void Process( unsigned long deltaTime, unsigned long accumTime );

	void InitHeroInfo();
	void UpdateHeroInfo();	/// 모든 스탯정보 갱신
	void UpdateHeroHPMP();	/// HP,MP만 갱신
	void UpdateExpSxp();
	void UpdateRecovery();
	void UpdateSkillPoint();
	void UpdateMinimap();

	/// 네트워크 
	void ReqFinish( bool exitEvent );

	// 캐릭터 선택으로 변경
	void SendGameOut();

	// 게임 종료 요청
	void SendDisConnect();
	
	void NetLockParty( int lockTry );	/// 파티와 관련있는 모든 UI LOCK
	void NetUnLockParty( int lockTry );

	/// 각종 윈도우의 상태 전환 함수들..
	void OnFocusChatWindow( unsigned long key, LPCTSTR addstring = _T("") );

	bool IsDuelMode() { return mDuelMode; }
	void SetDuelMode( bool mode );

	/// hero 상태에 따른 ui 닫기 
	void HideUIByDie();
	void HideUIByPushPull();

	/// 아이템 관련 세트 윈도우 닫기(조합창, 강화창, 카드장착창)
	void HideItemSetWindow( cUIWindow* exceptWin );

	///
	void ToggleGameUI();
	void ToggleOptionWindow();
	void ToggleHeroInfoWindow();
	void ToggleInventoryWindow();
	void ToggleSmallMiniMapWindow();
	void ToggleFriendWindow();
	void ToggleMiniMapWindow();
	void ToggleSkillWindow();
	void TogglePartyMenu();			/// 파티메뉴
	void TogglePartyDivideMenu();	/// 파티 분배메뉴
	void ToggleBagWindow( unsigned int index );
	void ToggleWareHouseBagWindow( unsigned int idx );
	void ToggleQuestWindow();
	void TogglePutCardWindow();
	void ToggleCardBookWindow();
	void ToggleSocialWindow();
	void ToggleGuildWindow();
	void ToggleThemeStateWindow();
	void ToggleShortcutWindow();	/// 단축키 설정 윈도우
	void ToggleTitleWindow();
	void ToggleItemMixWindow();
	void ToggleItemReceiptWindow();
	void ToggleTwitterWindow();
	void ToggleCamaelWindow();

	void ClearPartyWindow();		/// 파티윈도우
	void HidePartyMenu();			/// 파티메뉴
	void HidePartyDivideMenu();		/// 파티 분배메뉴
	void HideLootingWindow();		/// 루팅윈도우닫기 - 닫으면서 서버에 패킷 보냄
	void HideTarotControlWindow();
	void HideTradeWindow();	
	void HideCardBookWindow();
	void HideTodayWordWindow();
	void HideMailWindow();
	void HidePartyUnionWindow();
	void HidePartyUnionSmallWindow();
	void HideNpcBackWindow();
	void HideMoveMsgWindow();
	void HideCamaelWindow();

	void ShowHeroInfoWindow();
	void ShowFriendWindow();
	void ShowOptionWindow();
	void ShowLootingWindow();					/// 아이템 습득 상자 윈도우
	void ShowRevivalWindow();					/// 부활 윈도우
	void ShowTarotWindow( unsigned char page );	/// 타로개설, 설정 윈도우
	void ShowTarotControlWindow();
	void ShowItemCountWindow( eMSGBox_Event eventtype, unsigned long slotIndex0, unsigned long slotIndex1 );
	void ShowItemCountWindow( eMSGBox_Event eventtype, unsigned long itemIndex );
	void ShowItemCountWindow( eMSGBox_Event eventtype, unsigned long slotIndex, unsigned long itemIndex, unsigned long maxValue, unsigned long price );
	void ShowMoneyWindow( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex = USHRT_MAX );
	void ShowMoneyModifyWindow( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex = USHRT_MAX );
	void ShowNpcStoreWindow( unsigned long npcIndex );
	void ShowSkillWindow( bool backup = false );
	void ShowExitWindow();
	void ShowTutorialExitWindow();
	void ShowQuestWindow();
	void ShowNpcDisjointWindow();
	void ShowCardBookWindow();
	void ShowColumnMonSlot();
	void ShowGuildWindow();
	void ShowNpcTarotWindow( unsigned long* tarotArr, unsigned long price, unsigned long* spreadArr, short size );
	void ShowTodayWordWindow();
	void ShowNpcDealWindow();
	void ShowPartyUnionWindow();
	void ShowPartyUnionSmallWindow();
	void ShowThemeStateWindow();
	void ShowSkillPreviewWindow( unsigned long jobIndex1, unsigned long jobIndex2 );
	void ShowNpcBackWindow();
	void ShowMoveMsgWindow();
	void ShowReceiptWindow();
	void ShowSocialWindow();
	void ShowEnhanceWindow();
	void ShowCamaelWindow();
	void ShowInventory();
	void ShowWarehouse();

	void ShowItemDumpAskWindow( unsigned short slotIndex );
	void ShowItemBuyAskWindow( unsigned long itemIndex, unsigned int count );
	void ShowItemSellAskWindow( unsigned short slotIndex, unsigned int count );
	void ShowItemUserSellBuyAskWindow( unsigned short slotIndex, unsigned long itemIndex, 
									   unsigned short count, unsigned long price );
	void ShowItemNpcDealBuyAskWindow( unsigned long slotIndex, unsigned long itemIndex, 
									   unsigned short count, unsigned long price );
	void ShowItemPutCard( eMSGBox_Event eventtype, unsigned short slotIndex0, unsigned short putSlot, unsigned long itemIndex );
	void ShowItemCardBook( eMSGBox_Event eventtype, unsigned short slotIndex0, unsigned short slotIndex1, unsigned long itemIndex );

	bool IsAccessPartyObject ( int partyIdx );

	/// 루팅윈도우에 이벤트 주기
	void EventLootingWindow();

	void UpdateInvenWithUI( cUIWindow* win, bool backup = true );	// 인벤토리와 ui 의 위치 조정
	void UpdateInvenWithWare();									// 인벤토리와 창고 ui 위치 조정

	// npc 관련 윈도우 열기
	void OpenNpcUI( unsigned long npcIndex );

	cBaseObject* GetPartyObject( int i );

	cInventoryWindow*		GetInventoryWindow()	{ return mpInventoryWindow; }
	cTradeWindow*			GetTradeWindow()		{ return mpTradeWindow; }
	cItemTipWindow*			GetItemTip()			{ return mpItemTip; }
	cItemTipWindow*			GetEquipItemTip()		{ return mpEquipItemTip; }
	cSkillTipWindow*		GetSkillTip()			{ return mpSkillTip; }
	cBuffToolTipWindow*		GetBuffTip()			{ return mpBuffTip; }
	cSimpleTipWindow*		GetSimpleTip()			{ return mpSimpleTip; }
	cQuestTipWindow*		GetQuestTip()			{ return mpQuestTip; }
	cBuffWindow*			GetBuffWindow()			{ return mpBuffWindow; }
	cBuffWindow*			GetDeBuffWindow()		{ return mpDeBuffWindow; }
	cBuffWindow*			GetCashBuffWindow()		{ return mpCashBuffWindow; }
	cSkillWindow*			GetSkillWindow()		{ return mpSkillWindow; }
	cTargetWindow*			GetTargetWindow()		{ return mpTargetWindow; }
	cHeroInfoWindow*		GetHeroInfoWindow()		{ return mpHeroInfoWindow; }
	cHeroStateWindow*		GetHeroStateWindow()	{ return mpHeroStateWindow; }
	cPartyWindow*			GetPartyWindow()		{ return mpPartyWindow; }
	cPartyMenu*				GetPartyMenu()			{ return mpPartyMenu; }
	cPartyDivideMenu*		GetPartyDivideMenu()	{ return mpPartyDivideMenu; }
	cTarotWindow*			GetTarotWindow()		{ return mpTarotWindow; }
	cTarotControlWindow*	GetTarotControlWindow() { return mpTarotControlWindow; }
	cLootingWindow*			GetLootingWindow()		{ return mpLootingWindow; }
	cSmallMiniMapWindow*	GetSmallMinimapWindow() { return mpSmallMiniMapWindow; }
	cMiniMapWindow*			GetMinimapWindow()		{ return mpMiniMapWindow; }
	cFriendWindow*			GetFriendWindow()		{ return mpFriendWindow; }
	cMenuButtonWindow*		GetMenuButtonWindow()	{ return mpMenuButtonWindow; }
	cMenuOptionWindow*		GetMenuOptionWindow()	{ return mpMenuOption; }
	cMenuLifeWindow*		GetMenuLifeWindow()		{ return mpMenuLife; }
	cMenuHeroWindow*		GetMenuHeroWindow()		{ return mpMenuHero; }
	cRevivalWindow*			GetReVivalWindow()		{ return mpRevivalWindow; }
	cNpcStoreWindow*		GetNpcStore()			{ return mpNpcStoreWindow; }
	cWareHouseWindow*		GetWareHouseWindow()	{ return mpWareHouseWindow; }
	cMoneyWindow*			GetMoneyWindow()		{ return mpMoneyWindow; }
	cQuestWindow*			GetQuestWindow()		{ return mpQuestWindow; }
	cNpcWindow*				GetNpcWindow()			{ return mpNpcWindow; }
	cNpcRewardWindow*		GetNpcRewardWindow()	{ return mpNpcRewardWindow; }
	cNpcDisjointWindow*		GetNpcDisjointWindow()	{ return mpNpcDisjointWindow; }
	cNpcTutorialWindow*		GetNpcTutorialWindow()	{ return mpNpcTutorialWindow; }
	cExitWindow*			GetExitWindow()			{ return mpExitWindow; }
	cExitMsgWindow*			GetExitMsgWindow()		{ return mpExitMsgWindow; }
	cEnhancedWindow*		GetEnhancedWindow()		{ return mpEnhancedWindow; }
	cQuestSimpleWindow*		GetQuestSimpleWindow()	{ return mpQuestSimpleWindow; }
	cUserSellWindow*		GetUserSellWindow()		{ return mpUserSellWindow; }
	cItemMsgWindow*			GetItemMsgWindow()		{ return mpItemMsgWindow; }
	cItemCountWindow*		GetItemCountWindow()	{ return mpItemCountWindow; }
	cPutCardWindow*			GetPutCardWindow()		{ return mpPutCardWindow; }
	cDuelMsgWindow*			GetDuelMsgWindow()		{ return mpDuelMsgWindow; }
	cDuelWindow*			GetDuelWindow()			{ return mpDuelWindow; }
	cCardBookWindow*		GetCardBookWindow()		{ return mpCardBookWindow; }
	cSocialWindow*			GetSocialWindow()		{ return mpSocialWindow; }
	cDMScoreWindow*			GetDMScoreWindow()		{ return mpDMScoreWindow; }
	cDMResultWindow*		GetDMResultWindow()		{ return mpDMResultWindow; }
	cDMRankingListWindow*	GetDMRankingListWindow() { return mpDMRankingListWindow; }
	cChatButtonWindow*		GetChatButtonWindow()	{ return mpChatButtonWindow; }
	cChatWindow*			GetChatWindow()			{ return mpChatWindow; }
	cTitleWindow*			GetTitleWindow()		{ return mpTitleWindow; }
	cGuildWindow*			GetGuildWindow()		{ return mpGuildWindow; }
	cFSColumnQuickSlot*		GetFSColumnQuickSlot()  { return mpFSColumnQuickSlot; }
	cFSRowQuickSlot*		GetFSRowQuickSlot()		{ return mpFSRowQuickSlot; }
	cGuildButtonWindow*		GetGuildButtonWindow()  { return mpGuildButtonWindow; }
	cGuildMarkWindow*		GetGuildMarkWindow()	{ return mpGuildMarkWindow; }
	cTodayWordWindow*		GetTodayWordWindow()	{ return mpTodayWordWindow; }
	cItemMixWindow*			GetItemMixWindow()		{ return mpItemMixWindow; }
	cNpcDealWindow*			GetNpcDealWindow()		{ return mpNpcDealWindow; }
	cMailWindow*			GetMailWindow()			{ return mpMailWindow; }
	cPartyUnionWindow*		GetPartyUnionWindow()   { return mpUnionWindow; }
	cPartyUnionSmallWindow*	GetPartyUnionSmallWindow()   { return mpUnionSmallWindow; }
	cThemeEnterWindow*		GetThemeEnterWindow()	{ return mpThemeEnterWindow; }
	cThemeStateWindow*		GetThemeStateWindow()	{ return mpThemeStateWindow; }
	cDMMsgWindow*			GetDMMsgWindow()		{ return mpDMMsgWindow; }
	cSkillPreviewWindow*	GetSkillPreviewWindow() { return mpSkillPreviewWindow; }
	cShortcutWindow*		GetShortcutWindow()		{ return mpShortcutWindow; }
	cBoardWindow*			GetBoardWindow()		{ return mpBoardWindow; }
	cDiceWindow*			GetDiceWindow()			{ return mpDiceWindow; }
	cMoveMsgWindow*			GetMoveMsgWindow()		{ return mpMoveMsgWindow; }
	cItemReceiptWindow*		GetItemReceiptWindow()  { return mpReceiptWindow; }
	cOptionWindow*			GetOptionWindow()		{ return mpOptionWindow; }
	cTwitterWindow*			GetTwitterWindow()		{ return mpTwitterWindow; }
	cTwitterRegisterWindow*	GetTwitterRegisterWindow(){ return mpTwitterRegisterWindow; }
	cMegaphoneWindow*		GetMegaphoneWindow()	{ return mpMegaphoneWindow; }
	cThemeResetWindow*		GetThemeResetWindow()	{ return mpThemeResetWindow;}
	cItemDivideWindow*		GetItemDivideWindow()	{ return mpItemDivideWindow; }
	cLookWindow*			GetLookWindow()			{ return mpLookWindow; }
//	cChannelMoveWindow*		GetChannelMoveWindow()  { return mpChannelMoveWindow; }
//	cCertifyWindow*			GetCertifyWindow()		{ return mpCertifyWindow; }
	cSummonWindow*			GetSummonWindow()		{ return mpSummonWindow; }

	cColumnQuickSlot*		GetColumnQuickSlot( int type );
	cRowQuickSlot*			GetRowQuickSlot( int type );

	cBagWindow*				GetBagWindow( unsigned int i ) { return mpBagWindow[i]; }
	cWareHouseBagWindow*	GetWareHouseBagWindow( unsigned int i)	{ return mpWareHouseBagWindow[i]; }

	cPartyStateWindow*		GetPartyStateWindow( unsigned int idx );
	cPartyStateMiniWindow*  GetPartyStateMiniWindow( unsigned int idx );

	cRButtonMenuWindow*		GetRButtonMenuWindow() { return mpRButtonMenuWindow; }

	cCamaelWindow*			GetCamaelWindow(){ return mpCamaelWindow; }
	cCashFreeMoveWindow*    GetCashFreeMoveWindow(){ return mpCashFreeMoveWindow; }

	void ApplyWinodwOption_CharacterQuest();
	void ApplyWinodwOption_CharacterShortcut();

public:
	/// 단일체를 리턴
	static cGameUIManager* GetSingleton();

protected:
	/// window player save pos
	bool LoadWindowPos( const cString& pathName );
	void SaveWindowPos( const cString& pathName );

	bool LoadWindowOption_Character( const cString& pathName );
	void SaveWindowOption_Character( const cString& pathName );

	void UpdateWindowOption_Character();

	void ApplyWindowPos();
	void UpdateWindowPos();

protected:
	/// 채팅 관리자
	cChatManager* mpChatManager;

	/// 채팅 윈도우
	cChatWindow* mpChatWindow;

	/// 채팅 확장 윈도우
	cChatExtraWindow* mpChatExtraWindow[3];

	/// 타겟 윈도우
	cTargetWindow* mpTargetWindow;

	/// Hero 정보 윈도우
	cHeroInfoWindow* mpHeroInfoWindow;

	/// Hero 상태 윈도우 ( 화면 왼쪽 상단 )
	cHeroStateWindow* mpHeroStateWindow;

	/// Npc 윈도우
	cNpcBackWindow* mpNpcBackWindow;
	cNpcWindow* mpNpcWindow;
	cNpcStoreWindow* mpNpcStoreWindow;

	/// 인벤토리 윈도우
	cInventoryWindow* mpInventoryWindow;

	/// 가방 윈도우
	cBagWindow* mpBagWindow[3];

	/// 창고 윈도우
	cWareHouseWindow* mpWareHouseWindow;
	cWareHouseBagWindow* mpWareHouseBagWindow[3];
	cMoneyWindow*	mpMoneyWindow;

	/// 아이템
	cItemCountWindow* mpItemCountWindow;

	/// 아이템 메세지 윈도우
	cItemMsgWindow* mpItemMsgWindow;

	/// 거래 윈도우
	cTradeWindow* mpTradeWindow;

	/// 스킬 윈도우
	cSkillWindow* mpSkillWindow;

	cBuffWindow* mpBuffWindow;
	cBuffWindow* mpDeBuffWindow;
	cBuffWindow* mpCashBuffWindow;

	/// 드래그 윈도우
	cDragWindow* mpDragWindow;

	/// 팁 윈도우
	cItemTipWindow* mpItemTip;
	cItemTipWindow* mpEquipItemTip;
	cSkillTipWindow* mpSkillTip;
	cBuffToolTipWindow* mpBuffTip;
	cSimpleTipWindow* mpSimpleTip;
	cChatTipWindow*	mpChatItemTip;
	cQuestTipWindow*	mpQuestTip;

	/// 하단 버튼 윈ㄷ우
	cMenuButtonWindow* mpMenuButtonWindow;
	
	/// 하단 메뉴 윈도우
	cMenuHeroWindow*		mpMenuHero;
	cMenuCommuityWindow*	mpMenuCommu;
	cMenuOptionWindow*		mpMenuOption;
	cMenuLifeWindow*		mpMenuLife;

	/// 미니맵 윈도우
	cMiniMapWindow* mpMiniMapWindow;

	/// 캐스팅 게이지
	cCastingBar* mpCastingBar;

	/// 옵션 윈도우
	cOptionWindow* mpOptionWindow;

	/// small 미니맵 윈도우
	cSmallMiniMapWindow* mpSmallMiniMapWindow;

	/// 친구 윈도우
	cFriendWindow* mpFriendWindow;

	/// 퀵슬롯 (행)
	cColumnQuickSlot* mpColumnQuickSlot[3];

	/// 퀵슬롯 (열)
	cRowQuickSlot* mpRowQuickSlot[3];

	/// FS 퀵슬롯
	cFSColumnQuickSlot* mpFSColumnQuickSlot;

	/// FS 퀵슬롯
	cFSRowQuickSlot* mpFSRowQuickSlot;

	/// 타롯 점집 메인..
	cTarotWindow*		mpTarotWindow;

	/// 타롯 점집 관리 윈도우 ( 개설자 )
	cTarotControlWindow* mpTarotControlWindow;

	/// 노점상 개설 윈도우 
	cUserSellWindow* mpUserSellWindow;

	/// 게임 종료 윈도우
	cExitWindow*	mpExitWindow;
	cTutorialExitWindow* mpTutorialExitWindow;
	cExitMsgWindow* mpExitMsgWindow;

	/// 아이템 습득 상자 윈도우
	cLootingWindow*		mpLootingWindow;

	/// 부활 윈도우
	cRevivalWindow*		mpRevivalWindow;

	/// 파티 관리 윈도우
	cPartyWindow*	mpPartyWindow;

	/// 파티 상태 윈도우
	cPartyStateWindow*	mpPartyStateWindow[5];

	/// 파티 상태 Mini 윈도우
	cPartyStateMiniWindow* mpPartyStateMiniWindow[5];

	/// 파티 메뉴
	cPartyMenu*		mpPartyMenu;

	/// 파티 분배 메뉴
	cPartyDivideMenu*	mpPartyDivideMenu;

	/// 퀘스트 윈도우
	cQuestWindow*	mpQuestWindow;

	/// npc 보상 정보 윈도우
	cNpcRewardWindow*	mpNpcRewardWindow;

	/// npc 분해 윈도우
	cNpcDisjointWindow*	 mpNpcDisjointWindow;

	/// 아이템 강화 윈도우
	cEnhancedWindow*	mpEnhancedWindow;

	/// 퀘스트 간략정보 출력 윈도우
	cQuestSimpleWindow*	mpQuestSimpleWindow;

	/// 카드장착 윈도우
	cPutCardWindow*		mpPutCardWindow;

	/// 결투 윈도우
	cDuelMsgWindow*		mpDuelMsgWindow;
	cDuelWindow*		mpDuelWindow;
	bool				mDuelMode;

	/// 카드 컬렉션 북
	cCardBookWindow*	mpCardBookWindow;

	/// 사회 윈도우
	cSocialWindow*		mpSocialWindow;

	/// 데스매치 점수 윈도우
	cDMScoreWindow*		mpDMScoreWindow;
	cDMRankingListWindow* mpDMRankingListWindow;

	/// 데스매치 결과 윈도우
	cDMResultWindow*	mpDMResultWindow;

	///
	cDMMsgWindow*		mpDMMsgWindow;

	///
	cChatButtonWindow*	mpChatButtonWindow;

	/// 호칭관리 윈도우
	cTitleWindow*		mpTitleWindow;

	/// 길드 직위 버튼 윈도우
	cGuildButtonWindow* mpGuildButtonWindow;

	/// 길드관리 윈도우
	cGuildWindow*		mpGuildWindow;

	/// 길드마크 등록 윈도우
	cGuildMarkWindow*	mpGuildMarkWindow;

	///
	cRButtonMenuWindow*	mpRButtonMenuWindow;

	/// 오늘의 한마디 윈도우
	cTodayWordWindow*	mpTodayWordWindow;

	/// 아이템 조합 윈도우
	cItemMixWindow*		mpItemMixWindow;

	/// npc 판매대행 윈도우
	cNpcDealWindow*		mpNpcDealWindow;

	/// 우편 윈도우
	cMailWindow*		mpMailWindow;

	/// 파티 연합 윈도우
	cPartyUnionWindow*		mpUnionWindow;
	cPartyUnionSmallWindow* mpUnionSmallWindow;

	/// 연합용 드래그 윈도우
	cUnionDragWindow*		mpUnionDragWindow;

	cThemeEnterWindow*		mpThemeEnterWindow;

	/// 테마 던젼 상태 정보 윈도우
	cThemeStateWindow*		mpThemeStateWindow;

	/// 스킬 찍어보기 윈도우
	cSkillPreviewWindow*	mpSkillPreviewWindow;

	/// 단축키 설정 윈도우
	cShortcutWindow*		mpShortcutWindow;

	/// 게시판 윈도우
	cBoardWindow*			mpBoardWindow;

	/// 주사위 굴리기 윈도우
	cDiceWindow*			mpDiceWindow;

	// 튜토리얼book 윈도우
	cNpcTutorialWindow*	mpNpcTutorialWindow;
	
	/// 따라가기/찾아가기 메시지 윈도우
	cMoveMsgWindow*		mpMoveMsgWindow;

	/// 아이템 수령함
	cItemReceiptWindow* mpReceiptWindow;

	// 아이템 몰
	cItemMallWindow*	mpItemMallWindow;

	/// 트위터
	cTwitterWindow*			mpTwitterWindow;
	cTwitterRegisterWindow*	mpTwitterRegisterWindow;

	/// 카마엘 윈도우
	cCamaelWindow*		mpCamaelWindow;

	/// 메가폰 윈도우
	cMegaphoneWindow*	mpMegaphoneWindow;

	/// 캐시 아이템 - 자유이동 주문서
	cCashFreeMoveWindow*	mpCashFreeMoveWindow;

	/// 테마던전 귀속 초기화 윈도우
	cThemeResetWindow*	mpThemeResetWindow;

	/// 아이템 분배 윈도우
	cItemDivideWindow*	mpItemDivideWindow;

	/// 살펴보기윈도우
	cLookWindow*	mpLookWindow;

	/// 인증 윈도우
//	cCertifyWindow* mpCertifyWindow;

	/// 편의 기능 소환 윈도우
	cSummonWindow*	mpSummonWindow;

	/// save window pos
	typedef tPointerHashMap<unsigned long, void*> cSaveWindowPos;
	typedef tArray<unsigned int> cSaveQuestArr;
	cSaveWindowPos	mSaveWindowInfo;

	sChatWinOption	mChatWinOption[4];
	sInvenWinOption	mInvenWinOption;
	sMiniMapWinOption mMiniMapWinOption;
	sPartyStateOption mPartyStateOption;
	sQuestOption	 mQuestOption;
	sMoveLockOption  mMoveLockOption;
	sSkillWinOption	mSkillWinOption;
	sTitleWinOption	mTitleOption;

	int				mChatBoxMaxRow;
	int				mExtraChatBoxMaxRow[3];
	float			mMiniWinPer;

	unsigned long	mQuestCharacterIdx;
	cSaveQuestArr	mQuestCheckArr;

	typedef tHashSet<unsigned int> cSaveQuestSet;
	cSaveQuestSet	mClosedCategorySet;
	cSaveQuestSet	mClosedSimpleCategorySet;

	typedef tHashMap< int, int  >	cShortcutMap;
	cShortcutMap	mShortcutMap;

};

inline cPartyStateWindow* cGameUIManager::GetPartyStateWindow( unsigned int idx )
{
	if( idx >= 0 && idx < 5 )
		return mpPartyStateWindow[idx];
	else
		return 0;
}

inline cPartyStateMiniWindow* cGameUIManager::GetPartyStateMiniWindow( unsigned int idx )
{
	if( idx >= 0 && idx < 5 )
		return mpPartyStateMiniWindow[idx];
	else
		return 0;
}

inline cGameUIManager* cGameUIManager::GetSingleton()
{
	return mpSingleton;
}

#define GAMEUI cGameUIManager::GetSingleton()
