#include "stdafx.h"
#include "GameUIManager.h"

#include "RenderSystem.h"
#include "GameResourceManager.h"
#include "StageManager.h"
#include "UIBackWindow.h"
#include "ChatWindow.h"
#include "TargetWindow.h"
#include "HeroInfoWindow.h"
#include "CastingBar.h"
#include "HeroStateWindow.h"
#include "MenuWindow.h"
#include "NpcWindow.h"
#include "NpcStoreWindow.h"
#include "InventoryWindow.h"
#include "WareHouseWindow.h"
#include "ItemCountWindow.h"
#include "ItemMsgWindow.h"
#include "TradeWindow.h"
#include "DragWindow.h"
#include "TipWindow.h"
#include "MiniMapWindow.h"
#include "OptionWindow.h"
#include "SmallMiniMapWindow.h"
#include "SkillWindow.h"
#include "SkillStoreWindow.h"
#include "BuffWindow.h"
#include "ColumnQuickSlot.h"
#include "RowQuickSlot.h"
#include "FriendWindow.h"
#include "TarotWindow.h"
#include "TarotControlWindow.h"
#include "UserSellWindow.h"
#include "BagWindow.h"
#include "ExitWindow.h"
#include "LootingWindow.h"
#include "RevivalWindow.h"
#include "PartyWindow.h"
#include "PartyStateWindow.h"
#include "QuestWindow.h"
#include "NpcRewardWindow.h"
#include "NpcDisjointWindow.h"
#include "EnhancedWindow.h"
#include "QuestSimpleWindow.h"
#include "MoneyWindow.h"
#include "PutCardWindow.h"
#include "DuelMsgWindow.h"
#include "DuelWindow.h"
#include "PartyManager.h"
#include "CardBookWindow.h"
#include "ChatExtraWindow.h"
#include "SocialWindow.h"
#include "DMScoreWindow.h"
#include "DMMsgWindow.h"
#include "TitleWindow.h"
#include "GuildWindow.h"
#include "RButtonMenuWindow.h"
#include "TodayWordWindow.h"
#include "ItemMixWindow.h"
#include "NpcDealWindow.h"
#include "MailWindow.h"
#include "PartyUnionWindow.h"
#include "ThemeEnterWindow.h"
#include "ThemeStateWindow.h"
#include "SkillPreviewWindow.h"
#include "ShortcutWindow.h"
#include "BoardWindow.h"
#include "DiceWindow.h"
#include "NpcTutorialWindow.h"
#include "MoveMsgWindow.h"
#include "ItemReceiptWindow.h"
#include "ItemMallWindow.h"
#include "TwitterWindow.h"
#include "CamaelWindow.h"
#include "MegaphoneWindow.h"
#include "CashFreeMoveWindow.h"
#include "LookWindow.h"
//#include "ChannelWindow.h"
//#include "CertifyWindow.h"
#include "SummonWindow.h"

#include "ItemTipWindow.h"
#include "SkillTipWindow.h"
#include "BuffToolTipWindow.h"
#include "SimpleTipWindow.h"

#include "UIManager.h"
#include "UIRootNode.h"

#include "QuestManager.h"
#include "PVPManager.h"
#include "ObjectManager.h"
#include "ChatManager.h"
#include "hero.h"
#include "UIMsgboxEvent.h"
#include "Application.h"
#include "KeyboardAgent.h"
#include "ObjectManager.h"
#include "TarotManager.h"
#include "CheatManager.h"
#include "ItemMixManager.h"
#include "TutorialManager.h"
#include "TitleManager.h"
#include "Hero.h"

#include "Npc_Common.h"
#include "Npc.h"
#include "UINpcTelling.h"
#include "NpcScript.h"

#include "ShortcutManager.h"

cGameUIManager* cGameUIManager::mpSingleton = 0;

#define POSHEARDER_VER		12
#define CHARHEARDER_VER		5

cGameUIManager::cGameUIManager(void)
: mpChatWindow(0)
, mpTargetWindow(0)
, mpHeroInfoWindow(0)
, mpInventoryWindow( 0 )
, mpWareHouseWindow( 0 )
, mpItemCountWindow( 0 )
, mpItemMsgWindow(0)
, mpTradeWindow( 0 )
, mpDragWindow( 0 )
, mpUnionDragWindow(0)
, mpCastingBar(0)
, mpChatManager(0)
, mpHeroStateWindow(0)
, mpSkillWindow(0)
, mpBuffWindow(0)
, mpDeBuffWindow(0)
, mpCashBuffWindow(0)
, mpNpcWindow( 0 )
, mpNpcBackWindow( 0 )
, mpNpcStoreWindow( 0 )
, mpMiniMapWindow( 0 )
, mpOptionWindow(0)
, mpSmallMiniMapWindow(0)
, mpFriendWindow(0)
, mpTarotWindow(0)
, mpTarotControlWindow(0)
, mpUserSellWindow(0)
, mpItemTip(0)
, mpEquipItemTip(0)
, mpSkillTip(0)
, mpBuffTip(0)
, mpSimpleTip(0)
, mpMenuButtonWindow(0)
, mpMenuHero(0)
, mpMenuCommu(0)
, mpMenuOption(0)
, mpMenuLife(0)
, mpExitWindow(0)
, mpTutorialExitWindow(0)
, mpExitMsgWindow(0)
, mpLootingWindow(0)
, mpRevivalWindow(0)
, mpPartyWindow(0)
, mpPartyMenu(0)
, mpPartyDivideMenu(0)
, mpMoneyWindow(0)
, mpQuestWindow(0)
, mpNpcRewardWindow(0)
, mpNpcDisjointWindow(0)
, mpEnhancedWindow(0)
, mpQuestSimpleWindow(0)
, mpPutCardWindow(0)
, mpDuelMsgWindow(0)
, mpDuelWindow(0)
, mpCardBookWindow(0)
, mpSocialWindow(0)
, mpDMScoreWindow(0)
, mpDMResultWindow(0)
, mpFSColumnQuickSlot(0)
, mpFSRowQuickSlot(0)
, mpDMMsgWindow(0)
, mpDMRankingListWindow(0)
, mpChatButtonWindow(0)
, mpChatItemTip(0)
, mpTitleWindow(0)
, mpGuildWindow(0)
, mpRButtonMenuWindow(0)
, mpGuildButtonWindow(0)
, mpGuildMarkWindow(0)
, mpTodayWordWindow(0)
, mpItemMixWindow(0)
, mChatBoxMaxRow(0)
, mpNpcDealWindow(0)
, mpMailWindow(0)
, mpUnionWindow(0)
, mpUnionSmallWindow(0)
, mpThemeEnterWindow(0)
, mpThemeStateWindow(0)
, mpQuestTip(0)
, mpSkillPreviewWindow(0)
, mpShortcutWindow(0)
, mpBoardWindow(0)
, mpDiceWindow(0)
, mpNpcTutorialWindow(0)
, mpMoveMsgWindow(0)
, mpReceiptWindow(0)
, mpItemMallWindow(0)
, mpTwitterWindow(0)
, mpTwitterRegisterWindow(0)
, mpCamaelWindow(0)
, mpMegaphoneWindow(0)
, mpCashFreeMoveWindow(0)
, mpThemeResetWindow(0)
, mpItemDivideWindow(0)
, mpLookWindow(0)
//, mpCertifyWindow(0)
, mpSummonWindow(0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	for( int i = 0; i < 3; ++i )
	{
		mpBagWindow[i] = 0;
		mpWareHouseBagWindow[i] = 0;
	}

	for( int i = 0; i < 5; ++i )
	{
		mpPartyStateWindow[i] = 0;
		mpPartyStateMiniWindow[i] = 0;
	}

	for( int i = 0; i < 3; ++i )
	{
		mpChatExtraWindow[i] = 0;
		mExtraChatBoxMaxRow[i] = 0;
	}

	for( int i = 0; i < QUICKTYPE_COMMON_COUNT; ++i )
	{
		mpColumnQuickSlot[i] = 0;
		mpRowQuickSlot[i] = 0;
	}

	mDuelMode = false;
	mMiniWinPer = 0;

	mQuestCharacterIdx = 0;
}

cGameUIManager::~cGameUIManager()
{
	cSaveWindowPos::cIterator i = mSaveWindowInfo.Begin();
	cSaveWindowPos::cIterator end = mSaveWindowInfo.End();
	for( ; i!=end; i++ )
	{
		delete (sWindowPosData*)(i->mSecond);
	}
	mSaveWindowInfo.Clear();

	mpSingleton = 0;
}

void cGameUIManager::Init()
{
	UIMAN->SetCurrentStage( eStage_Game );

	cUIProperty* p = UIMAN->GetProperty();
	if( !p )
	{
		assert(0);
		return;
	}

	/// 채팅 관리자 생성 및 초기화
	mpChatManager = new cChatManager;
	if( !mpChatManager || !mpChatManager->Init() )
	{
		SAFE_DELETE( mpChatManager );
		return;
	}

	/// 미니맵 윈도우
	mpMiniMapWindow = new cMiniMapWindow;
	mpMiniMapWindow->CreateByPropertyName( "P_MiniMap_Window", 0 );

	/// 스몰 미니맵 윈도우
	mpSmallMiniMapWindow = new cSmallMiniMapWindow;
	mpSmallMiniMapWindow->CreateByPropertyName( "P_SmallMiniMap_Window", 0 );

	/// 채팅창 버튼 윈도우
	mpChatButtonWindow = new cChatButtonWindow;
	mpChatButtonWindow->CreateByPropertyName( "P_ChatButtonWindow", 0 );
	UIMAN->AddEventOtherClick( mpChatButtonWindow );

	/// 채팅윈도우
	mpChatWindow = new cChatWindow;
	mpChatWindow->CreateByPropertyName( "P_ChatWindow", 0 );

	cUINodeProperty* chatExtraProp = p->GetNodeProperty( "P_ChatExtraWindow" );
	if( chatExtraProp )
	{
		for( int i = 0; i < 3; ++i )
		{
			mpChatExtraWindow[i] = new cChatExtraWindow;
			mpChatExtraWindow[i]->CreateByPropertyId( chatExtraProp, 0, eUIID_GAME_EXTRA_CHATWINDOW1 + i );
			mpChatExtraWindow[i]->SetPos( i );

			if( mpChatWindow )
				mpChatWindow->AddExtraWindow( (cUIWindow*)mpChatExtraWindow[i] );
		}
	}

	if( mpChatExtraWindow[0] )
		mpChatExtraWindow[0]->Show( false );

	/// 타겟 윈도우 
	mpTargetWindow = new cTargetWindow;
	mpTargetWindow->CreateByPropertyName( "P_TargetWindow", 0 );

	/// 캐릭터 정보 윈도우
	mpHeroInfoWindow = new cHeroInfoWindow;
	mpHeroInfoWindow->CreateByPropertyName( "P_HeroInfoWindow", 0 );

	/// 캐스팅 게이지
	mpCastingBar = new cCastingBar;
	mpCastingBar->CreateByPropertyName( "P_CastingBar", 0 );

	/// 캐릭터 상태 윈도우 ( 왼쪽 상단 )
	mpHeroStateWindow = new cHeroStateWindow;
	mpHeroStateWindow->CreateByPropertyName( "P_HeroStateWindow", 0 );

	/// Npc 윈도우
	mpNpcWindow = new cNpcWindow;
	mpNpcWindow->CreateByPropertyName( "P_NpcTelling_Window", 0 );

	/// npc 보상 정보 윈도우
	mpNpcRewardWindow = new cNpcRewardWindow;
	mpNpcRewardWindow->CreateByPropertyName( "P_NpcReward_Window", mpNpcWindow );

	/// Npc 배경 윈도우
	mpNpcBackWindow = new cNpcBackWindow;
	mpNpcBackWindow->SetOnlyBackground( true );
	mpNpcBackWindow->CreateBySkinName( "NpcBack_Window", 0, 1 );

	/// Npc 상점 윈도우
	mpNpcStoreWindow = new cNpcStoreWindow;
	mpNpcStoreWindow->CreateByPropertyName( "P_NpcStore_Window", 0 );

	/// 퀵슬롯 (행)
	mpColumnQuickSlot[QUICKTYPE_COMMON] = new cColumnQuickSlot( QUICKTYPE_COMMON );
	mpColumnQuickSlot[QUICKTYPE_COMMON]->CreateByPropertyName( "P_ColumnQuickSlot", 0 );
	/// 퀵슬롯 (추가1 행)
	mpColumnQuickSlot[QUICKTYPE_COMMON_EXTRA1] = new cColumnQuickSlot( QUICKTYPE_COMMON_EXTRA1 );
	mpColumnQuickSlot[QUICKTYPE_COMMON_EXTRA1]->CreateByPropertyName( "P_ColumnExtraSlot1", 0 );
	/// 퀵슬롯 (추가2 행)
	mpColumnQuickSlot[QUICKTYPE_COMMON_EXTRA2] = new cColumnQuickSlot( QUICKTYPE_COMMON_EXTRA2 );
	mpColumnQuickSlot[QUICKTYPE_COMMON_EXTRA2]->CreateByPropertyName( "P_ColumnExtraSlot2", 0 );

	/// 퀵슬롯 (열)
	mpRowQuickSlot[QUICKTYPE_COMMON] = new cRowQuickSlot( QUICKTYPE_COMMON );
	mpRowQuickSlot[QUICKTYPE_COMMON]->CreateByPropertyName( "P_RowQuickSlot", 0 );
	/// 퀵슬롯 (열)
	mpRowQuickSlot[QUICKTYPE_COMMON_EXTRA1] = new cRowQuickSlot( QUICKTYPE_COMMON_EXTRA1 );
	mpRowQuickSlot[QUICKTYPE_COMMON_EXTRA1]->CreateByPropertyName( "P_RowExtraSlot1", 0 );
	/// 퀵슬롯 (열)
	mpRowQuickSlot[QUICKTYPE_COMMON_EXTRA2] = new cRowQuickSlot( QUICKTYPE_COMMON_EXTRA2 );
	mpRowQuickSlot[QUICKTYPE_COMMON_EXTRA2]->CreateByPropertyName( "P_RowExtraSlot2", 0 );

	/// FS 퀵슬롯
	mpFSColumnQuickSlot = new cFSColumnQuickSlot;
	mpFSColumnQuickSlot->CreateByPropertyName( "P_FSColumnQuickSlot", 0 );

	/// FS 퀵슬롯
	mpFSRowQuickSlot = new cFSRowQuickSlot;
	mpFSRowQuickSlot->CreateByPropertyName( "P_FSRowQuickSlot", 0 );

	/// 인벤토리 윈도우
	mpInventoryWindow = new cInventoryWindow;
	mpInventoryWindow->CreateByPropertyName( "P_Inventory_Window", 0 );

	/// 가방 윈도우
	mpBagWindow[0] = new cBagWindow( mpInventoryWindow, 0 );
	mpBagWindow[0]->CreateByPropertyName( "P_Bag0_Window", 0 );

	mpBagWindow[1] = new cBagWindow( mpInventoryWindow, 1 );
	mpBagWindow[1]->CreateByPropertyName( "P_Bag1_Window", 0 );

	mpBagWindow[2] = new cBagWindow( mpInventoryWindow, 2 );
	mpBagWindow[2]->CreateByPropertyName( "P_Bag2_Window", 0 );

	/// 아이템 개수 윈도우
	mpItemCountWindow = new cItemCountWindow;
	mpItemCountWindow->CreateByPropertyName( "P_ItemCount_Window", 0 );

	/// 창고 윈도우
	mpWareHouseWindow = new cWareHouseWindow;
	mpWareHouseWindow->CreateByPropertyName( "P_WareHouse_Window", 0 );

	mpWareHouseBagWindow[0] = new cWareHouseBagWindow( mpWareHouseWindow, 0 );
	mpWareHouseBagWindow[0]->CreateByPropertyName( "P_WareHouseBag1_Window", 0 );

	mpWareHouseBagWindow[1] = new cWareHouseBagWindow( mpWareHouseWindow, 1 );
	mpWareHouseBagWindow[1]->CreateByPropertyName( "P_WareHouseBag2_Window", 0 );

	mpWareHouseBagWindow[2] = new cWareHouseBagWindow( mpWareHouseWindow, 2 );
	mpWareHouseBagWindow[2]->CreateByPropertyName( "P_WareHouseBag3_Window", 0 );

	mpMoneyWindow = new cMoneyWindow;
	mpMoneyWindow->CreateByPropertyName( "P_Money_Window", 0 );

	/// 거래 윈도우
	mpTradeWindow = new cTradeWindow;
	mpTradeWindow->CreateByPropertyName( "P_Trade_Window", 0 );

	/// 스킬 윈도우
	mpSkillWindow = new cSkillWindow;
	mpSkillWindow->CreateByPropertyName( "P_SkillWindow", 0 );

	/// 버프 창
	mpBuffWindow = new cBuffWindow;
	mpBuffWindow->CreateByPropertyName( "P_BuffWindow", 0 );
	mpBuffWindow->SetHeroBuff( true );

	/// 디버프 창
	mpDeBuffWindow = new cBuffWindow;
	mpDeBuffWindow->CreateByPropertyName( "P_DeBuffWindow", 0 );
	mpDeBuffWindow->SetHeroBuff( true );

	// 캐쉬 버프 윈도우
	mpCashBuffWindow = new cBuffWindow;
	mpCashBuffWindow->CreateByPropertyName( "P_MallBuffWindow", 0 );
	mpCashBuffWindow->SetHeroBuff( true );

	/// 드래그 윈도우
	mpDragWindow = new cDragWindow;
	mpDragWindow->CreateByPropertyName( "P_Drag_Window", 0 );
	UIMAN->SetDragWindow( mpDragWindow );

	/// 연합용 드래그 윈도우
	mpUnionDragWindow = new cUnionDragWindow;
	mpUnionDragWindow->CreateByPropertyName( "P_UnionDrag_Window", 0 );
	UIMAN->SetUnionDragWindow( mpUnionDragWindow );

	/// 아이템팁 윈도우
	mpItemTip = new cItemTipWindow;
	mpItemTip->CreateByPropertyName( "P_ItemTip", 0 );

	///
	mpEquipItemTip = new cItemTipWindow( true );
	mpEquipItemTip->CreateByPropertyName( "P_EquipTip", 0 );

	/// 스킬 팁 윈도우
	mpSkillTip = new cSkillTipWindow;
	mpSkillTip->CreateByPropertyName( "P_SkillTip", 0 );

	/// 버프 팁 윈도우 
	mpBuffTip = new cBuffToolTipWindow;
	mpBuffTip->CreateByPropertyName( "P_BuffTip", 0 );

	///
	mpSimpleTip = new cSimpleTipWindow;
	mpSimpleTip->CreateByPropertyName( "P_SimpleTip", 0 );

	/// 
	mpQuestTip = new cQuestTipWindow;
	mpQuestTip->CreateByPropertyName( "P_QuestTip", 0 );

	/// 채팅 아이템 팁윈도우
	mpChatItemTip = new cChatTipWindow;
	mpChatItemTip->CreateByPropertyName( "P_ChatItemTip", 0 );
	UIMAN->SetChatItemTipWindow( mpChatItemTip );

	/// 하단 버튼 윈도우
	mpMenuButtonWindow = new cMenuButtonWindow;
	mpMenuButtonWindow->CreateByPropertyName( "P_MenuButtonWindow", 0 );

	/// 하단메뉴 윈도우
	mpMenuHero = new cMenuHeroWindow;
	mpMenuHero->CreateByPropertyName( "P_MenuHeroWindow", 0 );
	mpMenuButtonWindow->AddMenu( eMENU_HERO, mpMenuHero );

	mpMenuCommu = new cMenuCommuityWindow;
	mpMenuCommu->CreateByPropertyName( "P_MenuCommunityWindow", 0 );
	mpMenuButtonWindow->AddMenu( eMENU_COMMU, mpMenuCommu );

	mpMenuOption = new cMenuOptionWindow;
	mpMenuOption->CreateByPropertyName( "P_MenuOptionWindow", 0 );
	mpMenuButtonWindow->AddMenu( eMENU_OPTION, mpMenuOption );

	mpMenuLife = new cMenuLifeWindow;
	mpMenuLife->CreateByPropertyName( "P_MenuLifeWindow", 0 );
	mpMenuButtonWindow->AddMenu( eMENU_LIFE, mpMenuLife );

	/// 시스템 윈도우 
	mpOptionWindow = new cOptionWindow;
	mpOptionWindow->CreateByPropertyName( "P_Option_Window", 0 );

	/// 친구 윈도우 
	mpFriendWindow = new cFriendWindow;
	mpFriendWindow->CreateByPropertyName( "P_FriendWindow", 0 );

	mpTarotWindow = new cTarotWindow;
	mpTarotWindow->CreateByPropertyName( "P_TarotWindow", 0 );

	/// 타롯 점집 관리 ( 개설자 )
	mpTarotControlWindow = new cTarotControlWindow;
	mpTarotControlWindow->CreateByPropertyName( "P_TarotControlWindow", 0 );

	/// 노점상 개설
	mpUserSellWindow = new cUserSellWindow;
	mpUserSellWindow->CreateByPropertyName( "P_UserSellWindow", 0 );

	/// 게임 종료 윈도우
	mpExitWindow = new cExitWindow;
	mpExitWindow->CreateByPropertyName( "P_ExitWindow", 0 );

	mpTutorialExitWindow = new cTutorialExitWindow;
	mpTutorialExitWindow->CreateByPropertyName( "P_TutorialExitWindow", 0 );

	mpExitMsgWindow = new cExitMsgWindow;
	mpExitMsgWindow->CreateByPropertyName( "P_ExitMsg_Window", 0 );

	/// 아이템 습득 윈도우
	mpLootingWindow	= new cLootingWindow;
	mpLootingWindow->CreateByPropertyName( "P_LootingWindow", 0 );

	/// 부활 윈도우
	mpRevivalWindow = new cRevivalWindow;
	mpRevivalWindow->CreateByPropertyName( "P_RevivalWindow", 0 );

	/// 파티 관리 윈도우
	mpPartyWindow = new cPartyWindow;
	mpPartyWindow->CreateByPropertyName( "P_PartyWindow", 0 );

	/// 파티 상태 윈도우
	mpPartyMenu = new cPartyMenu;
	mpPartyMenu->CreateByPropertyName( "P_PartyMenu", 0 );
	UIMAN->AddEventOtherClick( mpPartyMenu );

	/// 파티 분배타입 변경 윈도우
	mpPartyDivideMenu = new cPartyDivideMenu;
	mpPartyDivideMenu->CreateByPropertyName( "P_PartyDivdeMenu", 0 );

	/// 파티원 상태 윈도우
	cUINodeProperty* pPartyProp = p->GetNodeProperty( "P_PartyStateWindow" );
	assert( pPartyProp );
	if( pPartyProp )
	{
		for( int i = 0; i < 5; ++i )
		{
			mpPartyStateWindow[i] = new cPartyStateWindow;
			mpPartyStateWindow[i]->CreateByPropertyId( pPartyProp, 0, eUIID_GAME_PARTYSTATE_WINDOW1 + i );
			mpPartyStateWindow[i]->InitPosition( i );
		}
	}

	/// 파티원 상태 Mini 윈도우
	cUINodeProperty* pPartyMiniProp = p->GetNodeProperty( "P_PartyStateMiniWindow" );
	assert( pPartyMiniProp );
	if( pPartyMiniProp )
	{
		for( int i = 0; i < 5; ++i )
		{
			mpPartyStateMiniWindow[i] = new cPartyStateMiniWindow;
			mpPartyStateMiniWindow[i]->CreateByPropertyId( pPartyMiniProp, 0, eUIID_GAME_PARTYSTATE_MINI_WINDOW1 + i );
			mpPartyStateMiniWindow[i]->InitPosition( i );
		}
	}

	/// 퀘스트 윈도우
	mpQuestWindow = new cQuestWindow;
	mpQuestWindow->CreateByPropertyName( "P_QuestWindow", 0 );

	/// 아이템 분해 윈도우
	mpNpcDisjointWindow = new cNpcDisjointWindow;
	mpNpcDisjointWindow->CreateByPropertyName( "P_NpcDisjointWindow", 0 );

	/// 아이템 강화 윈도우
	mpEnhancedWindow = new cEnhancedWindow;
	mpEnhancedWindow->CreateByPropertyName( "P_EnhancedWindow", 0 );

	/// 퀘스트 간략정보 출력 윈도우
	mpQuestSimpleWindow = new cQuestSimpleWindow;
	mpQuestSimpleWindow->CreateByPropertyName( "P_QuestSimpleWindow", 0 );

	mpItemMsgWindow = new cItemMsgWindow;
	mpItemMsgWindow->CreateByPropertyName( "P_ItemMsg_Window", 0 );

	/// 카드 장착 윈도우
	mpPutCardWindow = new cPutCardWindow;
	mpPutCardWindow->CreateByPropertyName( "P_PutCardWindow", 0 );

	/// 결투 윈도우
	mpDuelMsgWindow = new cDuelMsgWindow;
	mpDuelMsgWindow->CreateByPropertyName( "P_DuelMsg_Window", 0 );

	mpDuelWindow = new cDuelWindow;
	mpDuelWindow->CreateByPropertyName( "P_DuelWindow", 0 );

	/// 카드 컬렉션 북
	mpCardBookWindow = new cCardBookWindow;
	mpCardBookWindow->CreateByPropertyName( "P_CardBook_Window", 0 );

	/// 사회 윈도우
	mpSocialWindow = new cSocialWindow;
	mpSocialWindow->CreateByPropertyName( "P_SocialWindow", 0 );

	/// 데스매치 점수 윈도우
	mpDMScoreWindow = new cDMScoreWindow;
	mpDMScoreWindow->CreateByPropertyName( "P_PVP_DMScoreWindow", 0 );

	/// Ranking window
	mpDMRankingListWindow = new cDMRankingListWindow;
	mpDMRankingListWindow->CreateByPropertyName( "P_PVP_DMMiddleResultWindow", 0 );

	/// 데스매치 결과 윈도우
	mpDMResultWindow = new cDMResultWindow;
	mpDMResultWindow->CreateByPropertyName( "P_PVP_DMResultWindow", 0 );

	/// 데스매치 입장 윈도우
	mpDMMsgWindow = new cDMMsgWindow;
	mpDMMsgWindow->CreateByPropertyName( "P_PVP_DMMsgWindow", 0 );

	/// 호칭관리 윈도우
	mpTitleWindow = new cTitleWindow;
	mpTitleWindow->CreateByPropertyName( "P_HeroTitleWindow", 0 );

	/// 길드 관리 윈도우
	mpGuildWindow =	new cGuildWindow;
	mpGuildWindow->CreateByPropertyName( "P_GuildWindow", 0 );

	/// 길드 직위 버튼 윈도우
	mpGuildButtonWindow = new cGuildButtonWindow;
	mpGuildButtonWindow->CreateByPropertyName( "P_GuildPosition_Window", 0 );

	/// 길드 마크 등록 윈도우
	mpGuildMarkWindow = new cGuildMarkWindow;
	mpGuildMarkWindow->CreateByPropertyName( "P_GuildMark_Window", 0 );

	/// Popup Menu window
	mpRButtonMenuWindow = new cRButtonMenuWindow;
	mpRButtonMenuWindow->CreateByPropertyName( "P_RButtonMenu", 0 );

	/// 오늘의 한마디 윈도우
	mpTodayWordWindow = new cTodayWordWindow;
	mpTodayWordWindow->CreateByPropertyName( "P_TodayWordWindow", 0 );

	/// 아이템 조합 윈도우
	mpItemMixWindow = new cItemMixWindow;
	mpItemMixWindow->CreateByPropertyName( "P_ItemMixWindow", 0 );

	/// NPC 판매대행 윈도우
	mpNpcDealWindow = new cNpcDealWindow;
	mpNpcDealWindow->CreateByPropertyName( "P_NpcDeal_Window", 0 );

	/// 우편 윈도우
	mpMailWindow = new cMailWindow;
	mpMailWindow->CreateByPropertyName( "P_Mail_Window", 0 );

	/// 파티 연합 윈도우
	mpUnionWindow = new cPartyUnionWindow;
	mpUnionWindow->CreateByPropertyName( "P_PartyUnion_Window", 0 );

	mpUnionSmallWindow = new cPartyUnionSmallWindow;
	mpUnionSmallWindow->CreateByPropertyName( "P_PartyUnionSmall_Window", 0 );

	mpThemeEnterWindow = new cThemeEnterWindow;
	mpThemeEnterWindow->CreateByPropertyName( "P_ThemeEnterWindow", 0 );

	/// 테마 던젼 상태 윈도우
	mpThemeStateWindow = new cThemeStateWindow;
	mpThemeStateWindow->CreateByPropertyName( "P_ThemeStateWindow", 0 );

	/// 스킬 찍어보기 윈도우
	mpSkillPreviewWindow = new cSkillPreviewWindow;
	mpSkillPreviewWindow->CreateByPropertyName( "P_SkillPreviewWindow", 0 );

	/// 단축키 설정 윈도우
	mpShortcutWindow = new cShortcutWindow;
	mpShortcutWindow->CreateByPropertyName( "P_ShortcutWindow", 0 );

	/// 게시판 윈도우
	mpBoardWindow = new cBoardWindow;
	mpBoardWindow->CreateByPropertyName( "P_Board_Window", 0 );

	/// 주사위 굴리기 윈도우
	mpDiceWindow = new cDiceWindow;
	mpDiceWindow->CreateByPropertyName( "P_DiceWindow", 0 );

	/// 튜토리얼 북 윈도우
	mpNpcTutorialWindow = new cNpcTutorialWindow;
	mpNpcTutorialWindow->CreateByPropertyName( "P_TutorialBook_Window", 0 );

	/// 찾아가기/따라가기 메시지 표시 윈도우
	mpMoveMsgWindow = new cMoveMsgWindow;
	mpMoveMsgWindow->CreateByPropertyName( "P_MoveMsgWindow", 0 );

	/// 아이템 수령함
	mpReceiptWindow = new cItemReceiptWindow;
	mpReceiptWindow->CreateByPropertyName( "P_Receipt_Window", 0 );

	/// 몰 아이템 수령함 윈도우
	mpItemMallWindow = new cItemMallWindow;
	mpItemMallWindow->CreateByPropertyName( "P_Itemmall_Window", 0 );

	/// 트위터
	mpTwitterWindow = new cTwitterWindow;
	mpTwitterWindow->CreateByPropertyName( "P_TwitterWindow", 0 );

	mpTwitterRegisterWindow = new cTwitterRegisterWindow;
	mpTwitterRegisterWindow->CreateByPropertyName( "P_TwitterLoginWindow", 0 );

	/// 오토 유저 신고 윈도우
	mpCamaelWindow = new cCamaelWindow;
	mpCamaelWindow->CreateByPropertyName( "P_Camael_Window", 0 );

	// 메가폰 윈도우
	mpMegaphoneWindow = new cMegaphoneWindow;
	mpMegaphoneWindow->CreateByPropertyName( "P_MegaPhoneWindow", 0 );

	/// 캐시 아이템- 자유 이동 주문서
	mpCashFreeMoveWindow = new cCashFreeMoveWindow();
	mpCashFreeMoveWindow->CreateByPropertyName( "P_CashFreeMoveWindow", 0 );

	// 테마던전 귀속 초기화 윈도우
	mpThemeResetWindow = new cThemeResetWindow;
	mpThemeResetWindow->CreateByPropertyName( "P_ThemeResetWindow", 0 );

	// 아이템 분배 윈도우
	mpItemDivideWindow = new cItemDivideWindow;
	mpItemDivideWindow->CreateByPropertyName( "P_ItemDivideWindow", 0 );

	// 살펴보기 윈도우
	mpLookWindow = new cLookWindow;
	mpLookWindow->CreateByPropertyName( "P_Look_Window", 0 );

	// 편의 기능 윈도우
//	mpSummonWindow = new cSummonWindow;
//	mpSummonWindow->CreateByPropertyName( "P_NpcFuncSummonsWindow", 0 );

	// 인증번호 윈도우
//	mpCertifyWindow = new cCertifyWindow;
//	mpCertifyWindow->CreateByPropertyName( "P_AttestNumWindow", 0 );	

	///
	if( LoadWindowPos("./Option/user.opt") == false )
	{
		// 기본 처리
		mChatWinOption[0].checkNormal = true;
		mChatWinOption[0].checkParty = true;
		mChatWinOption[0].checkGuild = true;
		mChatWinOption[0].checkShout = true;
		mChatWinOption[0].checkTrade = true;
		mChatWinOption[0].checkWhisper = true;
		mChatWinOption[0].checkSystem = false;
		mChatWinOption[0].checkMegaPhone = true;

		mChatWinOption[1].checkNormal = false;
		mChatWinOption[1].checkParty = false;
		mChatWinOption[1].checkGuild = false;
		mChatWinOption[1].checkShout = false;
		mChatWinOption[1].checkTrade = false;
		mChatWinOption[1].checkWhisper = false;
		mChatWinOption[1].checkSystem = true;
		mChatWinOption[1].checkMegaPhone = false;

		for( unsigned int i=2;i<4; i++ )
		{
			mChatWinOption[i].checkNormal = false;
			mChatWinOption[i].checkParty = false;
			mChatWinOption[i].checkGuild = false;
			mChatWinOption[i].checkShout = false;
			mChatWinOption[i].checkTrade = false;
			mChatWinOption[i].checkWhisper = false;
			mChatWinOption[i].checkSystem = false;
			mChatWinOption[i].checkMegaPhone = false;
		}

		mInvenWinOption.showBag1 = false;
		mInvenWinOption.showBag2 = false;
		mInvenWinOption.showBag3 = false;

		mMiniMapWinOption.showHelper = true;
		mMiniMapWinOption.showQuestView = true;

		mPartyStateOption.showFlag = 0;
		mQuestOption.infoViewFlag = true;

		mMoveLockOption.chatLock = false;
		mMoveLockOption.partyLock = false;

		mSkillWinOption.showStepListWin = true;

		mMiniWinPer = 50.0f;
	}
}

void cGameUIManager::Exit()
{
	for( int i = 0; i < 5; ++i )
	{
		mpPartyStateMiniWindow[i]->Clear();
		mpPartyStateMiniWindow[i] = 0;

		mpPartyStateWindow[i]->Clear();
		mpPartyStateWindow[i] = 0;
	}

	for( int i = 0; i < 3; ++i )
	{
		mpBagWindow[i] = 0;
		mpWareHouseBagWindow[i] = 0;
	}

	mpLookWindow = 0;
	mpThemeResetWindow = 0;
	mpCashFreeMoveWindow = 0;
	mpMegaphoneWindow = 0;
	mpTwitterRegisterWindow = 0;
	mpTwitterWindow = 0;
	mpItemMallWindow = 0;
	mpNpcTutorialWindow = 0;
	mpReceiptWindow = 0;
	mpDiceWindow = 0;
	mpBoardWindow = 0;
	mpSkillPreviewWindow = 0;
	mpUnionWindow = 0;
	mpUnionSmallWindow = 0;
	mpMailWindow = 0;
	mpNpcDealWindow = 0;
	mpItemMixWindow = 0;
	mpTodayWordWindow = 0;
	mpRButtonMenuWindow = 0;
	mpGuildMarkWindow = 0;
	mpGuildButtonWindow = 0;
	mpGuildWindow = 0;
	mpTitleWindow = 0;
	mpChatButtonWindow = 0;
	mpDMRankingListWindow = 0;
	mpDMMsgWindow = 0;
	mpDMResultWindow = 0;
	mpDMScoreWindow = 0;
	mpSocialWindow = 0;
	mpCardBookWindow = 0;
	mpDuelMsgWindow = 0;
	mpDuelWindow = 0;
	mpPutCardWindow = 0;
	mpQuestSimpleWindow = 0;
	mpEnhancedWindow = 0;
	mpNpcDisjointWindow = 0;
	mpQuestWindow = 0;
	mpMoneyWindow = 0;
	mpPartyDivideMenu = 0;
	mpPartyMenu = 0;
	mpPartyWindow = 0;
	mpRevivalWindow = 0;
	mpLootingWindow = 0;
	mpExitWindow = 0;
	mpTutorialExitWindow = 0;
	mpExitMsgWindow = 0;
	mpUserSellWindow = 0;
	mpTarotControlWindow = 0;
	mpTarotWindow = 0;
	mpFriendWindow = 0;
	mpSmallMiniMapWindow = 0;
	mpMiniMapWindow = 0;
	mpOptionWindow = 0;
	mpMenuHero = 0;
	mpMenuCommu = 0;
	mpMenuOption = 0;
	mpMenuButtonWindow = 0;
	mpSkillWindow = 0;
	mpBuffWindow = 0;
	mpDeBuffWindow = 0;
	mpCashBuffWindow = 0;
	mpInventoryWindow = 0;
	mpNpcRewardWindow = 0;
	mpNpcWindow = 0;
	mpNpcBackWindow = 0;
	mpNpcStoreWindow = 0;
	mpHeroStateWindow = 0;
	mpHeroInfoWindow = 0;
	mpTargetWindow = 0;
	mpChatWindow = 0;
	mpCastingBar = 0;
	mpThemeEnterWindow = 0;
	mpShortcutWindow = 0;
	mpMoveMsgWindow = 0;
	mpCamaelWindow = 0;
//	mpCertifyWindow = 0;
	mpSummonWindow = 0;

	if( mpChatManager )
	{
		SAFE_DELETE( mpChatManager );
	}
}

void cGameUIManager::Open()
{
	UIMAN->SetCurrentStage( eStage_Game );

	///
	UIMAN->Open();

	///
	ApplyWindowPos();

	/// 기본 퀵슬롯은 항상 둘중 하나라도 꼭 보인다.
	if( QUICKMAN->IsRowQuick( QUICKTYPE_COMMON ) == true )
	{
		mpRowQuickSlot[QUICKTYPE_COMMON]->Show( false );
		mpColumnQuickSlot[QUICKTYPE_COMMON]->Hide();
	}
	else
	{
		mpRowQuickSlot[QUICKTYPE_COMMON]->Hide();
		mpColumnQuickSlot[QUICKTYPE_COMMON]->Show( false );
	}


	if( QUICKMAN->IsRowQuickFS() == true )
	{
		mpFSRowQuickSlot->Show( false );
		mpFSColumnQuickSlot->Hide();
	}
	else
	{
		mpFSRowQuickSlot->Hide();
		mpFSColumnQuickSlot->Show( false );
	}

	///
	if( mQuestCharacterIdx != HERO->GetObjectID() )
	{
		char name[MAX_NAME_BUFFER_SIZE];	/// 아이디
		ConvertToAscii( HERO->GetName(), name, MAX_NAME_BUFFER_SIZE );

		cString str;
		str.Format( "./Option/%s_%d.opc",  name, HERO->GetObjectID()%1000 );

		LoadWindowOption_Character( str );
	}
}

void cGameUIManager::Close()
{
	SaveWindowPos("./Option/user.opt");

	char name[MAX_NAME_BUFFER_SIZE];	/// 아이디
	ConvertToAscii( HERO->GetName(), name, MAX_NAME_BUFFER_SIZE );

	cString str;
	str.Format( "./Option/%s_%d.opc",  name, HERO->GetObjectID()%1000 );

	SaveWindowOption_Character( str );

	UIMAN->SetCurrentStage( eStage_Game );

	UIMAN->Close();

#ifdef _GMTOOL
	//	CHEATMAN->Clear();
#endif
}

void cGameUIManager::Process( unsigned long deltaTime, unsigned long accumTime )
{

}

void cGameUIManager::ReqFinish( bool exitEvent )
{
	/// 자동사냥 신고 창이 뜬 도중이라면 무시
	if( mpCamaelWindow && mpCamaelWindow->IsVisible() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11818 ) );
		return;
	}

	if( HERO->GetState() == ePLAYER_STATE_SITDOWN )
	{
		HERO->SetState( eOBJECT_STATE_IDLE );

		/// Network Msg
		MSG_REQ_PLAYER_STATE_SITDWON Msg;
		Msg.Category = NM_PLAYER;
		Msg.Protocol = NM_PLAYER_STATE_SITDOWN_REQ;
		Msg.mSitDown = false;

		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
	else
	{
		if( HERO->GetState() != eOBJECT_STATE_IDLE )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 58 ) );
			return;
		}
	}

	/// 게임 종료 5초
	MSGROOT Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_GAMEFINISH_REQ;
	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	if( mpExitMsgWindow )
	{
		mpExitMsgWindow->ShowExitMsg( exitEvent );
		mpExitMsgWindow->NetLock();
	}

	if( mpExitWindow )
		mpExitWindow->Hide(false);
}


void cGameUIManager::SendDisConnect()
{
	/// 게임 종료
	MSGROOT Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_DISCONNECT_REQ;
	if( NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) ) == false )
	{
		/// 프로그램 종료
		::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
		::PostQuitMessage(0);
	}

	/// 관련 ui 락
	if( mpExitWindow )
		mpExitWindow->NetLock();
}

void cGameUIManager::SendGameOut()
{
	/// 캐릭터 창으로 이동
	MSGROOT Msg;
	Msg.Category = NM_PLAYER;
	Msg.Protocol = NM_PLAYER_GAMEOUT_REQ;
	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	/// 관련 ui 락
	if( mpExitWindow )
		mpExitWindow->NetLock();
}

void cGameUIManager::NetLockParty( int lockTry )
{
	if( mpPartyMenu )
		mpPartyMenu->NetLock( lockTry );

	if( mpFriendWindow )
		mpFriendWindow->NetLock( lockTry );

	if( mpPartyDivideMenu )
		mpPartyDivideMenu->NetLock( lockTry );
}

void cGameUIManager::NetUnLockParty( int lockTry )
{
	if( mpPartyMenu )
		mpPartyMenu->NetUnLock( lockTry );

	if( mpFriendWindow )
		mpFriendWindow->NetUnLock( lockTry );

	if( mpPartyDivideMenu )
		mpPartyDivideMenu->NetUnLock( lockTry );
}

void cGameUIManager::OnFocusChatWindow( unsigned long key, LPCTSTR addstring )
{
	assert(mpChatWindow);
	if( mpChatWindow )
	{
		cStringT str;
		switch( key )
		{
		case KEY_RETURN:
			break;
		case KEY_SLASH:			/// "/" 자동입력
			str.Format(_T("/"));
			break;
		case KEY_APOSTROPHE:	/// 귓속말 자동입력
			{
				if( ::_tcslen(addstring) == 0 )
					str.Format( UIMAN->GetUIText(10001) );
				else
					str.Format( _T("%s%s "), UIMAN->GetUIText(10001), addstring );
			}
			break;
		case KEY_SEMICOLON:		/// 파티말 자동입력
			str.Format( UIMAN->GetUIText(10013) );
			break;
		case KEY_PERIOD:		/// 외치기 자동입력
			str.Format( UIMAN->GetUIText(10023) );
			break;
		case KEY_COMMA:			/// 길드말 자동입력
			str.Format( UIMAN->GetUIText(10033) );
			break;
		}

		if( key != KEY_RETURN )
			mpChatWindow->SetChatText( (LPCTSTR)str.Cstr() );

		///
		mpChatWindow->SetFocusToEdit();
	}
}

/// 상태에 따른 처리 - 죽을때는 패킷을 날리지 않는다
void cGameUIManager::HideUIByDie()
{
	UIMAN->ShowGameUI();

	if( mpTarotControlWindow )
		mpTarotControlWindow->Hide();

	if( mpTarotWindow )
		mpTarotWindow->Hide();

	if( mpUserSellWindow )
		mpUserSellWindow->Hide();

	if( mpTradeWindow )
		mpTradeWindow->Hide();

	if( mpCardBookWindow )
		mpCardBookWindow->Hide();

	if( mpEnhancedWindow )
		mpEnhancedWindow->Hide();

	if( mpItemMixWindow )
		mpItemMixWindow->Hide();

	if( mpNpcWindow )
		mpNpcWindow->Hide();

	if( mpMailWindow )
		mpMailWindow->Hide();

	if( mpBoardWindow )
		mpBoardWindow->Hide();

	if( mpNpcTutorialWindow )
		mpNpcTutorialWindow->Hide();

	if( mpMoveMsgWindow )
		mpMoveMsgWindow->Hide();

	if( mpMegaphoneWindow )
		mpMegaphoneWindow->Hide();

	if( mpCashFreeMoveWindow )
		mpCashFreeMoveWindow->Hide();

	if( mpThemeResetWindow )
		mpThemeResetWindow->Hide();

	if( mpWareHouseWindow )
		mpWareHouseWindow->Hide();

	if( mpNpcDealWindow )
		mpNpcDealWindow->Hide();

	if( mpSummonWindow )
		mpSummonWindow->Hide();

	//
	HERO->CloseNpcTalk();

	TAROTMAN->ChangePage( ePAGE_NONE );
	ITEMMAN->ClearUserSellTarget();

	HERO->DeActiveTarotBubble();
	HERO->DeActiveUserSellBubble();

	UIMAN->DestroyAllMsgBox();

	/// 부활창 띄우기 
	if( PVPMAN->IsEnableBattle() == true )
	{
		/// show 10' wait msgbox
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_PVP_REVIVAL, eSKIN_NONE,
			GAMERESOURCEMAN->GetGameText( 833 ),
			GAMERESOURCEMAN->GetGameText( 834 ),
			0, 0, PVP_DM_RESURRECTION_TIME, true, true );
	}
	else if( TUTORIALMAN->IsTutorialMode() == true )
	{
		cTutorialMode* mode = TUTORIALMAN->GetCurrentMode();
		if( mode )
		{
			// 튜토리얼중이면
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_TUTORIAL_REVIVAL, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( mode->mDieTextIndex ),
				GAMERESOURCEMAN->GetGameText( 834 ) );
		}
		else
			assert(0);
	}
	else
		ShowRevivalWindow();
}

/// 상태에 따른 처리 - 강제로 밀릴때는 메세지를 날리지 않는다.
void cGameUIManager::HideUIByPushPull()
{
	UIMAN->ShowGameUI();

	if( mpTarotControlWindow )
		mpTarotControlWindow->Hide();

	if( mpTarotWindow )
		mpTarotWindow->Hide();

	if( mpUserSellWindow )
		mpUserSellWindow->Hide();

	if( mpTradeWindow )
		mpTradeWindow->Hide();

	if( mpCardBookWindow )
		mpCardBookWindow->Hide();

	if( mpEnhancedWindow )
		mpEnhancedWindow->Hide();

	if( mpItemMixWindow )
		mpItemMixWindow->Hide();

	if( mpNpcWindow )
		mpNpcWindow->Hide();

	if( mpMailWindow )
		mpMailWindow->Hide();

	if( mpBoardWindow )
		mpBoardWindow->Hide();

	if( mpNpcTutorialWindow )
		mpNpcTutorialWindow->Hide();

	if( mpMoveMsgWindow )
		mpMoveMsgWindow->Hide();

	if( mpMegaphoneWindow )
		mpMegaphoneWindow->Hide();

	if( mpCashFreeMoveWindow )
		mpCashFreeMoveWindow->Hide();

	//
	HERO->CloseNpcTalk();

	TAROTMAN->ChangePage( ePAGE_NONE );
	ITEMMAN->ClearUserSellTarget();

	HERO->DeActiveTarotBubble();
	HERO->DeActiveUserSellBubble();
}

void cGameUIManager::HideItemSetWindow( cUIWindow* exceptWin )
{
	assert( mpItemMixWindow );
	assert( mpPutCardWindow );
	assert( mpEnhancedWindow );

	if( mpItemMixWindow != exceptWin )
		mpItemMixWindow->Hide();

	if( mpPutCardWindow != exceptWin )
		mpPutCardWindow->Hide();

	if( mpEnhancedWindow != exceptWin )
		mpEnhancedWindow->Hide();
}

void cGameUIManager::ToggleOptionWindow()
{
	assert(mpOptionWindow);

	if( mpOptionWindow )
		mpOptionWindow->Toggle();
}

void cGameUIManager::ToggleHeroInfoWindow()
{
	assert(mpHeroInfoWindow);

	if( mpHeroInfoWindow )
	{
		mpHeroInfoWindow->Toggle();

		/// hero정보를 재계산한다.
		if( mpHeroInfoWindow->IsVisible() )
			UpdateHeroInfo();
	}
}

void cGameUIManager::ToggleInventoryWindow()
{
	assert( mpInventoryWindow );

	if( mpInventoryWindow )
	{
		mpInventoryWindow->Toggle();
		if( mpInventoryWindow->IsVisible() )
		{
			mpItemCountWindow->Hide();
		}
	}
}

void cGameUIManager::ToggleBagWindow( unsigned int i )
{
	assert( mpBagWindow[i] );

	if( mpBagWindow[i] )
		mpBagWindow[i]->Toggle();
}

void cGameUIManager::ToggleWareHouseBagWindow( unsigned int idx )
{
	assert( mpWareHouseBagWindow[idx] );

	if( mpWareHouseBagWindow[idx] )
		mpWareHouseBagWindow[idx]->Toggle();
}


void cGameUIManager::ToggleSmallMiniMapWindow()
{
	if( mDuelMode )
		return;

	assert(mpSmallMiniMapWindow);

	if( mpSmallMiniMapWindow )
		mpSmallMiniMapWindow->Toggle();
}

void cGameUIManager::ToggleMiniMapWindow()
{
	assert(mpMiniMapWindow);

	if( mpMiniMapWindow )
		mpMiniMapWindow->Toggle();
}

///
void cGameUIManager::ToggleFriendWindow()
{
	assert( mpFriendWindow );

	if( mpFriendWindow )
		mpFriendWindow->Toggle();
}

void cGameUIManager::ToggleSkillWindow()
{
	assert( mpSkillWindow );

	if( mpSkillWindow )
		mpSkillWindow->Toggle();
}

void cGameUIManager::ToggleQuestWindow()
{
	assert( mpQuestWindow );

	if( mpQuestWindow )
		mpQuestWindow->Toggle();
}

void cGameUIManager::TogglePartyMenu()
{
	assert( mpPartyMenu );

	if( mpPartyMenu )
		mpPartyMenu->Toggle();
}

void cGameUIManager::TogglePartyDivideMenu()
{
	assert( mpPartyDivideMenu );

	if( mpPartyDivideMenu )
		mpPartyDivideMenu->Toggle();
}

void cGameUIManager::TogglePutCardWindow()
{
	if( mpPutCardWindow )
		mpPutCardWindow->Toggle();
}

void cGameUIManager::ToggleCardBookWindow()
{
	if( mpCardBookWindow )
	{
		// show 할 차례일경우
		if( mpCardBookWindow->IsVisible() == false )
		{
			if( TAROTMAN->IsUsingTarot() == true )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 329 ) );
				return;
			}
		}

		mpCardBookWindow->Toggle();
	}
}

void cGameUIManager::ToggleSocialWindow()
{
	if( mpSocialWindow )
		mpSocialWindow->Toggle();
}

void cGameUIManager::ToggleGuildWindow()
{
	if( mpGuildWindow )
		mpGuildWindow->Toggle();
}

void cGameUIManager::ToggleThemeStateWindow()
{
	if( mpThemeStateWindow )
		mpThemeStateWindow->Toggle();
}

void cGameUIManager::ToggleShortcutWindow()
{
	if( mpShortcutWindow )
		mpShortcutWindow->Toggle();
}

void cGameUIManager::ToggleTitleWindow()
{
	if( mpTitleWindow )
		mpTitleWindow->Toggle();
}

void cGameUIManager::ToggleItemMixWindow()
{
	if( mpItemMixWindow )
		mpItemMixWindow->Toggle();
}

void cGameUIManager::ToggleItemReceiptWindow()
{
	if( mpReceiptWindow )
		mpReceiptWindow->Toggle();
}

void cGameUIManager::ToggleTwitterWindow()
{
	if( mpTwitterWindow )
		mpTwitterWindow->Toggle();
}

void cGameUIManager::ToggleCamaelWindow()
{
	if( mpCamaelWindow )
		mpCamaelWindow->Toggle();
}

void cGameUIManager::ClearPartyWindow()
{
	assert( mpPartyWindow );

	if( mpPartyWindow )
	{
		mpPartyWindow->Hide();
		mpPartyWindow->Clear();
	}
}

void cGameUIManager::ToggleGameUI()
{
	/// 게임 UI 전체 ON / OFF
	cUIRootNode* pRoot = UIMAN->GetRootNode();
	if( pRoot )
	{
		if( pRoot->IsVisible() )
		{
			UIMAN->HideGameUI();
		}
		else
		{
			UIMAN->ShowGameUI();
		}
	}
}

void cGameUIManager::ShowNpcBackWindow()
{
	if( mpNpcBackWindow )
		mpNpcBackWindow->Show();

	UIMAN->AddCloseByNpcHide( mpNpcBackWindow );
}

void cGameUIManager::HideNpcBackWindow()
{
	if( mpNpcBackWindow )
		mpNpcBackWindow->Hide();
}


void cGameUIManager::ShowSkillWindow( bool backup )
{
	assert( mpSkillWindow );

	if( mpSkillWindow )
	{
		if( backup == true )
			mpSkillWindow->BackupWindowPos();

		mpSkillWindow->Reset();
		mpSkillWindow->Show();
	}
}

void cGameUIManager::ShowNpcDisjointWindow()
{
	assert( mpNpcDisjointWindow );

	if( mpNpcDisjointWindow )
		mpNpcDisjointWindow->Show();
}

void cGameUIManager::ShowCardBookWindow()
{
	assert( mpCardBookWindow );

	if( mpCardBookWindow )
		mpCardBookWindow->Show();
}

void cGameUIManager::ShowHeroInfoWindow()
{
	assert( mpHeroInfoWindow );

	if( mpHeroInfoWindow )
		mpHeroInfoWindow->Show();
}

void cGameUIManager::ShowInventory()
{
	if( mpInventoryWindow )
		mpInventoryWindow->Show();
	else
		assert(0);
}

void cGameUIManager::ShowWarehouse()
{
	if( mpWareHouseWindow )
		mpWareHouseWindow->Show();
	else
		assert(0);
}

void cGameUIManager::UpdateInvenWithUI( cUIWindow* win, bool backup )
{
	if( win && mpInventoryWindow && mpBagWindow[0] )
	{
		// inven backup
		if( backup == true )
		{
			mpInventoryWindow->BackupWindowPos();
		}

		// reset
		win->Reset();
		mpInventoryWindow->Reset();

		// bag width
		int bagWidth = 0;
		if( HERO->GetBagEndSlot() >= INVENTORY_BAG1_END && HERO->GetBagEndSlot() <= INVENTORY_BAG3_END )
		{
			bagWidth = mpBagWindow[0]->GetRelativeRect().GetWidth();
		}

		int winWidth = win->GetAbsoluteRect().GetWidth();
		int invenWidth = mpInventoryWindow->GetAbsoluteRect().GetWidth();
		int winTop = win->GetRelativeRect().mTop;
		int invenTop = mpInventoryWindow->GetRelativeRect().mTop;

		int left = (int)(( (int)RENDERSYS->GetScreenWidth() - (winWidth + invenWidth + bagWidth)) * 0.5f);
		if( left < 0 )
		{
			left = 0;
		}

		// 위치 갱신
		win->SetRelativePos( cUIPos( left, winTop ) );
		mpInventoryWindow->SetRelativePos( cUIPos( left + winWidth, invenTop ) );

		// inventory BagWindow
		for( unsigned int i=0; i<3; i++ )
		{
			if( mpBagWindow[i] )
			{
				mpBagWindow[i]->Reset();

				int invenLeft = mpInventoryWindow->GetRelativeRect().mLeft;
				int top = mpBagWindow[i]->GetRelativeRect().mTop;

				mpBagWindow[i]->SetRelativePos( cUIPos( invenLeft + invenWidth, top ) );
			}

			mpInventoryWindow->SetBagButtonPressed( i, true );
		}
	}
}

// 창고와의 위치 조절
void cGameUIManager::UpdateInvenWithWare()
{
	if( mpWareHouseWindow && mpInventoryWindow && mpBagWindow[0] && mpWareHouseBagWindow[0] )
	{
		// inven backup
		mpInventoryWindow->BackupWindowPos();

		// reset
		mpWareHouseWindow->Reset();
		mpInventoryWindow->Reset();

		// bag width
		int bagWidth = 0;
		if( HERO->GetBagEndSlot() >= INVENTORY_BAG1_END && HERO->GetBagEndSlot() <= INVENTORY_BAG3_END )
		{
			bagWidth = mpBagWindow[0]->GetRelativeRect().GetWidth();
		}

		int warebagWidth = 0;
		if( HERO->GetWareHouseEndSlot() >= INVENTORY_WAREHOUSE1_END && HERO->GetWareHouseEndSlot() <= INVENTORY_WAREHOUSE3_END )
		{
			warebagWidth = mpWareHouseBagWindow[0]->GetRelativeRect().GetWidth();
		}

		int wareWidth = mpWareHouseWindow->GetAbsoluteRect().GetWidth();
		int invenWidth = mpInventoryWindow->GetAbsoluteRect().GetWidth();
		int wareTop = mpWareHouseWindow->GetRelativeRect().mTop;
		int invenTop = mpInventoryWindow->GetRelativeRect().mTop;

		int left = (int)(( (int)RENDERSYS->GetScreenWidth() - (wareWidth + invenWidth + bagWidth + warebagWidth)) * 0.5f);
		if( left < 0 )
		{
			left = 0;
		}

		// 위치 갱신
		mpWareHouseWindow->SetRelativePos( cUIPos( left + warebagWidth, wareTop ) );
		mpInventoryWindow->SetRelativePos( cUIPos( left + warebagWidth + wareWidth, invenTop ) );

		// inventory BagWindow
		for( unsigned int i=0; i<3; i++ )
		{
			if( mpBagWindow[i] )
			{
				mpBagWindow[i]->Reset();

				int invenLeft = mpInventoryWindow->GetRelativeRect().mLeft;
				int top = mpBagWindow[i]->GetRelativeRect().mTop;

				mpBagWindow[i]->SetRelativePos( cUIPos( invenLeft + invenWidth, top ) );
			}

			mpInventoryWindow->SetBagButtonPressed( i, true );
		}

		// ware bagwindow
		for( unsigned int i=0; i<3; i++ )
		{
			if( mpWareHouseBagWindow[i] )
			{
				mpWareHouseBagWindow[i]->Reset();

				int top = mpWareHouseBagWindow[i]->GetRelativeRect().mTop;

				mpWareHouseBagWindow[i]->SetRelativePos( cUIPos( left, top ) );
			}

			mpWareHouseWindow->SetBagPressed( i, true );
		}

		mpInventoryWindow->Show();
		mpWareHouseWindow->Show();
	}
}

void cGameUIManager::ShowNpcStoreWindow( unsigned long npcIndex )
{
	if( mpNpcStoreWindow )
		mpNpcStoreWindow->Show( npcIndex );
	else
		assert(0);
}

void cGameUIManager::ShowFriendWindow()
{
	assert( mpFriendWindow );

	if( mpFriendWindow )
		mpFriendWindow->Show();
}

void cGameUIManager::ShowOptionWindow()
{
	assert( mpOptionWindow );

	if( mpOptionWindow )		
		mpOptionWindow->Show();		
}

void cGameUIManager::ShowTarotWindow( unsigned char page )
{
	assert( mpTarotWindow );
	mpTarotWindow->ChangeTarotPage( page );
}

void cGameUIManager::ShowTarotControlWindow()
{
	if( mpTarotWindow )
		mpTarotWindow->ChangeTarotPage( 0 );

	assert(mpTarotControlWindow);
	if( mpTarotControlWindow )
		mpTarotControlWindow->Show();
}

void cGameUIManager::ShowItemCountWindow( eMSGBox_Event eventtype, unsigned long i0, unsigned long i1 )
{
	assert( mpItemCountWindow );

	if( mpItemCountWindow )
		mpItemCountWindow->ShowModal( eventtype, i0, i1 );
}

void cGameUIManager::ShowItemCountWindow( eMSGBox_Event eventtype, unsigned long itemIndex )
{
	assert( mpItemCountWindow );

	if( mpItemCountWindow )
		mpItemCountWindow->ShowModal( eventtype, itemIndex );
}

void cGameUIManager::ShowItemCountWindow( eMSGBox_Event eventtype, unsigned long slotIndex, unsigned long itemIndex, 
										 unsigned long maxValue, unsigned long price )
{
	assert( mpItemCountWindow );

	if( mpItemCountWindow )
		mpItemCountWindow->ShowModal( eventtype, slotIndex, itemIndex, maxValue, price );
}

void cGameUIManager::ShowItemDumpAskWindow( unsigned short slotIndex )
{
	if( mpItemMsgWindow )
		mpItemMsgWindow->ShowModal( eMBEVENT_ITEM_DUMP, slotIndex, 0, 0, 0 );
}

void cGameUIManager::ShowItemBuyAskWindow( unsigned long itemIndex, unsigned int count )
{
	if( mpItemMsgWindow )
		mpItemMsgWindow->ShowModal( eMBEVENT_ITEM_BUY, 0, itemIndex, count, 0 );
}

void cGameUIManager::ShowItemSellAskWindow( unsigned short slotIndex, unsigned int count )
{
	if( mpItemMsgWindow )
		mpItemMsgWindow->ShowModal( eMBEVENT_ITEM_SELL, slotIndex, 0, count, 0 );
}

void cGameUIManager::ShowItemUserSellBuyAskWindow( unsigned short slotIndex, unsigned long itemIndex, 
												  unsigned short count, unsigned long price )
{
	if( mpItemMsgWindow )
		mpItemMsgWindow->ShowModal( eMBEVENT_ITEM_USERSELLBUY, slotIndex, itemIndex, count, price );
}

void cGameUIManager::ShowItemNpcDealBuyAskWindow( unsigned long slotIndex, unsigned long itemIndex, 
												 unsigned short count, unsigned long price )
{
	if( mpItemMsgWindow )
		mpItemMsgWindow->ShowModal( eMBEVENT_ITEM_NPCDEALBUY, slotIndex, itemIndex, count, price );
}

void cGameUIManager::ShowItemPutCard( eMSGBox_Event eventtype, unsigned short slotIndex0, unsigned short putSlot, unsigned long itemIndex )
{
	if( mpItemMsgWindow )
		mpItemMsgWindow->ShowModal( eventtype, slotIndex0, putSlot, itemIndex );
}	

void cGameUIManager::ShowItemCardBook( eMSGBox_Event eventtype, unsigned short slotIndex0, unsigned short slotIndex1, unsigned long itemIndex )
{
	if( mpItemMsgWindow )
		mpItemMsgWindow->ShowModal( eventtype, slotIndex0, slotIndex1, itemIndex );
}

void cGameUIManager::ShowMoneyWindow( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex )
{
	assert( mpMoneyWindow );

	if( mpMoneyWindow )
		mpMoneyWindow->ShowMoney( type, eventtype, slotIndex );
}

void cGameUIManager::ShowMoneyModifyWindow( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex )
{
	assert( mpMoneyWindow );

	if( mpMoneyWindow )
		mpMoneyWindow->ShowMoneyModify( type, eventtype, slotIndex );
}

void cGameUIManager::ShowExitWindow()
{
	assert( mpExitWindow );

	if( mpExitWindow )
		mpExitWindow->Show();
}

void cGameUIManager::ShowTutorialExitWindow()
{
	assert( mpTutorialExitWindow );

	if( mpTutorialExitWindow )
		mpTutorialExitWindow->Show();
}

void cGameUIManager::ShowQuestWindow()
{
	assert( mpQuestWindow );

	if( mpQuestWindow )
	{
		mpQuestWindow->Reset();
		mpQuestWindow->Show();
	}
}

void cGameUIManager::ShowLootingWindow()
{
	assert( mpLootingWindow );

	if( mpLootingWindow )
	{
		mpLootingWindow->Reset();
		mpLootingWindow->Show();
	}
}

void cGameUIManager::ShowRevivalWindow()
{
	assert( mpRevivalWindow );

	if( mpRevivalWindow )
	{
		mpRevivalWindow->Reset();
		mpRevivalWindow->Show();
	}
}

void cGameUIManager::ShowGuildWindow()
{
	assert( mpGuildWindow );

	if( mpGuildWindow )
		mpGuildWindow->Show();
}

void cGameUIManager::ShowNpcTarotWindow( unsigned long* tarotArr, unsigned long price, unsigned long* spreadArr, short size )
{
	assert( mpTarotWindow );

	if( mpTarotWindow )
	{
		mpTarotWindow->SetNpcTarotDeck( tarotArr );
		mpTarotWindow->SetNpcSpread( spreadArr, size );
		mpTarotWindow->SetPrice( price );

		MSGROOT Msg;
		Msg.Category = NM_NPC;
		Msg.Protocol = NM_NPC_TAROT_OPEN_REQ;
		NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
	}
}

void cGameUIManager::ShowTodayWordWindow()
{
	assert( mpTodayWordWindow );

	if( mpTodayWordWindow )
		mpTodayWordWindow->Show();
}

void cGameUIManager::ShowReceiptWindow()
{
	assert( mpReceiptWindow );

	if( mpReceiptWindow )
		mpReceiptWindow->Show();

}

void cGameUIManager::ShowSocialWindow()
{
	assert( mpSocialWindow );

	if( mpSocialWindow )
		mpSocialWindow->Show();
}

void cGameUIManager::ShowEnhanceWindow()
{
	assert( mpEnhancedWindow );

	if( mpEnhancedWindow )
		mpEnhancedWindow->Show();
}

void cGameUIManager::ShowNpcDealWindow()
{
	assert( mpNpcDealWindow );

	if( mpNpcDealWindow )
		mpNpcDealWindow->Show();
}

void cGameUIManager::ShowPartyUnionWindow()
{
	assert( mpUnionWindow );

	if( mpUnionWindow )
		mpUnionWindow->Show();
}

void cGameUIManager::ShowPartyUnionSmallWindow()
{
	assert( mpUnionWindow );

	if( mpUnionSmallWindow )
		mpUnionSmallWindow->Show();
}

void cGameUIManager::ShowThemeStateWindow()
{
	assert( mpThemeStateWindow );

	if( mpThemeStateWindow )
		mpThemeStateWindow->Show();
}

void cGameUIManager::ShowCamaelWindow()
{
	assert( mpCamaelWindow );
	if( mpCamaelWindow )
		mpCamaelWindow->Show();
}

void cGameUIManager::ShowSkillPreviewWindow( unsigned long jobIndex1, unsigned long jobIndex2 )
{
	assert( mpSkillPreviewWindow );

	if( mpSkillPreviewWindow )
		mpSkillPreviewWindow->Show( jobIndex1, jobIndex2 );
}

void cGameUIManager::ShowMoveMsgWindow()
{
	assert( mpMoveMsgWindow );

	if( mpMoveMsgWindow )
		mpMoveMsgWindow->Show();
}

void cGameUIManager::HidePartyDivideMenu()
{
	if( mpPartyDivideMenu )
		mpPartyDivideMenu->Hide();
}

void cGameUIManager::HidePartyMenu()
{
	if( mpPartyMenu )
		mpPartyMenu->Hide();
}

void cGameUIManager::HideTarotControlWindow()
{
	assert( mpTarotControlWindow );

	if( mpTarotControlWindow )
		mpTarotControlWindow->Hide();
}

void cGameUIManager::HideLootingWindow()
{
	if( mpLootingWindow && mpLootingWindow->IsVisible() == true )
		mpLootingWindow->Hide();
}

void cGameUIManager::HideTradeWindow()
{
	if( mpTradeWindow )
		mpTradeWindow->Hide();
}

void cGameUIManager::HideCardBookWindow()
{
	if( mpCardBookWindow )
		mpCardBookWindow->Hide();
}

void cGameUIManager::HideTodayWordWindow()
{
	if( mpTodayWordWindow )
		mpTodayWordWindow->Hide();
}

void cGameUIManager::HideMailWindow()
{
	if( mpMailWindow )
		mpMailWindow->Hide();
}

void cGameUIManager::HidePartyUnionWindow()
{
	if( mpUnionWindow )
		mpUnionWindow->Hide();
}

void cGameUIManager::HidePartyUnionSmallWindow()
{
	if( mpUnionSmallWindow )
		mpUnionSmallWindow->Hide();
}

void cGameUIManager::HideMoveMsgWindow()
{
	if( mpMoveMsgWindow )
		mpMoveMsgWindow->Hide();
}

void cGameUIManager::HideCamaelWindow()
{
	if( mpCamaelWindow )
		mpCamaelWindow->Hide();
}

/// 게임 접속시 HERO 관련 세팅
void cGameUIManager::InitHeroInfo()
{
	assert(mpHeroStateWindow);
	assert(mpHeroInfoWindow);

	if( mpHeroStateWindow )
		mpHeroStateWindow->InitHeroInfo();

	if( mpHeroInfoWindow )
		mpHeroInfoWindow->InitHeroInfo();

	if( mpSocialWindow )
		mpSocialWindow->UpdatePoint();
}

void cGameUIManager::UpdateHeroInfo()
{
	assert( mpHeroStateWindow );
	assert( mpHeroInfoWindow );

	if( mpHeroStateWindow )
		mpHeroStateWindow->UpdateHeroInfo();

	if( mpHeroInfoWindow )
		mpHeroInfoWindow->UpdateHeroInfo();
}

void cGameUIManager::UpdateHeroHPMP()
{
	assert( mpHeroStateWindow );
	assert( mpHeroInfoWindow );

	if( mpHeroStateWindow )
		mpHeroStateWindow->UpdateHeroHPMP();

	if( mpHeroInfoWindow )
		mpHeroInfoWindow->UpdateHeroHPMP();
}

void cGameUIManager::UpdateExpSxp()
{
	assert( mpHeroStateWindow );
	assert( mpHeroInfoWindow );

	if( mpHeroStateWindow )
		mpHeroStateWindow->UpdateExpSxp();

	if( mpHeroInfoWindow )
		mpHeroInfoWindow->UpdateExpSxp();
}

void cGameUIManager::UpdateRecovery()
{
	assert( mpHeroInfoWindow );

	if( mpHeroInfoWindow )
	{
		mpHeroInfoWindow->SetHPrecovery( HERO->GetStatus2()->mRecovHP , HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_RECOVHP ) );
		mpHeroInfoWindow->SetMPrecovery( HERO->GetStatus2()->mRecovMP, HERO->GetExtStatusInfoType( ePLAYER_STATUS_EXT_RECOVMP ) );
	}
}

/// 모든 스킬포인트 업데이트 
void cGameUIManager::UpdateSkillPoint()
{
	assert(mpSkillWindow);

	if( mpSkillWindow )
	{
		mpSkillWindow->UpdateSkillPoint();

		if( mpSkillWindow->IsVisible() )
		{
			mpSkillWindow->UpdateSkillUsePoint();
			mpSkillWindow->ShowSkillUpButton();
		}
	}
}

void cGameUIManager::UpdateMinimap()
{
	if( mpSmallMiniMapWindow )
		mpSmallMiniMapWindow->SetMapData();

	if( mpMiniMapWindow )
		mpMiniMapWindow->SetMapData();
}

cBaseObject* cGameUIManager::GetPartyObject( int i )
{
	if( mpPartyStateWindow[i] && mpPartyStateMiniWindow[i] && mpPartyStateWindow[i]->IsVisible() == true )
	{
		return OBJECTMAN->GetObject( eOBJECTTYPE_PLAYER, mpPartyStateWindow[i]->GetUserIndex() );
	}
	return 0;
}

cColumnQuickSlot* cGameUIManager::GetColumnQuickSlot( int type )
{ 
	if( type < QUICKTYPE_COMMON || type > QUICKTYPE_COMMON_EXTRA2 )
	{
		assert(0);
		return 0;
	}

	return mpColumnQuickSlot[type]; 
}

cRowQuickSlot* cGameUIManager::GetRowQuickSlot( int type )
{ 
	if( type < QUICKTYPE_COMMON || type > QUICKTYPE_COMMON_EXTRA2 )
	{
		assert(0);
		return 0;
	}

	return mpRowQuickSlot[type]; 
}

bool cGameUIManager::IsAccessPartyObject( int partyIdx )
{
	if( partyIdx < 0 || partyIdx > 4 )
		return false;

	if( mpPartyStateWindow[partyIdx] && mpPartyStateMiniWindow[partyIdx] )
		return true;

	return false;
}

/// 전체 습득 눌르기 단축
void cGameUIManager::EventLootingWindow()
{
	if( PVPMAN->IsEnableBattle() == true )
		return;

	/// 전체 습득 요청
	if( mpLootingWindow && mpLootingWindow->IsVisible() == true )
	{
		mpLootingWindow->OnCommand( 0, eUIID_GAME_LOOTING_ALL );
	}
	else
	{
		/// Near DropItem Click
		ITEMMAN->CheckNearestDropItem();
	}
}

void cGameUIManager::SetDuelMode( bool mode )
{
	mDuelMode = mode;

	if( mDuelMode )
	{
		if( mpSmallMiniMapWindow )
			mpSmallMiniMapWindow->Hide(false);
		if( mpHeroStateWindow )
			mpHeroStateWindow->Hide(false);
		if( mpBuffWindow )
			mpBuffWindow->Hide(false);
		if( mpDeBuffWindow )
			mpDeBuffWindow->Hide(false);
		if( mpCashBuffWindow )
			mpCashBuffWindow->Hide( false );

		if( mpTargetWindow )
		{
			mpTargetWindow->SetHoldView( true );
			//mpTargetWindow->SetEnabled( false );
		}
	}
	else
	{
		if( mpSmallMiniMapWindow )
			mpSmallMiniMapWindow->Show(false);
		if( mpHeroStateWindow )
			mpHeroStateWindow->Show(false);
		if( mpBuffWindow )
			mpBuffWindow->Show(false);
		if( mpDeBuffWindow )
			mpDeBuffWindow->Show(false);
		if( mpCashBuffWindow )
			mpCashBuffWindow->Show(false);

		if( mpTargetWindow )
		{
			mpTargetWindow->SetHoldView( false );
			//mpTargetWindow->SetEnabled( true );
		}
	}
}

bool cGameUIManager::LoadWindowOption_Character( const cString& pathName )
{
	mQuestCharacterIdx = HERO->GetObjectID();

	mQuestCheckArr.Clear();
	mShortcutMap.Clear();
	mClosedCategorySet.Clear();
	mClosedSimpleCategorySet.Clear();

	TITLEMAN->ClearQuickList();

	for( int i = 0; i < 5; i++ )
		mTitleOption.titleIdxArr[i] = -1;

	cFileLoader loader;
	if( loader.OpenNoPack( pathName, true ) == false )
		return false;

	sWindowOptCharacter header;
	if( loader.Read( &header, sizeof(sWindowOptCharacter) ) != sizeof(sWindowOptCharacter) )
	{
		return false;
	}

	switch( header.version )
	{
	case 1:
		{
			/// 버전정보 + 체크한 퀘스트 개수 카운터가 해드
			unsigned long  questIdx;
			for( unsigned int i=0; i<header.questCount;i++ )
			{
				if( loader.ReadInt( (int*)&questIdx ) == false )
					return false;
				mQuestCheckArr.PushBack( questIdx );
			}
		}
		break;
	case 2:
		{
			/// 버전정보 + 체크한 퀘스트 개수 + 단축키 개수 가 Head
			unsigned long  questIdx = 0;
			int			   shortcutCount = 0;
			//				int			   shortcutIndex = 0;

			if( loader.ReadInt( &shortcutCount ) == false )
				return false;

			for( unsigned int i=0; i<header.questCount;i++ )
			{
				if( loader.ReadInt( (int*)&questIdx ) == false )
					return false;
				mQuestCheckArr.PushBack( questIdx );
			}

			/// 추가적으로 단축키가 추가되어 단축키 저장 정보를 기본셋팅하도록 변경
			// 				for(  int i=0; i< shortcutCount;i++ )
			// 				{
			// 					if( loader.ReadInt( &shortcutIndex ) == false )
			// 						return false;
			// 					mShortcutArr.PushBack( shortcutIndex );
			// 				}

		}
		break;
	case 3:
		{
			/// 버전정보 + 체크한 퀘스트 개수 + 단축키 개수 가 Head
			unsigned long  questIdx = 0;
			int			   shortcutCount = 0;
			int			   shortcutIndex = 0;
			int			   keyboardIndex = 0;

			if( loader.ReadInt( &shortcutCount ) == false )
				return false;

			for( unsigned int i=0; i<header.questCount;i++ )
			{
				if( loader.ReadInt( (int*)&questIdx ) == false )
					return false;
				mQuestCheckArr.PushBack( questIdx );
			}

			/// 추가적으로 단축키가 추가되어 단축키 저장 정보를 기본셋팅하도록 변경
			for(  int i=0; i< shortcutCount;i++ )
			{
				if( loader.ReadInt( &shortcutIndex ) == false )
					return false;

				if( loader.ReadInt( &keyboardIndex ) == false )
					return false;

				mShortcutMap.Insert( shortcutIndex, keyboardIndex );
			}
		}
		break;
	case 4:
		{
			/// 버전정보 + 체크한 퀘스트 개수 + QuestWindow + QuestSimpleWindow 접혀져있는 퀘스트 맵 개수 + 단축키 개수 가 Head
			unsigned int   questIdx = 0;
			unsigned int   questMapCount = 0;
			unsigned int   questMapIndex = 0;
			unsigned int   simpleQuestMapCount = 0;
			unsigned int   simpleQuestMapIndex = 0;
			int			   shortcutCount = 0;
			int			   shortcutIndex = 0;
			int			   keyboardIndex = 0;

			if( loader.ReadUnsignedInt( &questMapCount ) == false )
				return false;

			if( loader.ReadUnsignedInt( &simpleQuestMapCount ) == false )
				return false;

			if( loader.ReadInt( &shortcutCount ) == false )
				return false;

			for( unsigned int i=0; i<header.questCount;i++ )
			{
				if( loader.ReadUnsignedInt( &questIdx ) == false )
					return false;
				mQuestCheckArr.PushBack( questIdx );
			}

			for( unsigned int cnt=0; cnt < questMapCount; ++cnt )
			{
				if( loader.ReadUnsignedInt( &questMapIndex ) == false )
					return false;
				mClosedCategorySet.Insert( questMapIndex  );
			}

			for( unsigned int cnt=0; cnt < simpleQuestMapCount; ++cnt )
			{
				if( loader.ReadUnsignedInt( &simpleQuestMapIndex ) == false )
					return false;
				mClosedSimpleCategorySet.Insert( simpleQuestMapIndex  );
			}

			/// 추가적으로 단축키가 추가되어 단축키 저장 정보를 기본셋팅하도록 변경
			for(  int i=0; i< shortcutCount;i++ )
			{
				if( loader.ReadInt( &shortcutIndex ) == false )
					return false;

				if( loader.ReadInt( &keyboardIndex ) == false )
					return false;

				mShortcutMap.Insert( shortcutIndex, keyboardIndex );
			}
		}
		break;
	case 5:
		{
			/// 버전정보 + 체크한 퀘스트 개수 + QuestWindow + QuestSimpleWindow 접혀져있는 퀘스트 맵 개수 + 단축키 개수 가 Head
			unsigned int   questIdx = 0;
			unsigned int   questMapCount = 0;
			unsigned int   questMapIndex = 0;
			unsigned int   simpleQuestMapCount = 0;
			unsigned int   simpleQuestMapIndex = 0;
			int			   shortcutCount = 0;
			int			   shortcutIndex = 0;
			int			   keyboardIndex = 0;

			if( loader.ReadUnsignedInt( &questMapCount ) == false )
				return false;

			if( loader.ReadUnsignedInt( &simpleQuestMapCount ) == false )
				return false;

			if( loader.ReadInt( &shortcutCount ) == false )
				return false;

			for( unsigned int i=0; i<header.questCount;i++ )
			{
				if( loader.ReadUnsignedInt( &questIdx ) == false )
					return false;
				mQuestCheckArr.PushBack( questIdx );
			}

			for( unsigned int cnt=0; cnt < questMapCount; ++cnt )
			{
				if( loader.ReadUnsignedInt( &questMapIndex ) == false )
					return false;
				mClosedCategorySet.Insert( questMapIndex  );
			}

			for( unsigned int cnt=0; cnt < simpleQuestMapCount; ++cnt )
			{
				if( loader.ReadUnsignedInt( &simpleQuestMapIndex ) == false )
					return false;
				mClosedSimpleCategorySet.Insert( simpleQuestMapIndex  );
			}

			/// 추가적으로 단축키가 추가되어 단축키 저장 정보를 기본셋팅하도록 변경
			for(  int i=0; i< shortcutCount;i++ )
			{
				if( loader.ReadInt( &shortcutIndex ) == false )
					return false;

				if( loader.ReadInt( &keyboardIndex ) == false )
					return false;

				mShortcutMap.Insert( shortcutIndex, keyboardIndex );
			}

			/// 타이틀 리스트
			loader.Read( &mTitleOption, sizeof(sTitleWinOption) );
			
			TITLEMAN->SetHaveQuickTitle( mTitleOption.titleIdxArr );
		}
		break;
	default:
		{
			assert(0);
		}
		return false;
	}

	return true;
}

void cGameUIManager::SaveWindowOption_Character( const cString& pathName )
{
	UpdateWindowOption_Character();

	///
	if( ::FileExist( "Option" ) == false )
	{
		::CreateDirectory( L"Option", 0 );
	}

	cFileSaver saver;
	if( saver.Open( pathName ) == false )
	{
		assert( 0 && "failed to open file to save UI pos file" );
		return;
	}

	/// ver 1 : 헤더( 버젼 + 선택된 퀘스트 개수 ) 
	/// ver 2 : ver1 + 단축키
	/// ver 3 : ver2 + 단축키 개선
	/// ver 4 : ver3 + 퀘스트 창에서 맵 카테고리가 닫혀 있는 개수(QuestWin/QuestSimpleWin)
	/// ver 5 : ver4 + 호칭창 호칭 리스트
	sWindowOptCharacter header;
	header.version = CHARHEARDER_VER;
	header.questCount =mQuestCheckArr.GetSize();
	if( saver.Write( &header, sizeof(sWindowOptCharacter) ) != sizeof(sWindowOptCharacter) )
	{
		assert( 0 && "failed to write character window option file header" );
		return;
	}

	unsigned int questMapCount = mClosedCategorySet.GetSize();
	if( saver.WriteUnsignedInt( questMapCount ) == false )
	{
		assert( 0 && "failed to write simple quest map count" );
		return;
	}

	unsigned int simpleQuestMapCount = mClosedSimpleCategorySet.GetSize();
	if( saver.WriteUnsignedInt( simpleQuestMapCount ) == false )
	{
		assert( 0 && "failed to write simple quest map count" );
		return;
	}

	unsigned int shortcutCount = mShortcutMap.GetSize();
	if( saver.WriteUnsignedInt( shortcutCount ) == false )
	{
		assert( 0 && "failed to write shortcut index" );
		return;
	}

	for( unsigned int i=0; i<header.questCount; i++)
	{
		if( saver.WriteUnsignedInt( mQuestCheckArr[i] ) == false )
		{
			assert( 0 && "failed to write quest index" );
			return;
		}
	}


	cSaveQuestSet::cConstIterator pos = mClosedCategorySet.Begin();
	cSaveQuestSet::cConstIterator end = mClosedCategorySet.End();
	for( ; pos != end; ++pos )
	{
		if( saver.WriteUnsignedInt( *pos ) == false )
		{
			assert( 0 && "failed to write closed quest category map index" );
			return;
		}
	}

	pos = mClosedSimpleCategorySet.Begin();
	end = mClosedSimpleCategorySet.End();
	for( ; pos != end; ++pos )
	{
		if( saver.WriteUnsignedInt( *pos ) == false )
		{
			assert( 0 && "failed to write closed simple quest category map index" );
			return;
		}
	}

	cShortcutMap::cIterator pos1 = mShortcutMap.Begin();
	cShortcutMap::cIterator end1 = mShortcutMap.End();
	for( ; pos1!= end1; ++pos1 )
	{
		int shortcutIndex = (*pos1).mFirst;
		int keyboardIndex = (*pos1).mSecond;

		if( saver.WriteInt( shortcutIndex ) == false )
		{
			assert( 0 && "failed to write shortcut index" );
			return;
		}
		if( saver.WriteInt( keyboardIndex ) == false )
		{
			assert( 0 && "failed to write keyboardIndex index" );
			return;
		}
	}

	if( saver.Write( &mTitleOption, sizeof(sTitleWinOption) ) == false )
	{
		assert( 0 && "failed to write chat title list option");
		return;
	}	
}

bool cGameUIManager::LoadWindowPos( const cString& pathName )
{
	cFileLoader loader;
	if( loader.OpenNoPack( pathName, true ) == false )
		return false;

	sWindowPosHeader header;
	if( loader.Read( &header, sizeof(sWindowPosHeader) ) != sizeof(sWindowPosHeader) )
	{
		return false;
	}

	switch( header.version )
	{
	case 1:
		break;
	case 2:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - (sizeof(bool)*2) );

				mChatWinOption[i].checkSystem = false;

				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
		}
		break;
	case 3:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );		
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
		}
		break;
	case 4:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );		
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );
		}
		break;
	case 5:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );		
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
		}
		break;
	case 6:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );		
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			};

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
			loader.Read( &mMiniMapWinOption, sizeof(sMiniMapWinOption)-sizeof(mMiniMapWinOption.showQuestView) );
		}
		break;
	case 7:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );		
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
			loader.Read( &mMiniMapWinOption, sizeof(sMiniMapWinOption)-sizeof(mMiniMapWinOption.showQuestView) );
			loader.Read( &mPartyStateOption, sizeof(mPartyStateOption) );
		}
		break;
	case 8:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );				
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
			loader.Read( &mMiniMapWinOption, sizeof(sMiniMapWinOption) );
			loader.Read( &mPartyStateOption, sizeof(mPartyStateOption) );
		}
		break;
	case 9:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );	
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
			loader.Read( &mMiniMapWinOption, sizeof(sMiniMapWinOption) );
			loader.Read( &mPartyStateOption, sizeof(mPartyStateOption) );
			loader.Read( &mQuestOption, sizeof(mQuestOption) );
			loader.Read( &mMoveLockOption, sizeof(sMoveLockOption) );
		}
		break;
	case 10:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );				
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
			loader.Read( &mMiniMapWinOption, sizeof(sMiniMapWinOption) );
			loader.Read( &mPartyStateOption, sizeof(mPartyStateOption) );
			loader.Read( &mQuestOption, sizeof(mQuestOption) );
			loader.Read( &mMoveLockOption, sizeof(sMoveLockOption) );
			loader.Read( &mSkillWinOption, sizeof(sSkillWinOption) );
		}
		break;
	case 11:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
			{
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) - sizeof(bool) );		
				if( i == 0)
					mChatWinOption[i].checkMegaPhone = true;
				else
					mChatWinOption[i].checkMegaPhone = false;
			}

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );
			loader.ReadInt( &mExtraChatBoxMaxRow[0] );
			loader.ReadInt( &mExtraChatBoxMaxRow[1] );
			loader.ReadInt( &mExtraChatBoxMaxRow[2] );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
			loader.Read( &mMiniMapWinOption, sizeof(sMiniMapWinOption) );
			loader.Read( &mPartyStateOption, sizeof(mPartyStateOption) );
			loader.Read( &mQuestOption, sizeof(mQuestOption) );
			loader.Read( &mMoveLockOption, sizeof(sMoveLockOption) );
			loader.Read( &mSkillWinOption, sizeof(sSkillWinOption) );
		}
		break;
	case 12:
		{
			/// 윈도우별 옵션저장
			for( unsigned int i=0; i<4; i++ )
				loader.Read( &(mChatWinOption[i]), sizeof(sChatWinOption) );

			loader.ReadFloat( &mMiniWinPer );
			loader.ReadInt( &mChatBoxMaxRow );
			loader.ReadInt( &mExtraChatBoxMaxRow[0] );
			loader.ReadInt( &mExtraChatBoxMaxRow[1] );
			loader.ReadInt( &mExtraChatBoxMaxRow[2] );

			loader.Read( &mInvenWinOption, sizeof(sInvenWinOption) );
			loader.Read( &mMiniMapWinOption, sizeof(sMiniMapWinOption) );
			loader.Read( &mPartyStateOption, sizeof(mPartyStateOption) );
			loader.Read( &mQuestOption, sizeof(mQuestOption) );
			loader.Read( &mMoveLockOption, sizeof(sMoveLockOption) );
			loader.Read( &mSkillWinOption, sizeof(sSkillWinOption) );
		}
		break;
	default:
		{
			assert(0);
		}
		return false;
	}

	if( header.dataCount == 0 )
		return false;

	for( unsigned int i=0;i< header.dataCount; i++ )
	{
		sWindowPosData* data = new sWindowPosData;
		loader.Read( data, sizeof(sWindowPosData) );

		/// 
		if( mSaveWindowInfo.Insert( data->index, data ) == false )
		{
			assert(0);
		}
	}
	return true;
}

void cGameUIManager::SaveWindowPos( const cString& pathName )
{
	UpdateWindowPos();

	///
	if( ::FileExist( "Option" ) == false )
	{
		::CreateDirectory( L"Option", 0 );
	}

	cFileSaver saver;
	if( saver.Open( pathName ) == false )
	{
		assert( 0 && "failed to open file to save UI pos file" );
		return;
	}

	sWindowPosHeader header;
	header.version = POSHEARDER_VER;
	header.dataCount = mSaveWindowInfo.GetSize();
	if( saver.Write( &header, sizeof(sWindowPosHeader) ) != sizeof(sWindowPosHeader) )
	{
		assert( 0 && "failed to write UI pos file header" );
		return;
	}

	///
	for( unsigned int i=0; i<4; i++ )
	{
		if( saver.Write( &(mChatWinOption[i]), sizeof(sChatWinOption) ) == false )
		{
			assert( 0 && "failed to write chat option" );
			return;
		}
	}

	///
	if( saver.WriteFloat( mMiniWinPer) == false )
	{
		assert( 0 && "failed to write minimap option" );
		return;
	}

	if( saver.WriteInt( mChatBoxMaxRow ) == false )
	{
		assert( 0 && "failed to write chat max row" );
		return;
	}

	for( unsigned int i=0; i<3; i++ )
	{
		if( saver.WriteInt( mExtraChatBoxMaxRow[i] ) == false )
		{
			assert( 0 && "failed to write extra chat max row" );
			return;
		}
	}

	if( saver.Write( &mInvenWinOption, sizeof(sInvenWinOption) ) == false )
	{
		assert( 0 && "failed to write Inven option");
	}

	if( saver.Write( &mMiniMapWinOption, sizeof(sMiniMapWinOption) ) == false )
	{
		assert( 0 && "failed to write minimap option");
	}

	if( saver.Write( &mPartyStateOption, sizeof(sPartyStateOption) ) == false )
	{
		assert( 0 && "failed to write partystate option");
	}

	if( saver.Write( &mQuestOption, sizeof(sQuestOption) ) == false )
	{
		assert( 0 && "failed to write quest option");
	}

	if( saver.Write( &mMoveLockOption, sizeof(sMoveLockOption) ) == false )
	{
		assert( 0 && "failed to write move lock option");
	}

	if( saver.Write( &mSkillWinOption, sizeof(sSkillWinOption) ) == false )
	{
		assert( 0 && "failed to write move lock option");
	}
	{
		cSaveWindowPos::cIterator i = mSaveWindowInfo.Begin();
		cSaveWindowPos::cIterator end = mSaveWindowInfo.End();
		for( ; i!=end; i++ )
		{
			sWindowPosData* data = (sWindowPosData*)(i->mSecond);

			saver.Write( data, sizeof(sWindowPosData) );
		}
	}
}

void cGameUIManager::ApplyWindowPos()
{
	/// chatting window
	mpChatWindow->SetPress( mChatWinOption[0].checkNormal,
		mChatWinOption[0].checkParty,
		mChatWinOption[0].checkGuild,
		mChatWinOption[0].checkShout,
		mChatWinOption[0].checkTrade,
		mChatWinOption[0].checkWhisper,
		mChatWinOption[0].checkSystem,
		mChatWinOption[0].checkMegaPhone );

	/// chatting extra window
	for( unsigned int k=1; k<4; k++ )
	{
		mpChatExtraWindow[k-1]->SetPress( mChatWinOption[k].checkNormal,
			mChatWinOption[k].checkParty,
			mChatWinOption[k].checkGuild,
			mChatWinOption[k].checkShout,
			mChatWinOption[k].checkTrade,
			mChatWinOption[k].checkWhisper, 
			mChatWinOption[k].checkSystem,
			mChatWinOption[k].checkMegaPhone );
	}

	///
	mpSmallMiniMapWindow->SetImagePercent( mMiniWinPer );
	mpChatWindow->InitBoxSize( mChatBoxMaxRow );
	for( unsigned int i=0; i<3; i++ )
	{
		mpChatExtraWindow[i]->InitBoxSize( mExtraChatBoxMaxRow[i] );
	}

	///
	mpInventoryWindow->SetBagButtonPressed( 0, mInvenWinOption.showBag1 );
	mpInventoryWindow->SetBagButtonPressed( 1, mInvenWinOption.showBag2 );
	mpInventoryWindow->SetBagButtonPressed( 2, mInvenWinOption.showBag3 );

	mpMiniMapWindow->SetPressExplain( mMiniMapWinOption.showHelper );
	mpMiniMapWindow->SetPressQuestView( mMiniMapWinOption.showQuestView );

	mpPartyWindow->ChangeDisplayType( mPartyStateOption.showFlag );

	mpQuestWindow->SetShowInfo( mQuestOption.infoViewFlag );

	mpChatWindow->SetMoveLock( mMoveLockOption.chatLock );
	mpPartyWindow->SetMoveLock( mMoveLockOption.partyLock );

	mpSkillWindow->SetPressStepListView( mSkillWinOption.showStepListWin );

	///
	if( mSaveWindowInfo.GetSize() == 0 )
		return;

	cSaveWindowPos::cIterator i = mSaveWindowInfo.Begin();
	cSaveWindowPos::cIterator end = mSaveWindowInfo.End();
	for( ; i!=end; i++ )
	{
		bool applyShow = true;
		sWindowPosData* data = (sWindowPosData*)(i->mSecond);

		cUIWindow* win = 0;
		switch( data->index )
		{
		case CHATWIN_POS:		win = mpChatWindow;			break;
		case ECHATWIN1_POS:		win = mpChatExtraWindow[0]; break;
		case ECHATWIN2_POS:		win = mpChatExtraWindow[1]; break;
		case ECHATWIN3_POS:		win = mpChatExtraWindow[2]; break;
		case HEROSTATWIN_POS:	win = mpHeroStateWindow;	break;
		case HEROINFOWIN_POS:	win = mpHeroInfoWindow;		break;
		case TARGETWIN_POS:		win = mpTargetWindow;		break;
		case INVENWIN_POS:		win = mpInventoryWindow;	break;
		case BAGWIN1_POS:		win = mpBagWindow[0];		break;
		case BAGWIN2_POS:		win = mpBagWindow[1];		break;
		case BAGWIN3_POS:		win = mpBagWindow[2];		break;
		case SKILLWIN_POS:		win = mpSkillWindow;		break;
		case OPTWIN_POS:		win = mpOptionWindow;		break;
		case MINIMAPWIN_POS:	win = mpSmallMiniMapWindow;	break;
		case CQUICK_POS:		win = mpColumnQuickSlot[0];	break;
		case RQUICK_POS:		win = mpRowQuickSlot[0];	break;
		case FSCQUICK_POS:		win = mpFSColumnQuickSlot;	break;
		case FSRQUICK_POS:		win = mpFSRowQuickSlot;		break;
		case PARTYWIN_POS:		win = mpPartyWindow;		 applyShow = false;	break;
		case PARTYSTATWIN1_POS:	win = mpPartyStateWindow[0]; applyShow = false; break;
		case PARTYSTATWIN2_POS:	win = mpPartyStateWindow[1]; applyShow = false; break;
		case PARTYSTATWIN3_POS:	win = mpPartyStateWindow[2]; applyShow = false; break;
		case PARTYSTATWIN4_POS:	win = mpPartyStateWindow[3]; applyShow = false; break;
		case PARTYSTATWIN5_POS:	win = mpPartyStateWindow[4]; applyShow = false; break;
		case PARTYSTATMINIWIN1_POS:	win = mpPartyStateMiniWindow[0]; applyShow = false; break;
		case PARTYSTATMINIWIN2_POS:	win = mpPartyStateMiniWindow[1]; applyShow = false; break;
		case PARTYSTATMINIWIN3_POS:	win = mpPartyStateMiniWindow[2]; applyShow = false; break;
		case PARTYSTATMINIWIN4_POS:	win = mpPartyStateMiniWindow[3]; applyShow = false; break;
		case PARTYSTATMINIWIN5_POS:	win = mpPartyStateMiniWindow[4]; applyShow = false; break;
		case FRIENDWIN_POS:		win = mpFriendWindow;		break;
		case TAROTCON_POS:		win = mpTarotControlWindow;	applyShow = false; break;
		case SELLWIN_POS:		win = mpUserSellWindow;		applyShow = false; break;
		case LOOTWIN_POS:		win = mpLootingWindow;		applyShow = false; break;
		case QUESTWIN_POS:		win = mpQuestWindow;		break;
		case SQUESTWIN_POS:		win = mpQuestSimpleWindow;	break;
		case DISJOINT_POS:		win = mpNpcDisjointWindow;	applyShow = false; break;
		case ENHANCEDWIN_POS:	win = mpEnhancedWindow;		break;
		case PUTCARD_POS:		win = mpPutCardWindow;		break;
		case SOCIALWIN_POS:		win = mpSocialWindow;		break;
		case TITLEWIN_POS:		win = mpTitleWindow;		break;
		case GUILDWIN_POS:		win = mpGuildWindow;		break;
		case THEMESTATEWIN_POS: win = mpThemeStateWindow;	break;
		case TODAYWORDWIN_POS:	win = mpTodayWordWindow;	applyShow = false; break;
		case ITEMMIXWIN_POS:	win = mpItemMixWindow;		break;
		case MAILWIN_POS:		win = mpMailWindow;			applyShow = false; break;
		case PARTYUNIONWIN_POS:	win = mpUnionWindow;		applyShow = false; break;
		case PARTYUNIONSMALLWIN_POS: win = mpUnionSmallWindow;	applyShow = false; break;
		case CASHFREEMOVE_POS:	win = mpCashFreeMoveWindow;	break;
		case TWITTER_POS:		win = mpTwitterWindow;	  break;
		case CQUICK_EXTRA1_POS: win = mpColumnQuickSlot[1];  break;
		case RQUICK_EXTRA1_POS: win = mpRowQuickSlot[1];     break;
		case CQUICK_EXTRA2_POS: win = mpColumnQuickSlot[2];  break;
		case RQUICK_EXTRA2_POS: win = mpRowQuickSlot[2];     break;

		default:win = 0; break;
		}

		if( win == 0 )
			continue;

		win->SetRelativePos( cUIPos( (int)data->pos.x, (int)data->pos.y ) );

		if( applyShow == false )
			continue;

		if( data->showFlag >= 1 )
		{
			win->Show( false );

			switch( data->index )
			{
			case RQUICK_POS:   QUICKMAN->SetRowQuick( true , QUICKTYPE_COMMON ); break;
			case FSRQUICK_POS: QUICKMAN->SetRowQuickFS( true );					 break;
			case RQUICK_EXTRA1_POS: QUICKMAN->SetRowQuick( true , QUICKTYPE_COMMON_EXTRA1 ); break;
			case RQUICK_EXTRA2_POS: QUICKMAN->SetRowQuick( true , QUICKTYPE_COMMON_EXTRA2 ); break;
			}
		}
		else
			win->Hide( false );
	}
}

void cGameUIManager::ApplyWinodwOption_CharacterQuest()
{
	///
	for( unsigned int i=0;i<mQuestCheckArr.GetSize();i++ )
		QUESTMAN->AddQuestSelect( mQuestCheckArr[i], true );

	/// 접힌 QuestWin/QuestSimpleWin 맵 카테고리 적용
	if( !mpQuestWindow || !mpQuestSimpleWindow )
	{
		assert(0);
		return;
	}

	cSaveQuestSet::cConstIterator pos = mClosedCategorySet.Begin();
	cSaveQuestSet::cConstIterator end = mClosedCategorySet.End();
	for( ; pos != end; ++pos )
	{
		mpQuestWindow->SetHideMapCategory( *pos );
	}

	pos = mClosedSimpleCategorySet.Begin();
	end = mClosedSimpleCategorySet.End();
	for( ; pos != end; ++pos )
	{
		mpQuestSimpleWindow->SetHideMapCategory( *pos );
	}

	///윈도우 위치 갱신해주기
	sWindowPosData* data = (sWindowPosData*)mSaveWindowInfo.GetAt( SQUESTWIN_POS );
	if( data && data->index == SQUESTWIN_POS )
	{
		cUIWindow* win = mpQuestSimpleWindow;
		if( win )
			win->SetRelativePos( cUIPos( (int)data->pos.x, (int)data->pos.y ) );
	}
}

void cGameUIManager::ApplyWinodwOption_CharacterShortcut()
{
	SHORTCUTMAN->ResetShortcut();

	cShortcutMap::cIterator pos = mShortcutMap.Begin();
	cShortcutMap::cIterator end = mShortcutMap.End();
	for( ; pos!= end; ++pos )
	{
		int shortcutIndex = (*pos).mFirst;
		int keyboardIndex = (*pos).mSecond;

		SHORTCUTMAN->SetShortCutValue( shortcutIndex, keyboardIndex );
	}

	SHORTCUTMAN->UpdateAllIconByShortcut();
}


void cGameUIManager::UpdateWindowPos()
{
	bool holdView;
	for( unsigned int i=0; i<POS_END; i++ )
	{
		holdView = false;

		cUIWindow* win = 0;
		switch( i )
		{
		case CHATWIN_POS:		win = mpChatWindow;			holdView = true;	break;
		case ECHATWIN1_POS:		win = mpChatExtraWindow[0]; break;
		case ECHATWIN2_POS:		win = mpChatExtraWindow[1]; break;
		case ECHATWIN3_POS:		win = mpChatExtraWindow[2]; break;
		case HEROSTATWIN_POS:	win = mpHeroStateWindow;	holdView = true;	break;
		case HEROINFOWIN_POS:	win = mpHeroInfoWindow;		break;
		case TARGETWIN_POS:		win = mpTargetWindow;		break;
		case INVENWIN_POS:		win = mpInventoryWindow;	break;
		case BAGWIN1_POS:		win = mpBagWindow[0];		break;
		case BAGWIN2_POS:		win = mpBagWindow[1];		break;
		case BAGWIN3_POS:		win = mpBagWindow[2];		break;
		case SKILLWIN_POS:		win = mpSkillWindow;		break;
		case OPTWIN_POS:		win = mpOptionWindow;		break;
		case MINIMAPWIN_POS:	win = mpSmallMiniMapWindow;	holdView = true;	break;
		case CQUICK_POS:		win = mpColumnQuickSlot[0];	holdView = QUICKMAN->IsRowQuick( QUICKTYPE_COMMON )? false:true; break;
		case RQUICK_POS:		win = mpRowQuickSlot[0];	holdView = QUICKMAN->IsRowQuick( QUICKTYPE_COMMON )? true:false; break;
		case FSCQUICK_POS:		win = mpFSColumnQuickSlot;	holdView = QUICKMAN->IsRowQuickFS()? false:true; break;
		case FSRQUICK_POS:		win = mpFSRowQuickSlot;		holdView = QUICKMAN->IsRowQuickFS()? true:false; break;
		case PARTYWIN_POS:		win = mpPartyWindow;		break;
		case PARTYSTATWIN1_POS:	win = mpPartyStateWindow[0];break;
		case PARTYSTATWIN2_POS:	win = mpPartyStateWindow[1];break;
		case PARTYSTATWIN3_POS:	win = mpPartyStateWindow[2];break;
		case PARTYSTATWIN4_POS:	win = mpPartyStateWindow[3];break;
		case PARTYSTATWIN5_POS:	win = mpPartyStateWindow[4];break;
		case PARTYSTATMINIWIN1_POS:	win = mpPartyStateMiniWindow[0];break;
		case PARTYSTATMINIWIN2_POS:	win = mpPartyStateMiniWindow[1];break;
		case PARTYSTATMINIWIN3_POS:	win = mpPartyStateMiniWindow[2];break;
		case PARTYSTATMINIWIN4_POS:	win = mpPartyStateMiniWindow[3];break;
		case PARTYSTATMINIWIN5_POS:	win = mpPartyStateMiniWindow[4];break;
		case FRIENDWIN_POS:		win = mpFriendWindow;		break;
		case TAROTCON_POS:		win = mpTarotControlWindow;	break;
		case SELLWIN_POS:		win = mpUserSellWindow;		break;
		case LOOTWIN_POS:		win = mpLootingWindow;		break;
		case QUESTWIN_POS:		win = mpQuestWindow;		break;
		case SQUESTWIN_POS:		win = mpQuestSimpleWindow;	break;
		case DISJOINT_POS:		win = mpNpcDisjointWindow;	break;
		case ENHANCEDWIN_POS:	win = mpEnhancedWindow;		break;
		case PUTCARD_POS:		win = mpPutCardWindow;		break;
		case SOCIALWIN_POS:		win = mpSocialWindow;		break;
		case TITLEWIN_POS:		win = mpTitleWindow;		break;
		case GUILDWIN_POS:		win = mpGuildWindow;		break;
		case THEMESTATEWIN_POS: win = mpThemeStateWindow;	break;
		case TODAYWORDWIN_POS:	win = mpTodayWordWindow;	break;
		case ITEMMIXWIN_POS:	win = mpItemMixWindow;		break;
		case MAILWIN_POS:		win = mpMailWindow;			break;
		case PARTYUNIONWIN_POS:	win = mpUnionWindow;		break;
		case PARTYUNIONSMALLWIN_POS: win = mpUnionSmallWindow;	break;
		case CASHFREEMOVE_POS:	win = mpCashFreeMoveWindow;		break;
		case TWITTER_POS:		win = mpTwitterWindow;	break;
		case CQUICK_EXTRA1_POS: win = mpColumnQuickSlot[1]; break;
		case RQUICK_EXTRA1_POS: win = mpRowQuickSlot[1];   	break;
		case CQUICK_EXTRA2_POS: win = mpColumnQuickSlot[2];	break;
		case RQUICK_EXTRA2_POS: win = mpRowQuickSlot[2];   	break;
		case SUMMON_POS:		win = mpSummonWindow;		break;
		default:win = 0; break;
		}

		if( win == 0 )
			continue;

		sWindowPosData* data = (sWindowPosData*)mSaveWindowInfo.GetAt( i );
		if( data == 0 )
		{
			data = new sWindowPosData;
			data->index = i;

			if( mSaveWindowInfo.Insert( data->index, data ) == false )
			{
				assert(0);
				continue;
			}
		}

		cUIRect rt= win->GetAbsoluteRect();
		data->pos = NiPoint2( (float)rt.mLeft, (float)rt.mTop );

		if( holdView )
			data->showFlag = 1;
		else
		{
			data->showFlag = win->IsVisible()? 1:0;

			if( i == TARGETWIN_POS || i == CASHFREEMOVE_POS || i == TWITTER_POS )
				data->showFlag = 0;
		}
	}

	/// windows extra option
	/// chatting
	mChatWinOption[0].checkNormal = mpChatWindow->IsPressNormal();
	mChatWinOption[0].checkParty = mpChatWindow->IsPressParty();
	mChatWinOption[0].checkGuild = mpChatWindow->IsPressGuild();
	mChatWinOption[0].checkShout = mpChatWindow->IsPressShout();
	mChatWinOption[0].checkTrade = mpChatWindow->IsPressTrade();
	mChatWinOption[0].checkWhisper = mpChatWindow->IsPressWhisper();
	mChatWinOption[0].checkSystem = mpChatWindow->IsPressSystem();
	mChatWinOption[0].checkMegaPhone = mpChatWindow->IsPressMegaPhone();

	/// chatting extra
	for( unsigned int i=1; i<4; i++ )
	{
		mChatWinOption[i].checkNormal = mpChatExtraWindow[i-1]->IsPressNormal();
		mChatWinOption[i].checkParty = mpChatExtraWindow[i-1]->IsPressParty();
		mChatWinOption[i].checkGuild = mpChatExtraWindow[i-1]->IsPressGuild();
		mChatWinOption[i].checkShout = mpChatExtraWindow[i-1]->IsPressShout();
		mChatWinOption[i].checkTrade = mpChatExtraWindow[i-1]->IsPressTrade();
		mChatWinOption[i].checkWhisper = mpChatExtraWindow[i-1]->IsPressWhisper();
		mChatWinOption[i].checkSystem = mpChatExtraWindow[i-1]->IsPressSystem();
		mChatWinOption[i].checkMegaPhone = mpChatExtraWindow[i-1]->IsPressMegaPhone();
	}

	///
	mChatBoxMaxRow = mpChatWindow->GetMaxRowInPage();
	for( unsigned int i=0; i<3; i++ )
		mExtraChatBoxMaxRow[i] = mpChatExtraWindow[i]->GetMaxRowInPage();

	mMiniWinPer = mpSmallMiniMapWindow->GetImagePercent();

	mInvenWinOption.showBag1 = mpInventoryWindow->IsShowBag(0);
	mInvenWinOption.showBag2 = mpInventoryWindow->IsShowBag(1);
	mInvenWinOption.showBag3 = mpInventoryWindow->IsShowBag(2);

	mMiniMapWinOption.showHelper = mpMiniMapWindow->IsPressExplain();
	mMiniMapWinOption.showQuestView = mpMiniMapWindow->IsPressQuestView();

	mPartyStateOption.showFlag = mpPartyWindow->GetDisplayType();
	mQuestOption.infoViewFlag  = mpQuestWindow->IsShowInfo();

	mMoveLockOption.chatLock = mpChatWindow->IsLockedMove();
	mMoveLockOption.partyLock = mpPartyWindow->IsLockedMove();

	mSkillWinOption.showStepListWin = mpSkillWindow->IsShowStepListWin();
	
}

void cGameUIManager::UpdateWindowOption_Character()
{
	///
	mQuestCheckArr.Clear();

	tHashSet<unsigned long>& questSet = QUESTMAN->GetSelectedSet();
	tHashSet<unsigned long>::cIterator itr = questSet.Begin();
	tHashSet<unsigned long>::cIterator end = questSet.End();
	for( ; itr!=end; itr++ )
	{
		unsigned long questIdx = (unsigned long)(*itr);
		mQuestCheckArr.PushBack( questIdx );
	}

	mClosedCategorySet.Clear();

	if( mpQuestWindow )
	{
		mpQuestWindow->GetHideMapCategorySet( mClosedCategorySet );
	}

	mClosedSimpleCategorySet.Clear();

	if( mpQuestSimpleWindow )
	{
		mpQuestSimpleWindow->GetHideMapCategorySet( mClosedSimpleCategorySet );
	}

	mShortcutMap.Clear();

	cShortcutMap* shortcutMap;
	shortcutMap = SHORTCUTMAN->GetShortcutMap();
	if( shortcutMap )
	{
		cShortcutMap::cIterator pos = shortcutMap->Begin();
		cShortcutMap::cIterator end = shortcutMap->End();

		for( ; pos!=end; ++pos )
		{
			int shortcutIndex = (*pos).mFirst;
			int keyboardIndex = (*pos).mSecond;
			mShortcutMap.Insert( shortcutIndex, keyboardIndex );
		}
	}

	for( int i = 0; i < 5; i++ )
		mTitleOption.titleIdxArr[i] = -1;

	if( TITLEMAN )
		TITLEMAN->GetQuickTitle( mTitleOption.titleIdxArr );
}

void cGameUIManager::OpenNpcUI( unsigned long npcIndex )
{
	// npc 타입에 따라 적용
	cNpc* pNpc = (cNpc*)OBJECTMAN->GetNpc( npcIndex );
	if( pNpc )
	{
		// npc 타입에 따라 분류
		sNPCList* npcInfo = NPCSCRIPT->GetNPCList( pNpc->GetClassId() );
		if( npcInfo )
		{
			UIMAN->ShowGameUI();

			switch( npcInfo->mType )
			{
			case eNPCTYPE_NORMAL:
				{
					// 일반
					if( mpNpcWindow )
						mpNpcWindow->Show( pNpc->GetClassId(), npcIndex, pNpc->GetName() );
				}
				break;
			case eNPCTYPE_MAIL:
				{
					// 우편
					if( mpMailWindow )
						mpMailWindow->Show( npcIndex );
				}
				break;
			case eNPCTYPE_BOARD:
				{
					// 게시판
					if( mpBoardWindow )
						mpBoardWindow->Show( npcIndex );
				}
				break;
			case eNPCTYPE_TUTORIALBOOK:
				{
					// 튜토리얼 북
					if( mpNpcTutorialWindow )
						mpNpcTutorialWindow->Show( pNpc->GetClassId(), npcIndex, pNpc->GetName() );
				}
				break;
			default:
				assert(0);
				break;
			}
		}
		else
			assert(0);
	}
	else
		assert(0);
}
