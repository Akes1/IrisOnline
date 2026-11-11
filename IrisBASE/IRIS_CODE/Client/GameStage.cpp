#include "stdafx.h"
#include "GameStage.h"
#include "StageManager.h"

#include "Application.h"
#include "UIManager.h"
#include "InputSystem.h"

#include "CameraManager.h"
#include "ObjectManager.h"
#include "WorldManager.h"
#include "ItemManager.h"

#include "SkillExecuter.h"
#include "SkillManager.h"
#include "CommunityScript.h"
#include "ChatManager.h"
#include "PartyManager.h"
#include "ItemMixManager.h"
#include "PartyUnionManager.h"
#include "GameResourceManager.h"
#include "FilterManager.h"
#include "ShortcutManager.h"
#include "TutorialManager.h"
#include "StageScript.h"
#include "GameUIManager.h"
#include "WebManager.h"
#include "OptionManager.h"
#include "TwitterScript.h"

#include "Npc.h"
#include "Monster.h"
#include "Hero.h"
#include "DoorSceneNode.h"

#include "MsgBox.h"
#include "FriendWindow.h"
#include "SmallMiniMapWindow.h"
#include "MiniMapWindow.h"
#include "TarotManager.h"
#include "TarotWindow.h"
#include "TarotControlWindow.h"
#include "InventoryWindow.h"
#include "NumberEditBox.h"
#include "CastingBar.h"
#include "WareHouseWindow.h"
#include "MoneyWindow.h"
#include "NpcWindow.h"
#include "RevivalWindow.h"
#include "MenuWindow.h"
#include "SkillWindow.h"
#include "SkillSheetWindow.h"
#include "UserSellWindow.h"
#include "ItemMsgWindow.h"
#include "ItemCountWindow.h"
#include "PutCardWindow.h"
#include "CardBookWindow.h"
#include "HeroInfoWindow.h"
#include "SocialWindow.h"
#include "LootingWindow.h"
#include "ChatWindow.h"
#include "TodayWordWindow.h"
#include "MailWindow.h"
#include "MailSheet.h"
#include "NpcDealSearchSheet.h"
#include "NpcDealWindow.h"
#include "NpcDealRegistSheet.h"
#include "TargetWindow.h"
#include "TwitterWindow.h"
#include "OptionWindow.h"
#include "OptionSheet.h"
#include "DuelWindow.h"
#include "ThemeStateWindow.h"
#include "LookWindow.h"

#include "Skill_Common.h"
#include "Npc_Common.h"
#include "Quest_Common.h"
#include "Cheat_Common.h"
#include "Totem_Common.h"
#include "Gathering_Common.h"
#include "Stage_Common.h"
#include "Obstruction_Common.h"
#include "UserPortal_Common.h"
#include "Notice_Common.h"
#include "Tutorial_Common.h"
#include "TeleportPortal_Common.h"
#include "Player_Common.h"
#include "Twitter_Common.h"

#include "SkillStoreWindow.h"
#include "QuickManager.h"
#include "SceneManager.h"
#include "PartyManager.h"
#include "QuestManager.h"
#include "CheatManager.h"
#include "TitleManager.h"
#include "GuildManager.h"
#include "TipManager.h"
#include "ThemeManager.h"

#include "UICursor.h"
#include "ChatBubble.h"

#include "NpcRewardWindow.h"
#include "ExitWindow.h"
#include "ObjectNameCard.h"
#include "Duel_Common.h"
#include "DuelManager.h"
#include "PVPManager.h"
#include "DMMsgWindow.h"
#include "GuildWindow.h"
#include "CamaelWindow.h"

// 순간이동
#include "Protocol_Cheat.h"
#include "rendersystem.h"
#include "SoundSystem.h"

#ifdef _GMTOOL
	#include "GMTool.h"
#endif

cGameStage::cGameStage()
: mpUI(0)
, mpDuelManager(0)
{
}

cGameStage::~cGameStage()
{
}

void cGameStage::Init( cStageManager* pStageManager )
{
	mpStageManager = pStageManager;
}

void cGameStage::InitStage()
{
	if( mInit ) return;

	/// UI
	mpUI = new cGameUIManager;
	mpUI->Init();

	mpDuelManager = new cDuelManager;

	mInit = true;
}

void cGameStage::Exit()
{
	/// UI
	SAFE_DELETE(mpDuelManager);
	SAFE_EXIT_DELETE( mpUI );

	mpStageManager = NULL;
}

void cGameStage::Open()
{
	RENDERSYS->SetWorldRendering( true );

	CAMERAMAN->Open();
	WORLDMAN->Open();

#ifdef NOCONNECT_SERVER
	OBJECTMANAGER->AddHero();
#endif

	OBJECTMANAGER->Open();
	SKILLMAN->Open();
	ITEMMAN->Open();
	PARTYMAN->Open();
	GUILDMAN->Open();
	
	/// UI
	mpUI->Open();
	mpDuelManager->Open();
	TUTORIALMAN->Open();
	
	PVPMAN->Open();

	SOUNDSYS->Open( STAGEMAN->GetMapIdx(), STAGEMAN->GetMapMode() );
}

void cGameStage::Close()
{
	SOUNDSYS->Close();

	/// 순서 변경 절대 불가
	PARTYMAN->Close();
	PARTYUNIONMAN->Close();

	THEMEMAN->Close();
	PVPMAN->Close();
	TUTORIALMAN->Close();
	ITEMMAN->Close();
	SKILLMAN->Close();
	OBJECTMAN->Close();
	QUESTMAN->Close();
	TITLEMAN->Close();
	GUILDMAN->Close();
	ITEMMIXMAN->Close();
	CHATMANAGER->Close();
	
	/// UI
	mpUI->Close();
	
	OBJECTMAN->ClearHeroData();
	SCENEMAN->Clear();
}

void cGameStage::Process( unsigned long deltaTime, unsigned long accumTime )
{
	/// duel manager
	mpDuelManager->Process( deltaTime );

	///
	TIPMAN->Process( accumTime );

	/// 
	THEMEMAN->Process( deltaTime, accumTime );

	
	/// UI
	mpUI->Process( deltaTime, accumTime );
}

void cGameStage::ProcessLButtonClick( cMouseAgent* pMouse )
{
	assert(pMouse);
	if( HERO == 0 || 
		HERO->GetState() == eOBJECT_STATE_DIE )
	{
		return;
	}

	/// npc대화 모드..
	if( HERO->IsOpenNpcTalk() == true )
		return;

	int mouseX = pMouse->GetMouseEventX();
	int mouseY = pMouse->GetMouseEventY();

	if( SKILLEXECUTER->IsFieldSkillCharge() == true )
	{
		NiPoint3 pos;
		sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
		if( WORLDMAN->Pick( &pos, mouseX, mouseY ) == false )
		{
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
			return;
		}
		if( WORLDMAN->EnableMove( mouseX, mouseY) == false )
		{
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
			return;
		}

		if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
		else
		{
			if( HERO && HERO->IsReadyUseSkill() == true )
				SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, pos );
		}

		return;
	}

	/// 이부분은 관리 모드에서만 들어가도록 체크하시오!!
#ifdef _GMTOOL
	if( pMouse->ControlPressed() == true )
	{
		if( HERO->GetState() == eOBJECT_STATE_DIE )
			return;

		if( HERO->GetState() == eOBJECT_STATE_STOP )
		{
			eSTOPFLAG stopFlag = HERO->GetStopFlag();
			if( !(stopFlag == eSTOP_ENHANCED || stopFlag == eSTOP_ITEMMIX || stopFlag == eSTOP_NPCSPEECH ) )
				return;
		}

		NiPoint3 pos;
		if( WORLDMAN->Pick( &pos, mouseX, mouseY ) == false )
			return;

		if( pos.x < 0 || pos.y < 0 )
		{
			assert(0);
			return;
		}

		if( NETWORK->IsConnected() == true )
		{
			MSG_REQ_MOVE Msg;
			Msg.Category = NM_CHEAT;
			Msg.Protocol = NM_CHEAT_TELEPORT_REQ;
			Msg.destX = pos.x;
			Msg.destY = pos.y;

			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
		}

		//if( WORLDMAN == 0 )
		//{
		//	assert(0);
		//	return;
		//}

		///// 높이값 계산
		//if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
		//{
		//	HERO->SetState( eOBJECT_STATE_IDLE );
		//	HERO->SetPos( pos );
		//}
		//else
		//{
		//	assert(0);
		//	return;
		//}

		return;
	}

	/// 운영자 전용 스킬을 썼는지 체크
	if( GMTOOL->ProcessGMSkill( mouseX, mouseY ) == true )
		return;

#endif

	if( SKILLMAN->ProcessLButtonClick( mouseX, mouseY ) == false )
	{
		if( OBJECTMAN->ProcessLButtonClick( mouseX, mouseY ) == false )
		{
			/// Hero 이동 처리..
			if( HERO->IsKeyMoving() == true )
			{
				if( HERO->IsGoContinue() == true )
					OBJECTMAN->HeroGoto( mouseX, mouseY );
			}
			else
			{
				OBJECTMAN->HeroGoto( mouseX, mouseY );
			}
		}
	}
}

void cGameStage::ProcessLButtonDClick( cMouseAgent* pMouse )
{
	assert(pMouse);
	if( HERO == 0 || 
		HERO->GetState() == eOBJECT_STATE_DIE )
	{
		return;
	}

	/// npc대화 모드..
	if( HERO->IsOpenNpcTalk() == true )
		return;

	int mouseX = pMouse->GetMouseEventX();
	int mouseY = pMouse->GetMouseEventY();

	if( SKILLMAN->ProcessLButtonDClick( mouseX, mouseY ) == false )
	{
		OBJECTMAN->ProcessLButtonDClick( mouseX, mouseY );
	}
}

void cGameStage::ProcessRButtonDown( cMouseAgent* pMouse )
{
	assert(pMouse);
	if( HERO == 0 || HERO->GetState() == eOBJECT_STATE_DIE )
		return ;

	/// npc대화 모드..
	if( HERO->IsOpenNpcTalk() == true )
		return;

	int mouseX = pMouse->GetMouseEventX();
	int mouseY = pMouse->GetMouseEventY();

	OBJECTMAN->ProcessRButtonDown( mouseX, mouseY );
}

void cGameStage::ProcessRButtonUp( cMouseAgent* pMouse )
{
	assert(pMouse);
	if( HERO == 0 || HERO->GetState() == eOBJECT_STATE_DIE )
		return;

	/// npc대화 모드..
	if( HERO->IsOpenNpcTalk() == true )
		return;

	int mouseX = pMouse->GetMouseEventX();
	int mouseY = pMouse->GetMouseEventY();

	OBJECTMAN->ProcessRButtonUp( mouseX, mouseY );
}


static unsigned long mMouseOverTime = 0;
void cGameStage::ProcessMouseover( unsigned long time, cMouseAgent* pMouse )
{
	/// npc대화 모드..
	if( HERO->IsOpenNpcTalk() == true )
		return;

	assert(pMouse);
	int mouseX = pMouse->GetMouseX();
	int mouseY = pMouse->GetMouseY();

	if( SKILLEXECUTER->IsFieldSkillCharge() == true )
	{
		mMouseOverTime = 0;

		if( WORLDMAN->EnableMove( mouseX, mouseY) == false )
		{
			/// cursor X
			CURSOR->SetCursor( eCURSOR_MOVESTOP );
			return;
		}

		NiPoint3 pos;
		if( WORLDMAN->Pick( &pos, mouseX, mouseY ) == false )
			return;
		if( WORLDMAN->CalcHeight(&pos.z, pos.x, pos.y) == false )
			return;

		sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
		if( (HERO->GetPos() - pos).Length() > chargeInfo.mSkillRange )
		{
			/// cursor X
			CURSOR->SetCursor( eCURSOR_MOVESTOP );
			return;
		}

		HERO->UpdateFieldTargetEffect( pos );

		CURSOR->SetCursor( eCURSOR_MAX );

		return;
	}

	if( mMouseOverTime > 200 )
	{
		mMouseOverTime = 0;

		if( OBJECTMAN->ProcessMouseOver( mouseX, mouseY ) == false )
		{
			/// navifield check
			if( WORLDMAN->EnableMove( mouseX, mouseY ) == false )
				CURSOR->SetCursor( eCURSOR_MOVESTOP );
			else
				CURSOR->SetCursor( eCURSOR_DEFAULT );
		}
	}
	else
	{
		mMouseOverTime += time;
	}
}

void cGameStage::ProcessKeyUpDown( unsigned long time, cKeyboardAgent* pKeyboard )
{
#ifdef _GMTOOL
	if( pKeyboard->GetKeyDown( KEY_MOVE_UP )   || pKeyboard->GetKeyDown( KEY_MOVE_LEFT ) ||
		pKeyboard->GetKeyDown( KEY_MOVE_DOWN ) || pKeyboard->GetKeyDown( KEY_MOVE_RIGHT ) )
	{
		if( CAMERAMAN->GetCurrentIdx() != 0 )
			return;
	}
#endif

	if( HERO->IsOpenNpcTalk() == true )
	{
		/// 타로중이면 리턴
		if( TAROTMAN->IsUsingTarot() )
			return;

		/// npc모드에서의 키보드 처리를 별도로 정리한다...
		if( pKeyboard->GetKeyDown(KEY_ESCAPE) )
		{
			UIMAN->HideESC();
		}
		return;
	}

#ifdef _GMTOOL
	if( pKeyboard->GetKeyPressed(KEY_CONTROL) )
	{
		if( pKeyboard->GetKeyDown(KEY_B) )
		{
			if( CAMERAMAN->GetCurrentIdx() == 0 )
			{
				NiPoint3 pos = CAMERAMAN->GetCurrentCameraPos();
				CAMERAMAN->ChangeCamera(2);
				CAMERAMAN->GetCurrent()->SetTranslate( pos );
			}
			else
			{
				CAMERAMAN->ChangeCamera(0);
			}
			return;
		}
	}
#endif
	
	if( pKeyboard->GetKeyDown(KEY_ESCAPE) )
	{
		/// 타로중이면 리턴
		if( TAROTMAN->IsUsingTarot() )
		{
			return;
		}
		if( SKILLEXECUTER->IsSkillCharge() ) 
		{
			/// 스킬이 장착 중이라면 해제...
			sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
			return;
		}
		if( UIMAN->HideESC() )
		{
			/// 윈도우창 차례대로 닫기...
			return;
		}
		if( HERO->GetState() == ePLAYER_STATE_GATHERING && HERO->GatheringCancel() == true )
		{
			return;
		}
		cSkillObject* pCurrentSKillObj = SKILLEXECUTER->GetCurrentHeroSkillObject();
		if( pCurrentSKillObj )
		{
			if( pCurrentSKillObj->GetDramaState() == eDRAMASTATE_CASTING )
			{
				cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
				if( pWin )
				{
					if( pWin->GetPercent() < 98.0f )
					{
						/// cast stop
						pCurrentSKillObj->WaitCancel();
						pWin->Clear( eCasting_Skill );
						HERO->SetState( eOBJECT_STATE_IDLE );

						/// 070712 PKH 스킬 요청 메세지 발송
						MSG_REQ_SKILL_CAST_CANCEL Msg;
						Msg.Category = NM_SKILL;
						Msg.Protocol = NM_SKILL_CAST_CANCEL_REQ;
						Msg.mAttacker.index = HERO->GetObjectID();
						Msg.mAttacker.type = HERO->GetObjectType();

						NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );
					}
				}

				if( pCurrentSKillObj->GetSpecialType() == eSpecial_MapChange )
				{
					ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
				}
				else if( pCurrentSKillObj->GetSpecialType() == eSpecial_Vehicle )
				{
					ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
				}

				/// 캐스팅 취소 패킷을 쏜다..
				SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
			}
			else if( pCurrentSKillObj->GetDramaState() == eDRAMASTATE_ACTIVITY && pCurrentSKillObj->IsChanneling() )
			{

				cCastingBar* pCastingWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
				if( pCastingWin )
				{
					pCastingWin->Clear( eCasting_Skill );
				}

				pCurrentSKillObj->WaitCancel();
				HERO->SetState( eOBJECT_STATE_IDLE );

				/// 070712 PKH 스킬 요청 메세지 발송
				MSG_REQ_CHANNELING_SKILL_CANCEL Msg;
				Msg.Category = NM_SKILL;
				Msg.Protocol = NM_SKILL_CHANNELING_CANCEL_REQ;
				Msg.mAttacker.index = HERO->GetObjectID();
				Msg.mAttacker.type = HERO->GetObjectType();

				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

				if( pCurrentSKillObj->GetSpecialType() == eSpecial_MapChange )
				{
					ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
				}
				else if( pCurrentSKillObj->GetSpecialType() == eSpecial_Vehicle )
				{
					ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
				}

				/// 캐스팅 취소 패킷을 쏜다..
				SKILLEXECUTER->SetCurrentHeroSkillObject( 0 );
			}

			return;
		}

		if( HERO->GetTargetObject() )
		{
			/// 타켓창 해제...
			HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
			return;
		}

		/// 찾아가기 기능중일때 취소처리
		if( HERO->IsActiveFindPositon() )
		{
			HERO->StopFindPos();
			return;
		}

		/// 따라가기 기능중일때 취소처리
		if( HERO->IsFollowing() )
		{
			HERO->ReleaseFollowPlayer();
			return;
		}

		/// 종료창 띄우기...
		if( TUTORIALMAN->IsTutorialMode() == false )
		{
			mpUI->ShowExitWindow();
		}
		else
		{
			mpUI->ShowTutorialExitWindow();
		}
		
		return;
	}

	/// 캐릭터 관련 단축..
	if( pKeyboard->GetKeyDown( KEY_ATTACK ) )
	{
		HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

		if( SKILLEXECUTER->IsSkillCharge() )
		{
			sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
			return;
		}

		/// 공격
		if( HERO->GetState() != eOBJECT_STATE_ATTACK )
			SKILLEXECUTER->ExecuteAttack( HERO->GetTargetObject() );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_IDLE_STATE ) )
	{
		/// 전투/비전투 전환
		HERO->ToggleIdleFlag();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_SITDOWN ) )
	{
		HERO->ToggleSitDown();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_JUMP ) )
	{
		// 점프 
		HERO->Jump();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_AUTO_RUN ) )
	{
		// 자동 달리기
		HERO->GoCountinueMove();
		return;
	}

	/// UI창 ON/OFF
	if( pKeyboard->GetKeyDown( KEY_RETURN ) )
	{
		/// 채팅창에 포커스를 준다.
		UIMAN->ShowGameUI();
		mpUI->OnFocusChatWindow( KEY_RETURN );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_OPTION ) )
	{
		/// 옵션창
		UIMAN->ShowGameUI();
		mpUI->ToggleOptionWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_GUILD ) )
	{
		/// 길드창
		UIMAN->ShowGameUI();
		mpUI->ToggleGuildWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUEST ) )
	{
		/// 퀘스트창
		UIMAN->ShowGameUI();
		mpUI->ToggleQuestWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_SKILL ) )
	{
		/// 스킬창
		UIMAN->ShowGameUI();
		mpUI->ToggleSkillWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_FRIEND ) )
	{
		/// 친구창
		UIMAN->ShowGameUI();
		mpUI->ToggleFriendWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_INVEN ) )
	{
		/// 인벤토리창
		UIMAN->ShowGameUI();
		mpUI->ToggleInventoryWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_ANSWHISPER ) )
	{
		UIMAN->ShowGameUI();

		/// 귓속말 답장
		cChatWindow* win = mpUI->GetChatWindow();
		if( win )
			win->ApplyLastWhisperUser();
		return;
	}
// 	else if( pKeyboard->GetKeyDown( KEY_QUICK_SWITCH ) )
// 	{
// 		/// 단축 슬롯 변경..
// 		UIMAN->ShowGameUI();
// 		mpUI->ChangeTab();
// 		return;
//	}
	else if( pKeyboard->GetKeyDown( KEY_WIDE_MAP ))
	{
		/// 큰지도 열기
		UIMAN->ShowGameUI();
		mpUI->ToggleMiniMapWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_HIDE_UI ) )
	{
		/// 모든창 보이기/안보이기
		mpUI->ToggleGameUI();

		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_CHAR_INFO ) )
	{
		/// 캐릭터 창
		UIMAN->ShowGameUI();
		mpUI->ToggleHeroInfoWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_BOOK ) )
	{
		/// 컬렉션북 창
		UIMAN->ShowGameUI();
		mpUI->ToggleCardBookWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_PVP ) )
	{
		/// 사회창
		UIMAN->ShowGameUI();
		mpUI->ToggleSocialWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_ITEM_PICK ) )
	{
		/// 루팅창 전체습득 단축
		UIMAN->ShowGameUI();
		mpUI->EventLootingWindow();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_SLASH ) ) 
	{
		mpUI->OnFocusChatWindow( KEY_SLASH );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_WHISPER ) ) 
	{
		mpUI->OnFocusChatWindow( KEY_APOSTROPHE );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_CHAT_PARTY ) ) 
	{
		mpUI->OnFocusChatWindow( KEY_SEMICOLON );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_CHAT_YELL ) )
	{
		mpUI->OnFocusChatWindow( KEY_PERIOD );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_CHAT_GUILD ) )
	{
		mpUI->OnFocusChatWindow( KEY_COMMA );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_DUNGEON ) )
	{
		UIMAN->ShowGameUI();
		mpUI->ToggleThemeStateWindow();
		return;
	}
	
	/// 단축창 지원..
	if( pKeyboard->GetKeyDown( KEY_TARGET ) )
	{
		UIMAN->ShowGameUI();
		HERO->CheckTabTargetObject();
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_WEAPON_SWAP ) )
	{
		/// 스킬 사용중이면 변경 제한
		if( HERO->IsReadyUseSkill() == false )
		{
			/// 무기 스위칭 등록..
			HERO->WaitWeaponSwitch();
			return;
		}
		if( HERO->CheckSwichDelay() )
			return;

		/// 장착한 무기가 하나도 없으면 리턴
		if( !ITEMMAN->IsEquipWeapon() )
			return;

		/// 이미 무기변경 요청중이면 제한
		if( HERO->IsWeaponChanging() == true )
			return;

		if( ITEMMAN->TryToSwitchWeapon() )
			HERO->SetWeaponChangeFlag( true );

		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK1 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK2 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 1 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK3 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 2 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK4 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 3 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK5 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 4 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK6 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 5 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK7 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 6 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK8 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 7 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK9 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 8 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA1_QUICK0 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + 9 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK1 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK2 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 1 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK3 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 2 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK4 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 3 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK5 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 4 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK6 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 5 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK7 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 6 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK8 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 7 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK9 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 8 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_EXTRA2_QUICK0 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + 9 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK1 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK2 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 1 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK3 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 2 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK4 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 3 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK5 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 4 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK6 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 5 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK7 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 6 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK8 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 7 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK9 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 8 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_QUICK0 ) )
	{
		///
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_FS_BEGIN + 9 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK1 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK2 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 1 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK3 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 2 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK4 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 3 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK5 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 4 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK6) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 5 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK7) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 6 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK8 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 7 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK9 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 8 );
		return;
	}
	else if( pKeyboard->GetKeyDown( KEY_COMMON_QUICK0 ) )
	{
		QUICKMAN->OnKeyDown( QUICKSLOT_ICON_COMMON_BEGIN + 9 );
		return;
	}
	else if( pKeyboard->GetKeyDown(KEY_F11) )
	{
		if( TUTORIALMAN->IsTutorialMode() == true )
			return;
		
		/// 열려져 있는 윈도우를 닫을려고 했을 경우
		cUIWindow* shortcutWindow = (cUIWindow*)mpUI->GetShortcutWindow();
		if( shortcutWindow )
		{
			if(shortcutWindow->IsVisible() == true )
			{
				/// 취소 메시지 창 출력
				if( SHORTCUTMAN->IsSettingChanged() == true )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						 				 eMBEVENT_SHORTCUT_CANCEL, eSKIN_YESNO,
										 GAMERESOURCEMAN->GetGameText( 10202 ),
										 GAMERESOURCEMAN->GetGameText( 10200 ) );
				}
				else
					shortcutWindow->Hide();
			}
			else
			{
				shortcutWindow->Show();
			}
		}
		return;
	}
	
	cBaseStage::ProcessKeyUpDown( time, pKeyboard );
}

void cGameStage::NetworkMsgParser( unsigned long /*connectionindex*/, char* pMsg, unsigned long length )
{
	MSGROOT* pTemMsg = (MSGROOT*)pMsg;
	switch( pTemMsg->Category )
	{

	case NM_TWITTER:
		{
			switch( pTemMsg->Protocol )
			{ 
			case NM_TWITTER_MONSTER_DIE_SYN:
				{
					if( OPTIONMAN->IsAutoSendTwitterMsg()	== true ) 
					{
						MSG_SYN_TWITTER_MONSTER_DIE* msg = (MSG_SYN_TWITTER_MONSTER_DIE*)pMsg;
						
						if( TWITTERSCRIPT->IsMonsterEvnet( msg->mMonsterIdx ) == true )
						{
							cTwitterWindow* pTwitWin = GAMEUI->GetTwitterWindow();
							if( pTwitWin == NULL )
							{
								assert(0);
								return;
							}

							if( pTwitWin->IsLogin() == true )
							{
								sTwitterEvent tEvent;
								tEvent.mEvent	= eTWITEVENT_MONSTER;
								tEvent.mExData	= (void*)msg->mMonsterIdx;

								pTwitWin->AddEvent( tEvent );
							}
						}
					}
				}
				break;
			}
		}
		break;		
	case NM_USER:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_USER_TWITTER_RES:
				{
					cTwitterWindow*	pTwitWin					= mpUI->GetTwitterWindow();
					cTwitterRegisterWindow* pTwitRegisterWin	= mpUI->GetTwitterRegisterWindow();
					cOptionWindow*	pOptionWin					= GAMEUI->GetOptionWindow();
					cOptionSheet3*	pTwitSheet					= NULL;
					if( pOptionWin )
					{
						pTwitSheet = (cOptionSheet3*)pOptionWin->GetSheetByID( eUIID_OPTION_SHEET3 );
						if( pTwitSheet == NULL )
						{
							assert(0);
							return;
						}
					}
					else
					{
						assert(0);
						return;
					}

					if( pTwitWin && pTwitRegisterWin && pTwitSheet )
					{
						pTwitRegisterWin->NetUnLock();

						MSG_RES_TWITTER* msg = (MSG_RES_TWITTER*)pMsg;

						if( msg->ErrorCode != ERROR_TWITTER_SUCCESS )	
						{	
							// 계정 등록 실패
							if( pTwitWin->GetRegistType() == cTwitterWindow::eRegister_OnlySave		||
								pTwitWin->GetRegistType() == cTwitterWindow::eRegister_LoginAndSave )
							{
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 11738 ),	// 알림 메세지
									GAMERESOURCEMAN->GetGameText( 2 )		// 알림
									);
							}
							// 계정 초기화 실패
							else		
							{
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 11741 ),	// 알림 메세지
									GAMERESOURCEMAN->GetGameText( 2 )		// 알림
									);
							}

						}
						else	// 성공
						{	// 계정 등록 
							if( pTwitWin->GetRegistType() == cTwitterWindow::eRegister_OnlySave		|| 
								pTwitWin->GetRegistType() == cTwitterWindow::eRegister_LoginAndSave )
							{
								// DB 저장 상태에 따른 연관된 버튼들 활성화
								pTwitRegisterWin->SetResetBtnEnable( true );
								pTwitSheet->SetResetBtnEnable( true );
								pTwitWin->SetDBAccountSave( true );

								LPCSTR pID = pTwitWin->GetID();
								LPCSTR pPW = pTwitWin->GetPW();

								if( pID && strlen(pID) > 0 )
									STAGEMAN->SetTwitterUid( const_cast<LPSTR>(pID) );
								if( pPW && strlen(pPW) > 0 )
									STAGEMAN->SetTwitterPwd( const_cast<LPSTR>(pPW) );

								if( pTwitRegisterWin->IsVisible() )
									pTwitRegisterWin->Hide();

								if( pTwitWin->GetRegistType() == cTwitterWindow::eRegister_OnlySave )
								{
									UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
										eMBEVENT_NONE, eSKIN_OK,
										GAMERESOURCEMAN->GetGameText( 11739 ),	// 알림 메세지
										GAMERESOURCEMAN->GetGameText( 2 )		// 알림
										);
								}

							}
							else/// 초기화 성공
							{
								pTwitRegisterWin->SetResetBtnEnable( false  );
								pTwitSheet->SetResetBtnEnable( false );
								pTwitWin->SetDBAccountSave( false );
						
								char uid[LEN_TWITTER_UID] = {0,};	// 트위터 계정
								char pwd[LEN_TWITTER_PWD] = {0,};	// 트위터 암호

								STAGEMAN->SetTwitterUid( uid );
								STAGEMAN->SetTwitterPwd( pwd );
				
								if( pTwitWin->GetRegistType() == cTwitterWindow::eRegister_Refresh )
								{
									pTwitWin->LogOut();		

									UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
										eMBEVENT_NONE, eSKIN_OK,
										GAMERESOURCEMAN->GetGameText( 11740 ),	// 알림 메세지
										GAMERESOURCEMAN->GetGameText( 2 )		// 알림
										);
								}								
							}						
						}					
					}
				}
				break;
			}
		}
		break;
	case NM_NPC:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_NPC_LIST_RES:
				{
					// 미니맵에 추가 
					cSmallMiniMapWindow* pSmall = mpUI->GetSmallMinimapWindow();
					cMiniMapWindow* pWide = mpUI->GetMinimapWindow();
					if( pSmall && pWide )
					{
						MSG_NPC_INFO* msg = (MSG_NPC_INFO*)pMsg;
						sNpcData* data = msg->mNpcInfo;
						for( unsigned int i = 0; i < msg->mCount; ++i, ++data )
						{
							QUESTMAN->AddNpcStatus( data->mNpcIdx, data->mNpcClassIdx );

							pSmall->AddNpc( data );
							pWide->AddNpc( data );
						}	
					}
					else
					{
						assert(0);
					}
				}
				break;
			case NM_NPC_SIGHT_IN_RES:
				{
					Msg_NpcGenerateInfo( pTemMsg );
				}
				break;
			case NM_NPC_SIGHT_OUT_RES:
				{
					MSG_NPCIDX* msg = (MSG_NPCIDX*)pMsg;
					OBJECTMANAGER->RemoveNpc( msg->mNpcIdx );
				}
				break;
			case NM_NPC_OPEN_RES:
				{
					MSG_RES_NPC_OPEN* pmsg = (MSG_RES_NPC_OPEN*)pMsg;
					if( pmsg->ErrorCode == ERROR_NPC_OPEN_SUCCESS )
					{
						mpUI->OpenNpcUI( pmsg->mNpcIdx );
					}
					else
					{
						/// 거리 오류 - 좌표 수정
						if( ERROR_NPC_OPEN_RANGE == pmsg->ErrorCode )
							HERO->FixPos( pmsg->mPosX, pmsg->mPosY );

						/// 대화중인 npc에 다시 대화요청 했음
						switch( pmsg->ErrorCode )
						{
						case ERROR_NPC_OPEN_ALREADY:
							{
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 431 ),
									GAMERESOURCEMAN->GetGameText( 301 )
									);
							}
							break;
						case ERROR_NPC_OPEN_RANGE:
							{
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 432 ),
									GAMERESOURCEMAN->GetGameText( 301 )
									);
							}
							break;
						default:
							{
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 420 ),
									GAMERESOURCEMAN->GetGameText( 301 )
									);
							}
							break;
						}
					}

					CURSOR->SetCursor( eCURSOR_DEFAULT );
				}
				break;
			case NM_NPC_OPEN_SYN:
				{
					MSG_SYN_NPC_OPEN* pmsg = (MSG_SYN_NPC_OPEN*)pMsg;

					cPlayer* pPlayer = OBJECTMAN->GetPlayer( pmsg->mCharacterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->FixPos( pmsg->mPosX, pmsg->mPosY );

						if( pPlayer->IsTransformMonster() )
							pPlayer->ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
						else
							pPlayer->ChangeAnimation( ANITYPE_IDLE );

						pPlayer->SetStopFlag( eSTOP_NPCSPEECH );
						pPlayer->SetState( eOBJECT_STATE_STOP );	
					}
				}
				break;
			case NM_NPC_ITEM_MOV2_INVENTORY_RES:
				{
					MSG_RES_NPC_ITEM_MOV2_INVENTORY* msgData = (MSG_RES_NPC_ITEM_MOV2_INVENTORY*)pMsg;

					switch( msgData->ErrorCode )
					{
					case ERROR_ITEM_MOV2_INVENTORY_SUCCESS:
						{
							if( msgData->RowCount == 0 || msgData->RowCount == 1 )
								ITEMMAN->NpcNetTryItem( NM_NPC_ITEM_MOV2_INVENTORY_RES );
							else if( msgData->RowCount == 2 )
								ITEMMAN->NpcNetTryItem( NM_NPC_ITEM_MOV2_INVENTORY_RES, msgData );
						}
						break;
					case ERROR_ITEM_MOV2_INVENTORY_MONEY:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 230 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_MOV2_INVENTORY_FAIL:
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 231 ) );
						break;
					case ERROR_ITEM_MOV2_INVENTORY_NOTYET:
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 223 ) );
						break;
					case ERROR_ITEM_MOV2_INVENTORY_NPC:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 420 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_MOV2_INVENTORY_MERGE:
						{
							ITEMMAN->LockChangeToMerge();
						}
						break;
					default:
						assert(0);
						break;
					}
					
					if( ITEMMAN->GetItemTry() == ITEM_TRY_MOVE )
						ITEMMAN->UnlockItemTry( ITEM_TRY_MOVE );
					else if( ITEMMAN->GetItemTry() == ITEM_TRY_SPLIT )
						ITEMMAN->UnlockItemTry( ITEM_TRY_SPLIT );
					else if( ITEMMAN->GetItemTry() == ITEM_TRY_MERGE )
					{
						//스킵
					}
					else
						assert(0);
				}
				break;
			case NM_NPC_ITEM_BUY_RES:
				{
					MSG_RES_NPC_ITEM_BUY* msgData = (MSG_RES_NPC_ITEM_BUY*)pMsg;

					switch( msgData->ErrorCode )
					{
					case ERROR_ITEM_BUY_SUCCESS:
						ITEMMAN->NpcNetTryItem( NM_NPC_ITEM_BUY_RES, msgData );
						break;
					case ERROR_ITEM_BUY_MONEY:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 200 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_BUY_PVP_POINT:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 198 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_BUY_TAROT_POINT:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 197 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_BUY_FAIL:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 232 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_BUY_NOTYET:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 223 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_BUY_NPC:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 420 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_BUY_MAX_ITEM:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 220 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					default:
						assert( 0 && "network error!" );
						break;
					}

					ITEMMAN->UnlockItemTry( ITEM_TRY_BUY );

					cUIWindow* store = (cUIWindow*)mpUI->GetNpcStore();
					if( store )
						store->NetUnLock();
				}
				break;
			case NM_NPC_ITEM_SELL_RES:
				{
					MSG_RES_NPC_ITEM_SELL* msgData = (MSG_RES_NPC_ITEM_SELL*)pMsg;

					switch( msgData->ErrorCode )
					{
					case ERROR_ITEM_SELL_SUCCESS:
						ITEMMAN->NpcNetTryItem( NM_NPC_ITEM_SELL_RES, msgData );
						break;
					case ERROR_ITEM_SELL_FAIL:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 233 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_SELL_NOTYET:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 223 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_SELL_NPC:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 420 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_ITEM_SELL_HAVE_MONEY:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 11204 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
                        break;
                    case ERROR_ITEM_SELL_EXCHANGE:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 2019 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
                        break;
					default:
						assert( 0 && "network error!" );
						break;	
					}

					ITEMMAN->UnlockItemTry( ITEM_TRY_SELL );
				}
				break;
			case NM_NPC_DEPOSIT_RES:
				{
					MSG_RES_NPC_DEPOSIT* msg = (MSG_RES_NPC_DEPOSIT*)pMsg;

					cMoneyWindow* win = mpUI->GetMoneyWindow();

					if( !win )
					{
						assert(0);
						return;
					}

					switch( msg->ErrorCode )
					{
					case ERROR_NPC_DEPOSIT_FAIL:

						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 55 ) );
						break;

					case ERROR_NPC_DEPOSIT_HAVE_MONEY:
						// 입금한 금액 + 창고 보유 금액이 한도(30억) 초과시
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 11202 ),
							GAMERESOURCEMAN->GetGameText( 301 ) );
							
						break;

					case ERROR_NPC_DEPOSIT_TRADE_MONEY:
						// 입금시 한도(10억) 이상 입금하려고 할때
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 11211 ),
							GAMERESOURCEMAN->GetGameText( 301 ) );

						break;

					}
					
					win->NetUnLock( eMONEY_IN );
					
					
				}
				break;
			case NM_NPC_WITHDRAW_RES:
				{
					MSG_RES_NPC_WITHDRAW* msg = (MSG_RES_NPC_WITHDRAW*)pMsg;
					switch( msg->ErrorCode )
					{
					case ERROR_NPC_WITHDRAW_FAIL:
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 56 ) );
						break;

					case ERROR_NPC_WITHDRAW_HAVE_MONEY:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 11203 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;

					case ERROR_NPC_WITHDRAW_TRADE_MONEY:
						// 출금시 한도(10억) 이상 출금하려고 시도할때

						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 11210 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);

						break;
					}
					
					cMoneyWindow* win = mpUI->GetMoneyWindow();
					assert( win );
					if( win )
					{
						win->NetUnLock( eMONEY_OUT );
					}
				}
				break;
			case NM_NPC_QUEST_ADD_RES:
				{
					/// 퀘스트 추가에 대한 정보
					MSG_RES_QUEST_ADD* pmsg = (MSG_RES_QUEST_ADD*)pMsg;

					QUESTMAN->ResQuestAdd( pmsg );
				}
				break;
			case NM_NPC_QUEST_COMPLETE_RES:
				{
					/// 퀘스트 진행 완료 요청에 대한 결과
					MSG_RES_NPC_QUEST_COMPLETE* pmsg = (MSG_RES_NPC_QUEST_COMPLETE*)pMsg;

					QUESTMAN->ResQuestComplete( pmsg );
				}
				break;
			case NM_NPC_QUEST_REWARD_RES:
				{
					/// 퀘스트 보상요청에 관한 결과
					MSG_RES_NPC_QUEST_REWARD* pmsg = (MSG_RES_NPC_QUEST_REWARD*)pMsg;

					QUESTMAN->ResQuestReward( pmsg );
				}
				break;
			case NM_NPC_GUILD_CREATE_RES:
				{
					MSG_RES_NPC_GUILD_CREATE* pmsg = (MSG_RES_NPC_GUILD_CREATE*)pMsg;

					GUILDMAN->ResGuildCreate( pmsg );
				}
				break;
			case NM_NPC_CLOSE_SYN:
				{
					MSG_SYN_NPC_CLOSE* pmsg = (MSG_SYN_NPC_CLOSE*)pMsg;

					cPlayer* pPlayer = OBJECTMAN->GetPlayer( pmsg->mCharacterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->SetStopFlag( eSTOP_NONE );
					}
				}
				break;
			case NM_NPC_CLOSEFORCE_RES:
				{
					/// 데스매치 메세지 윈도우로 인한 강제 종료
					/// 강제 닫기 오면
					HERO->CloseNpcTalk( false );
				}
				break;
			case NM_NPC_TAROT_OPEN_RES:
				{
					MSG_RES_NPC_TAROT_OPEN* pmsg = (MSG_RES_NPC_TAROT_OPEN*)pMsg;
					
					/// 보기 가능한 운세 처리
					cTarotWindow* win = mpUI->GetTarotWindow();
					if( win )
						win->SetFortuneData( pmsg->fortuneData );

					TAROTMAN->ChangePage( ePAGE_NPCVIEW );
				}
				break;
			case NM_NPC_TAROT_JOIN_RES:
				{
					/// 
					MSG_RES_NPC_TAROT_JOIN* pmsg = (MSG_RES_NPC_TAROT_JOIN*)pMsg;
					
					cTarotWindow* win = mpUI->GetTarotWindow();
					if( win )
						win->NetUnLock( ePAGE_NPCVIEW );

					switch( pmsg->ErrorCode )
					{
					case ERROR_NPC_TAROT_JOIN_SUCESS:
						{
							TAROTMAN->ChangePage( ePAGE_NPCSELECT );
						}
						break;
					case ERROR_NPC_TAROT_JOIN_MONEY:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 333 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_NPC_TAROT_JOIN_ONCE:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 332 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					default:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 334 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					}
				}
				break;
			case NM_NPC_TAROT_FORTUNE_RES:
				{
					/// 운세 선택에 관한 결과
					MSG_RES_NPC_TAROT_FORTUNE* pmsg = (MSG_RES_NPC_TAROT_FORTUNE*)pMsg;

					cTarotWindow* win = mpUI->GetTarotWindow();
					if( win )
						win->NetUnLock( ePAGE_SELECT );

					switch( pmsg->ErrorCode )
					{
					case ERROR_NPC_TAROT_FORTUNE_SUCESS:
						{
							TAROTMAN->ChangePage( ePAGE_NPCRESULT );
						}
						break;
					case ERROR_NPC_TAROT_FORTUNE_FAIL:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 334 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					case ERROR_NPC_TAROT_FORTUNE_ONCE:
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 332 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						break;
					}
				}
				break;
			case NM_NPC_THEME_JOIN_RES:
				{
					MSG_ERROR* pResmsg = (MSG_ERROR*)pMsg;
					
					/// 대기중이였다면 대기 창 닫기.
					UIMAN->DestroyMsgBox( eMBEVENT_THEME_ENTER_INFO_WAIT );

					unsigned int errorMsgIndex = 9003;
					switch( pResmsg->ErrorCode )
					{
					case ERROR_NPC_OPEN_RANGE:		 errorMsgIndex = 432;  break;  /// 허용거리 밖
					case ERROR_NPC_OPEN_PLAYERSTATE: errorMsgIndex = 9015; break;  /// 플레이어 상태 오류
					case ERROR_NPC_OPEN_NPC:		 errorMsgIndex = 420;  break;  /// npc 대화 상태가 틀림
					case ERROR_NPC_OPEN_DEALLIST:    errorMsgIndex = 420;  break;  /// npc에 맞지않는 목록
					case ERROR_NPC_OPEN_ALREADY:     errorMsgIndex = 431;  break;  /// 대화중인 npc에 다시 요청 들어옴
					}

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( errorMsgIndex ),
						GAMERESOURCEMAN->GetGameText( 9000 ) );

					/// NPC대화창 닫기
					if( HERO->GetNpcFunc() == NPC_FUNC_THEME )
						HERO->CloseNpcTalk();
				}
				break;
			case NM_NPC_ITEM_AGENT_OPEN_RES:
				{
					MSG_ERROR* pResmsg = (MSG_ERROR*)pMsg;

					unsigned int errorMsgIndex = 1506;
					switch( pResmsg->ErrorCode )
					{
					case ERROR_NPC_OPEN_RANGE:		 errorMsgIndex = 432;  break;  /// 허용거리 밖
					case ERROR_NPC_OPEN_PLAYERSTATE: errorMsgIndex = 1520; break;  /// 플레이어 상태 오류
					case ERROR_NPC_OPEN_NPC:									   /// npc 대화 상태가 틀림
					case ERROR_NPC_OPEN_DEALLIST:    errorMsgIndex = 420;  break;  /// npc에 맞지않는 목록
					case ERROR_NPC_OPEN_ALREADY:     errorMsgIndex = 431;  break;  /// 대화중인 npc에 다시 요청 들어옴
					//case ERROR_NPC_ITEM_AGENT_OPEN_NOTYET: errorMsgIndex = 1506; break;/// DB 처리중
					}

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( errorMsgIndex ),
						GAMERESOURCEMAN->GetGameText( 9000 ) );

					/// NPC대화창 닫기
					if( HERO->GetNpcFunc() == NPC_FUNC_DEAL )
						HERO->CloseNpcTalk();

					cNpcDealWindow* dealWin = GAMEUI->GetNpcDealWindow();
					if( dealWin )
						dealWin->NetUnLock();
				}
				break;
			case NM_NPC_ITEM_AGENT_OPEN_PAGE_RES:
				{
					MSG_RES_NPC_ITEM_AGENT_OPEN* msgData = (MSG_RES_NPC_ITEM_AGENT_OPEN*)pMsg;

					cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
					if( !win )
					{
						assert(0);
						return;
					}

					cNpcDealRegistSheet* regist = win->GetRegistSheet();
					if( !regist )
					{
						assert(0);
						return;
					}

					switch( msgData->ErrorCode )
					{
					case ERROR_NPC_ITEM_AGENT_OPEN_PAGE_SUCCESS:
						{
							regist->Clear();

							// hero가 등록한 아이템 
							NPC_ITEM_AGENT_OPEN_RESULT* data = msgData->Result;
							for( long i = 0; i < msgData->RowCount; ++i, ++data )
							{
								NPC_ITEM_AGENT_OPEN_RESULT* p = new NPC_ITEM_AGENT_OPEN_RESULT;
								*p = *data;

								if( regist->AddItem( p ) == false )
								{
									assert(0);
									SAFE_DELETE( p );
									continue;
								}
							}

							regist->SetTotalPage( msgData->EndPage );
							regist->ChangePage( msgData->StartPage );
						}
						break;
					case ERROR_NPC_ITEM_AGENT_OPEN_PAGE_FAIL:
					case ERROR_NPC_ITEM_AGENT_OPEN_PAGE_NOTYET:
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1506 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;
					}

					cNpcDealWindow* dealWin = GAMEUI->GetNpcDealWindow();
					if( dealWin )
						dealWin->NetUnLock();
				}
				break;
			case NM_NPC_ITEM_AGENT_ADD_RES:
				{
					/// 판매대행 아이템 등록 결과
					MSG_RES_NPC_ITEM_AGENT_ADD* msgData = (MSG_RES_NPC_ITEM_AGENT_ADD*)pMsg;

					UIMAN->DestroyMsgBox( eMBEVENT_NPCDEAL_ITEM_REGIST );

					switch( msgData->ErrorCode )
					{
					case ERROR_NPC_ITEM_AGENT_ADD_SUCCESS:
						{
							ITEMMAN->NpcNetTryItem( NM_NPC_ITEM_AGENT_ADD_RES, pMsg );
						}
						break;
					case ERROR_NPC_ITEM_AGENT_ADD_FAIL:
						{
							/// 실패
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1504 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;
					case ERROR_NPC_ITEM_AGENT_ADD_NOTYET:
						{
							/// DB 처리중
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1506 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;
					case ERROR_NPC_ITEM_AGENT_ADD_OVER:
						{
							/// 등록 개수 초과
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1527 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;

					case ERROR_NPC_ITEM_AGENT_ADD_TRADE_MONEY:
						{
							/// 판매 대행에 최대 거래 금액 이상(10억)으로 설정을 하려고 할때
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 11206 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);

						}
						break;
                    case ERROR_NPC_ITEM_AGENT_ADD_EXCHANGE:
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1516 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);

						}
						break;
					}

					ITEMMAN->UnlockItemTry( ITEM_TRY_NPCDEALREGIST );

					cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();

					if( win )
						win->NetUnLock();
					else
					{
						assert(0);
						return;
					}
				}
				break;
			case NM_NPC_ITEM_AGENT_DEL_RES:
				{
					MSG_RES_NPC_ITEM_AGENT_DEL* msgData = (MSG_RES_NPC_ITEM_AGENT_DEL*)pMsg;

					cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
					if( !win )
					{
						assert(0);
						return;
					}

					win->NetUnLock();

					UIMAN->DestroyMsgBox( eMBEVENT_NPCDEAL_ITEM_DELETE );

					switch( msgData->ErrorCode )
					{
					case ERROR_NPC_ITEM_AGENT_DEL_SUCCESS:
						{
							ITEMMAN->NpcNetTryItem( NM_NPC_ITEM_AGENT_DEL_RES, pMsg );
						}
						break;
					case ERROR_NPC_ITEM_AGENT_DEL_FAIL:
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1514 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;
					case ERROR_NPC_ITEM_AGENT_DEL_NOTYET:
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1515 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;
					}

					ITEMMAN->UnlockItemTry( ITEM_TRY_NPCDEALDELETE );
				}
				break;
			case NM_NPC_ITEM_AGENT_GET_RES:
				{
					MSG_RES_NPC_ITEM_AGENT_GET* msgData = (MSG_RES_NPC_ITEM_AGENT_GET*)pMsg;

					cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
					if( !win )
					{
						assert(0);
						return;
					}

					win->NetUnLock();

					UIMAN->DestroyMsgBox( eMBEVENT_NPCDEAL_ITEM_BUY );

					switch( msgData->ErrorCode )
					{
					case 0:
						{
							ITEMMAN->NpcNetTryItem( NM_NPC_ITEM_AGENT_GET_RES, pMsg );
						}
						break;
					default:
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NPCDEAL_ITEM_GET_FAIL, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 1519 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;
					}

					ITEMMAN->UnlockItemTry( ITEM_TRY_NPCDEALBUY );
					
				}
				break;
			case NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES:
				{
					/// 검색 결과
					MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT* msgData = (MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT*)pMsg;

					cNpcDealWindow* dealWin = GAMEUI->GetNpcDealWindow();
					if( !dealWin )
					{
						assert(0);
						return;
					}

					cNpcDealSearchSheet* searchSheet = dealWin->GetSearchSheet();
					if( !searchSheet )
					{
						assert(0);
						return;
					}

					// 성공
					if( msgData->ErrorCode == ERROR_NPC_ITEM_AGENT_SEARCH_SUCCESS )
					{
						searchSheet->ClearData();

						/// 검색 결과 초기화
						NPC_ITEM_AGENT_SEARCH_RESULT* data = msgData->Result;
						for( long i = 0; i < msgData->RowCount; ++i, ++data )
						{
							NPC_ITEM_AGENT_SEARCH_RESULT* p = new NPC_ITEM_AGENT_SEARCH_RESULT;
							*p = *data;

							if( searchSheet->AddItem( p ) == false )
							{
								assert(0);
								SAFE_DELETE(p);
								continue;
							}
						}

						// 페이지 저장
						searchSheet->SavePage( msgData->StartPage, msgData->EndPage );
					}
					
					// 결과 저장 - 후처리 위함
					searchSheet->SaveResult( msgData->ErrorCode );
					
					dealWin->NetUnLock();
				}
				break;
			case NM_NPC_OBT_EVENT_RES:
				{
					MSG_RES_NPC_OBT_EVENT* msgData = (MSG_RES_NPC_OBT_EVENT*)pMsg;

					/// 메시지 박스 삭제.
					UIMAN->DestroyMsgBox( eMBEVENT_PVP_NPC_REWARD );

					unsigned long msgIndex = 13801;
					switch( msgData->ErrorCode )
					{
					case ERROR_NPC_OBT_EVENT_SUCCESS:	msgIndex = 13800;	break;
					case ERROR_NPC_OBT_EVENT_FAIL:		msgIndex = 13801;	break;// 실패
					case ERROR_NPC_OBT_EVENT_NPC:		msgIndex = 420;		break;// 실패 - NPC인증 
					case ERROR_NPC_OBT_EVENT_COMPLETE:	msgIndex = 13802;	break;// 실패 - 이미 모두 완료
					case ERROR_NPC_OBT_EVENT_NO_USER:	msgIndex = 13803;	break;// 실패 - 받을 아이템이 없음
					default: assert(0); break;
					}

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( msgIndex ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);

					ITEMMAN->UnlockItemTry( ITEM_TRY_NPCEVENT );
				}
				break;
			case NM_NPC_EVENT_ITEM_GIVE_RES:
				{
					MSG_RES_NPC_EVENT_ITEM_GIVE* msgData = (MSG_RES_NPC_EVENT_ITEM_GIVE*)pMsg;

					/// 메시지 박스 삭제.
					UIMAN->DestroyMsgBox( eMBEVENT_PVP_NPC_REWARD );
					
					unsigned long msgIndex = 13801;
					switch( msgData->ErrorCode )
					{
					case ERROR_NPC_EVENT_ITEM_GIVE_SUCCESS:		msgIndex = 13800;	break;
					case ERROR_NPC_EVENT_ITEM_GIVE_FAIL:		msgIndex = 13801;	break;// 실패
					case ERROR_NPC_EVENT_ITEM_GIVE_NPC:			msgIndex = 420;		break;// 실패 - NPC인증 
					case ERROR_NPC_EVENT_ITEM_GIVE_COMPLETE:	msgIndex = 13802;	break;// 실패 - 이미 모두 완료
					case ERROR_NPC_EVENT_ITEM_GIVE_NO_USER:		msgIndex = 13803;	break;// 실패 - 받을 아이템이 없음
					default: assert(0); break;
					}

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( msgIndex ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);

					ITEMMAN->UnlockItemTry( ITEM_TRY_NPCEVENT );
				}
				break;
			}
		}break;
	case NM_MONSTER:
		{
			switch( pTemMsg->Protocol )
			{
				/// 061122 PKH 몬스터 생성 되어있는지 정보만 클라이언트 로딩시점에 받아서 처리한다.
			case NM_MONSTER_SIGHT_IN_RES:
				{
					Msg_MonsterGenerateInfo( pTemMsg );	
				}
				break;
			case NM_MONSTER_REGEN_SYN:
				{
					MSG_MONSTER_INFO* msg = (MSG_MONSTER_INFO*)pMsg;
					OBJECTMANAGER->AddMonster( &msg->mMonsterInfo, true );

				}break;
			case NM_MONSTER_TEAMSIGHT_IN_RES:
				{
					Msg_MonsterGenerateTeamInfo( pTemMsg );	
				}
				break;
			case NM_MONSTER_TEAMREGEN_SYN:
				{
					MSG_MONSTERTEAM_INFO* msg = (MSG_MONSTERTEAM_INFO*)pMsg;
					OBJECTMANAGER->AddMonster( &msg->mMonsterInfo, true );

					cMonster* pMonster = OBJECTMAN->GetMonster( msg->mMonsterInfo.mMonsterIdx );
					if( pMonster )
					{
						pMonster->SetPvPTeamType( msg->mTeamType );
					}

				}break;
			case NM_MONSTER_MOVE_SYN:
				{
					MSG_SYN_MOVE_MONSTER* pMessage = (MSG_SYN_MOVE_MONSTER*)pMsg;

					OBJECTMANAGER->MonsterGoto( pMessage->mMonsterIdx, pMessage->mPathArray, 
						pMessage->mCount, pMessage->mMoveSpeed, (eMOVE_SPEED_TYPE)pMessage->mMoveSpeedType );
				}
				break;
			case NM_MONSTER_ACTIONMOVE_SYN:
				{
					MSG_SYN_ACTIONMOVE_MONSTER* pMessage = (MSG_SYN_ACTIONMOVE_MONSTER*)pMsg;

					OBJECTMANAGER->MonsterGoto( pMessage->mMonsterIdx, pMessage->mPathArray, 
						pMessage->mCount, pMessage->mMoveSpeed, eMOVESPEED_RUN,
						pMessage->mRange, &pMessage->mTarget );
				}
				break;

			case NM_MONSTER_HP_SYN:
				{
					MSG_SYN_MONSTER_HP* msg = (MSG_SYN_MONSTER_HP*)pMsg;
					cMonster* pMonster = OBJECTMANAGER->GetMonster( msg->mMonsterIdx );
					if( pMonster != NULL )
					{
						if( msg->mDie == true )
							pMonster->HPChangeDie( msg->mMaxHP, 0, eDAMAGE_NORMAL );
						else
							pMonster->HPChange( msg->mHP, msg->mMaxHP );
					}
				}
				break;
			case NM_MONSTER_SIGHT_OUT_RES:
				{
					MSG_MONSTER_SIGHTOUT* msg = (MSG_MONSTER_SIGHTOUT*)pMsg;
					cMonster* pMonster = OBJECTMANAGER->GetMonster( msg->mMonsterIdx );
					if( pMonster == NULL )
					{
						assert(NULL);
						return;
					}

					OBJECTMANAGER->RemoveMonster( msg->mMonsterIdx );

				}break;
			case NM_MONSTER_MOVESPEED_SYN:
				{
					MSG_SYN_MONSTER_MOVESPEED* pSynMsg = (MSG_SYN_MONSTER_MOVESPEED*)pMsg;
					cMonster* pMonster = OBJECTMANAGER->GetMonster( pSynMsg->mMonsterIdx );
					if( pMonster != NULL )
					{
						pMonster->SetMoveSpeed( pSynMsg->mMoveSpeed );
					}
				}
				break;
			case NM_MONSTER_SCALE_SYN:
				{
					MSG_SYN_MONSTER_SCALE* pSynMsg = (MSG_SYN_MONSTER_SCALE*)pMsg;
					cMonster* pMonster = OBJECTMANAGER->GetMonster( pSynMsg->mMonsterIdx );
					if( pMonster != NULL )
					{
						pMonster->SetFixedObjectSizeScale( pSynMsg->mScale, false );
					}
				}
				break;
			case NM_MONSTER_MOVESTOP_SYN:
				{
					MSG_SYN_MONSTER_MOVESTOP* pSynMsg = (MSG_SYN_MONSTER_MOVESTOP*)pMsg;
					cMonster* pMonster = OBJECTMANAGER->GetMonster( pSynMsg->mMonsterIdx );
					if( pMonster != NULL )
					{
						pMonster->MoveStop( pSynMsg->mMonsterPos.x, pSynMsg->mMonsterPos.y );						
					}
				}
				break;
			case NM_MONSTER_MODECHANGE_SYN:
				{
					MSG_SYN_MONSTER_MODECHANGE* pSynMsg = (MSG_SYN_MONSTER_MODECHANGE*)pMsg;
					cMonster* pMonster = OBJECTMANAGER->GetMonster( pSynMsg->mMonsterIdx );
					if( pMonster != NULL )
					{
						pMonster->ChangeMonsterMode( pSynMsg->mModeIdx );
					}
				}
				break;
			case NM_MONSTER_IDLE2_ANI_SYN:
				{
					MSG_SYN_MONSTER_IDLE2ANI* pSynMsg = (MSG_SYN_MONSTER_IDLE2ANI*)pMsg;
					cMonster* pMonster = OBJECTMANAGER->GetMonster( pSynMsg->mMonsterIdx );
					if( pMonster != NULL )
					{
						pMonster->PlayIdle2Ani();					
					}
				}
				break;
			case NM_MONSTER_SPEECH_SYN:
				{
					MSG_SYN_MONSTER_SPEECH* pSynMsg = (MSG_SYN_MONSTER_SPEECH*)pMsg;
					cMonster* pMonster = OBJECTMAN->GetMonster( pSynMsg->mMonsterIdx );
					if( pMonster )
					{
						cBaseObject* target = OBJECTMAN->GetPlayer( pSynMsg->mCharacterIdx );

						sTextItem* text = GAMERESOURCEMAN->GetMonsterTalk( pSynMsg->mTalkIdx, pMonster, target );
						pMonster->ActiveChatBubble( text );

						if( pSynMsg->mViewType )
						{
							cStringT str;
							str += pMonster->GetName();
							str += _T(" :");

							sTextItem* check = text;
							while( check )
							{
								str += check->mText;
								check = check->mpNext;
							}

							if( pSynMsg->mViewType == eM_TALKVIEW_NORMAL )
							{
								CHATMANAGER->AddChatMsg( str.Cstr(), eCHAT_NORMAL );
							}
							else if( pSynMsg->mViewType == eM_TALKVIEW_SHOUT )
							{
								CHATMANAGER->AddChatMsg( str.Cstr(), eCHAT_SHOUT );
							}
						}
					}
				}
				break;
			case NM_MONSTER_DESTROY_SYN:
				{
					MSG_SYN_MONSTER_DESTROY* pSynMsg = (MSG_SYN_MONSTER_DESTROY*)pMsg;

					cMonster* monster = OBJECTMAN->GetMonster( pSynMsg->mMonsterIdx );
					if( monster )
					{
						monster->SetState( eOBJECT_STATE_IDLE );
						monster->SetState( eOBJECT_STATE_DIE, false );
					}
				}
				break;
			}
		}break;
	case NM_GATHERING:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_GATHERING_REGEN_SYN:
			case NM_GATHERING_SIGHT_IN_RES:
				{
					MSG_GATHERING_INFO* msg = (MSG_GATHERING_INFO*)pMsg;

					if( OBJECTMAN->GetGatherSceneNode( msg->mObjectIdx ) == 0 )
						OBJECTMAN->AddGathering( msg->mObjectIdx, msg->mGatheringIdx, NiPoint3( msg->mPosX, msg->mPosY, msg->mAppendZ), msg->mDirection );
				}
				break;
			case NM_GATHERING_DEL_SYN:
			case NM_GATHERING_SIGHT_OUT_RES:
				{
					MSG_GATHERINGIDX* msg = (MSG_GATHERINGIDX*)pMsg;
					OBJECTMAN->RemoveGathering( msg->mObjectIdx );
				}
				break;
			case NM_GATHERING_START_RES:
				{
					MSG_ERROR* msg = (MSG_ERROR*)pMsg;
					if( msg->ErrorCode == ERROR_GATHERING_SUCCESS )
					{
						/// 성공
					}
					else
					{
						switch( msg->ErrorCode )
						{
						case ERROR_GATHERING_FAIL: CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 601 ) );break;
						case ERROR_GATHERING_LEVEL:CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 602 ) );break;
						case ERROR_GATHERING_ITEM:CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 603 ) );break;
						case ERROR_GATHERING_QUEST:CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 604 ) );break;
						case ERROR_GATHERING_MAKESKILL:CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 601 ) );break;
						default:break;
						}

						HERO->GatheringEnd();
					}
				}
				break;
			case NM_GATHERING_CANCEL_SUCCESS_RES:
				{
					/// 
				}
				break;
			case NM_GATHERING_START_SYN:
				{
					MSG_SYN_GATHERING_START* msg = (MSG_SYN_GATHERING_START*)pMsg;
					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIdx );
					if( player )
					{
						NiPoint3 pos = OBJECTMAN->GetGatherPos( msg->mObjectIdx );
						player->GatheringStart( msg->mObjectIdx, msg->mClassIdx, pos );
					}
				}
				break;
			case NM_GATHERING_CANCEL_SYN:
				{
					MSG_SYN_GATHERING_CANCEL* msg = (MSG_SYN_GATHERING_CANCEL*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIdx );
					if( player && (player->GetState() == ePLAYER_STATE_GATHERING) )
					{
						if( HERO == player )
						{
							HERO->GatheringEnd();
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 607 ) );
						}
						else
						{
							player->GatheringEnd();
						}
					}
				}
				break;
			case NM_GATHERING_END_SUCCESS_SYN:
				{
					MSG_SYN_GATHERING_END* msg = (MSG_SYN_GATHERING_END*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIdx );
					if( player && (player->GetState() == ePLAYER_STATE_GATHERING) )
						player->GatheringEnd();
				}
				break;
			case NM_GATHERING_END_FAILE_SYN:
				{
					MSG_SYN_GATHERING_END* msg = (MSG_SYN_GATHERING_END*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIdx );
					if( player && (player->GetState() == ePLAYER_STATE_GATHERING) )
						player->GatheringEnd();

					if( HERO == player )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 608 ) );
					}
				}
				break;
			case NM_GATHERING_REGEN_HOIST_SYN:
			case NM_GATHERING_HOIST_SIGHT_IN_RES:
				{
					// 게양 포인트 리젠
					MSG_GATHERING_HOIST_INFO* msg = (MSG_GATHERING_HOIST_INFO*)pMsg;

					if( PVPMAN->IsEnableBattle() == false )
					{
						assert(0);
						return;
					}
					
					if( OBJECTMAN->GetGatherSceneNode( msg->mObjectIdx ) == 0 )
						OBJECTMAN->AddGathering( msg->mObjectIdx, msg->mGatheringIdx, NiPoint3( msg->mPosX, msg->mPosY, msg->mAppendZ), msg->mDirection, msg->mTeamType );
				}
				break;
			}
		}break;
	case NM_PLAYER:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_PLAYER_LIST_RES:
				{
					MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsg;

					/// 070626 PKH 플레이어 리스트 로드 성공
					if( pmsg->ErrorCode == 0 )
					{
						OBJECTMANAGER->AddPlayerFromServerMsg( pTemMsg );
					}
					/// 070626 PKH 플레이어 리스트 로드 실패
					else 
					{
						assert(NULL);
						return;
					}

				}
				break;
			case NM_PLAYER_GAMEIN_SYN:
				{
					OBJECTMANAGER->AddPlayerFromServerMsg( pTemMsg, true );
				}
				break;
			case NM_PLAYER_GAMEOUT_RES:
				{
					MSG_ERROR* pmsg = (MSG_ERROR*)pMsg;
					if( pmsg->ErrorCode == 0 )
					{
						mpStageManager->ChangeStage( eStage_Select );

						/// 채팅창 클리어 
						CHATMANAGER->ClearChat();

						cTwitterWindow* pTwitWin = mpUI->GetTwitterWindow();
						if( pTwitWin )
						{
							pTwitWin->LogOut();
						}
						else
						{
							assert(0);
						}

					}
					else
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 58 ) );
						HERO->SetState( eOBJECT_STATE_IDLE );
					}

					cExitWindow* win = mpUI->GetExitWindow();
					if( win )
						win->NetUnLock();

					cUIWindow* menuOption = mpUI->GetMenuOptionWindow();
					if( menuOption )
						menuOption->NetUnLock();
				}
				break;
			case NM_PLAYER_DISCONNECT_RES:
				{
					MSG_ERROR* pmsg = (MSG_ERROR*)pMsg;

					cExitWindow* win = mpUI->GetExitWindow();
					if( win )
						win->NetUnLock();

					cUIWindow* menuOption = mpUI->GetMenuOptionWindow();
					if( menuOption )
						menuOption->NetUnLock();

					if( pmsg->ErrorCode == 0 )
					{
						/// 프로그램 종료
						::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
						::PostQuitMessage(0);
					}
					else
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 58 ) );
						HERO->SetState( eOBJECT_STATE_IDLE );
					}
				}
				break;
			case NM_PLAYER_DISCONNECT_SYN:
				{
					MSG_CHARACTERIDX* pmsg = (MSG_CHARACTERIDX*)pMsg;
					OBJECTMANAGER->RemovePlayer( pmsg->mCharacterIdx );
				}
				break;

			case NM_PLAYER_GAMEFINISH_RES:
				{
					MSG_ERROR* pmsg = (MSG_ERROR*)pMsg;
					if( pmsg->ErrorCode == 0 )
					{
						cExitMsgWindow* win = mpUI->GetExitMsgWindow();
						if( win )
						{
							win->NetUnLock();
							win->StartCount();
						}
						HERO->SetStopFlag( eSTOP_GAMEFINISH );
										
					}
					else
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 58 ) );
						cExitMsgWindow* win = mpUI->GetExitMsgWindow();
						if( win )
						{
							win->NetUnLock();
							win->Hide(false);
						}
					}
				}
				break;
			case NM_PLAYER_KEYMOVE_RES:
				{
//					assert(0);

					MSG_RES_KEYMOVE* pmsg = (MSG_RES_KEYMOVE*)pMsg;
					if( HERO )
					{
						NiPoint3 pos = NiPoint3( pmsg->mCharacterPosX, pmsg->mCharacterPosY, 0.0f );
						if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
						{
							HERO->SetPos_ResetCam( pos );
						}
						
						if( !(HERO->GetState() == eOBJECT_STATE_DIE || HERO->GetState() == eOBJECT_STATE_STOP) )
							HERO->SetState( eOBJECT_STATE_IDLE );
					}
				}
				break;
			case NM_PLAYER_KEYMOVE_STOP_RES:
			case NM_PLAYER_FINDPOSITIONMOVE_STOP_RES:
			case NM_PLAYER_FOLLOWMOVE_STOP_RES:
			case NM_PLAYER_ACTIONMOVE_STOP_RES:
				{
					assert(0);

					MSG_RES_MOVE_STOP* pmsg = (MSG_RES_MOVE_STOP*)pMsg;
					if( HERO )
					{
						NiPoint3 pos = NiPoint3( pmsg->mCharacterPosX, pmsg->mCharacterPosY, 0.0f );
						if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
						{
							HERO->SetPos_ResetCam( pos );
						}

						if( !(HERO->GetState() == eOBJECT_STATE_DIE || HERO->GetState() == eOBJECT_STATE_STOP) )
							HERO->SetState( eOBJECT_STATE_IDLE );
					}
				}
				break;
			case NM_PLAYER_KEYMOVE_STOP_SYN:
			case NM_PLAYER_FINDPOSITIONMOVE_STOP_SYN:
			case NM_PLAYER_FOLLOWMOVE_STOP_SYN:
			case NM_PLAYER_ACTIONMOVE_STOP_SYN:
			case NM_PLAYER_MOVE_STOP_SYN:
				{
					MSG_SYN_MOVE_STOP* pmsg = (MSG_SYN_MOVE_STOP*)pMsg;

					cPlayer* player = OBJECTMANAGER->GetPlayer( pmsg->mCharacterIdx );
					if( player )
					{
						player->FixPos( pmsg->mCharacterPosX, pmsg->mCharacterPosY );
						
						if( !(player->GetState() == eOBJECT_STATE_DIE || player->GetState() == eOBJECT_STATE_STOP) )
							player->SetState( eOBJECT_STATE_IDLE );
					}
				}
				break;

			case NM_PLAYER_TEST_SERVERPOS:
				{
					MSG_SERVERPOS_TEST* pmsg = (MSG_SERVERPOS_TEST*)pMsg;

					cSceneNode* node = OBJECTMANAGER->GetGatherSceneNode( pmsg->mCharacterIdx );
					if( node == 0 )
					{
						OBJECTMANAGER->AddGathering( pmsg->mCharacterIdx, 10016, NiPoint3( pmsg->mServerPosX, pmsg->mServerPosY, 0.0f), 0.0f );
					}
					else
					{
						NiPoint3 pos = NiPoint3( pmsg->mServerPosX, pmsg->mServerPosY, 0.0f );
						if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
						{
							node->SetTranslate( pos );
						}
					}
				}
				break;

			case NM_PLAYER_MOVE_SYN:
				{
					MSG_SYN_MOVE* pmsg = (MSG_SYN_MOVE*)pMsg;
					OBJECTMANAGER->MovePlayer( pmsg->characterIdx, pmsg->destX, pmsg->destY, pmsg->moveSpeed );
				}
				break;
			case NM_PLAYER_MOVE_RES:
			case NM_PLAYER_ACTIONMOVE_RES:	/// 이패킷은 이동 불가능 일때만 보내줌
				{
					/// 070625 PKH 원래 좌표로 옮김
					MSG_RES_ERR_MOVE* pmsg = (MSG_RES_ERR_MOVE*)pMsg;
					NiPoint3 pos = NiPoint3( pmsg->destX, pmsg->destY, 0.0f );
					if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
					{
						HERO->SetWorldPos( pos );
						HERO->SetPos_ResetCam( pos );

						bool isFindPos = false;
						if( HERO->IsActiveFindPositon() == true )
							isFindPos = true;

						if( !(HERO->GetState() == eOBJECT_STATE_DIE || HERO->GetState() == eOBJECT_STATE_STOP) )
							HERO->SetState( eOBJECT_STATE_IDLE );

						if( isFindPos == true )
						{
							if( HERO->ReActivePosition() == false )
								CHATMANAGER->AddSystemMsg(eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 73 ) );
						}
						else
							CHATMANAGER->AddSystemMsg(eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 73 ) );						
					}
				}
				break;
			case NM_PLAYER_ACTIONMOVE_SYN:
				{
					OBJECTMANAGER->ActionMovePlayer( pTemMsg );
				}
				break;
			case NM_PLAYER_MOVESPEED_SYN:
				{
					MSG_SYN_MOVESPEED* pSynMsg = (MSG_SYN_MOVESPEED*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharacterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->SetMoveSpeed( pSynMsg->moveSpeed );
					}
				}
				break;
			case NM_PLAYER_ATTACKSPEED_SYN:
				{
					MSG_SYN_ATTACKSPEED* pSynMsg = (MSG_SYN_ATTACKSPEED*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharacterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->SetAttackSpeed( pSynMsg->mAttackSpeed );
					}
				}
				break;
			case NM_PLAYER_CANTMOVE_SYN:
				{
					MSG_SYN_PLAYER_CANTMOVE* pSynMsg = (MSG_SYN_PLAYER_CANTMOVE*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharaterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->FixPos( pSynMsg->mCharacterPos.x, pSynMsg->mCharacterPos.y );
						pPlayer->SetCantMove( pSynMsg->mIsCantMove );
						if( pPlayer->GetState() == eOBJECT_STATE_MOVE )
							pPlayer->SetState( eOBJECT_STATE_IDLE );
						/// 따라가기 종료
						if( pSynMsg->mCharaterIdx == HERO->GetObjectID() && HERO->IsFollowing() )
							HERO->ReleaseFollowPlayer();
					}
				}
				break;
			case NM_PLAYER_SLEEP_SYN:
				{
					MSG_SYN_PLAYER_SLEEP* pSynMsg = (MSG_SYN_PLAYER_SLEEP*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharaterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->FixPos( pSynMsg->mCharacterPos.x, pSynMsg->mCharacterPos.y );
						pPlayer->SetSleep( pSynMsg->mIsSleep );
						if( pPlayer->GetState() == eOBJECT_STATE_MOVE )
							pPlayer->SetState( eOBJECT_STATE_IDLE );
						/// 따라가기 종료
						if( pSynMsg->mCharaterIdx == HERO->GetObjectID() && HERO->IsFollowing() )
							HERO->ReleaseFollowPlayer();
					}
				}
				break;
			case NM_PLAYER_STUN_SYN:
				{
					MSG_SYN_PLAYER_STUN* pSynMsg = (MSG_SYN_PLAYER_STUN*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharaterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->FixPos( pSynMsg->mCharacterPos.x, pSynMsg->mCharacterPos.y );
						pPlayer->SetStun( pSynMsg->mIsStun );
						if( pPlayer->GetState() == eOBJECT_STATE_MOVE )
							pPlayer->SetState( eOBJECT_STATE_IDLE );
						/// 따라가기 종료
						if( pSynMsg->mCharaterIdx == HERO->GetObjectID() && HERO->IsFollowing() )
							HERO->ReleaseFollowPlayer();
						
					}
				}
				break;
			case NM_PLAYER_CANTSKILL_SYN:
				{
					MSG_SYN_PLAYER_CANTSKILL* pSynMsg = (MSG_SYN_PLAYER_CANTSKILL*)pMsg;
					if( HERO != NULL )
						HERO->SetCantSkill( pSynMsg->mIsCantSkill, pSynMsg->mAttributeType );
				}
				break;
			case NM_PLAYER_SCALE_SYN:
				{
					MSG_SYN_PLAYER_SIZE* pSynMsg = (MSG_SYN_PLAYER_SIZE*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharacterIdx );
					if( pPlayer != NULL )
					{
						pPlayer->SetFixedObjectSizeScale( pSynMsg->mScale, false );
					}
				}
				break;
			case NM_PLAYER_CHG_MONSTER_SYN:
				{
					MSG_SYN_PLAYER_CHG_MONSTER* pSynMsg = (MSG_SYN_PLAYER_CHG_MONSTER*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharacterIdx );
					if( pPlayer == 0 )
						return;

					/// target effect change
					bool reTarget = false;
					if( HERO->GetTargetObject() == pPlayer )
					{
						reTarget = true;
						HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
					}

					if( pSynMsg->mMonsterIdx == 0 )
					{
						if( pPlayer->GetState() == eOBJECT_STATE_ATTACK || pPlayer->IsReadyUseSkill() == false || pPlayer->IsJumping() == true )
							pPlayer->ReadyTransform( eReady_RestoreMonster, 0 );
						else
							pPlayer->RestoreMonster();
					}
					else
					{
						if( pPlayer->IsRideVehicle() == true )
						{
							pPlayer->AlightVehicle();
						}

						if( pPlayer->GetState() == eOBJECT_STATE_ATTACK || pPlayer->IsReadyUseSkill() == false || pPlayer->IsJumping() == true )
							pPlayer->ReadyTransform( eReady_TransformMonster, pSynMsg->mMonsterIdx );
						else
							pPlayer->TransformMonster( pSynMsg->mMonsterIdx );
					}

					/// target effect change
					if( reTarget )
					{
						HERO->SetTargetObject( pPlayer->GetObjectType(), pPlayer->GetObjectID() );
					}
				}
				break;
			case NM_PLAYER_VEHICLE_SYN:
				{
					MSG_SYN_PLAYER_VEHICLE* pSynMsg = (MSG_SYN_PLAYER_VEHICLE*)pMsg;
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mCharacterIdx );
					if( pPlayer == 0 )
						return;

					/// target effect change
					bool reTarget = false;
					if( HERO->GetTargetObject() == pPlayer )
					{
						reTarget = true;
						HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
					}

					if( pSynMsg->mVehicleIdx == 0 )
					{
						pPlayer->AlightVehicle();
					}
					else
					{
						pPlayer->RideVehicle( pSynMsg->mVehicleIdx );

						NiTransform trans;
						trans.MakeIdentity();
						pPlayer->SetLinkdEffect( eLINK_FOOT, "/Data/Effect/eff_pc_riding_02.nif", &trans );
						pPlayer->SetLinkdSound( 2036 );
					}

					//
					if( pPlayer == HERO )
					{
						if( ITEMMAN->GetItemTry() == ITEM_TRY_VEHICLE )
							ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
					}

					/// target effect change
					if( reTarget )
					{
						HERO->SetTargetObject( pPlayer->GetObjectType(), pPlayer->GetObjectID() );
					}
				}
				break;
			case NM_PLAYER_EQUIP_WEAR_RES:
				{
					MSG_RES_PLAYER_EQUIP_WEAR* pmsg = (MSG_RES_PLAYER_EQUIP_WEAR*)pMsg;

					HERO->EquipWearInfo( (eWEAR_TYPE)pmsg->mType, pmsg->mWearInfo );
					HERO->CalcStatus();
				}
				break;
			case NM_PLAYER_EQUIP_WEAR_SYN:
				{
					MSG_SYN_PLAYER_EQUIP_WEAR* pmsg = (MSG_SYN_PLAYER_EQUIP_WEAR*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pmsg->mCharacterIdx );
					if( pPlayer )
						pPlayer->EquipWearInfo( (eWEAR_TYPE)pmsg->mType, pmsg->mWearInfo );
				}
				break;
			case NM_PLAYER_COSTUME_WEAR_RES:
				{
					MSG_RES_PLAYER_COSTUME_WEAR* pmsg = (MSG_RES_PLAYER_COSTUME_WEAR*)pMsg;

					HERO->EquipCostumeInfo( pmsg->mType, pmsg->mWearInfo );
					HERO->CalcStatus();
				}
				break;
			case NM_PLAYER_COSTUME_WEAR_SYN:
				{
					MSG_SYN_PLAYER_COSTUME_WEAR* pmsg = (MSG_SYN_PLAYER_COSTUME_WEAR*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pmsg->mCharacterIdx );
					if( pPlayer )
						pPlayer->EquipCostumeInfo( pmsg->mType, pmsg->mWearInfo );
				}
				break;
			case NM_PLAYER_COSTUME_ACC_RES:
				{
					MSG_RES_PLAYER_COSTUME_ACC* pmsg = (MSG_RES_PLAYER_COSTUME_ACC*)pMsg;

					HERO->EquipCostumeAcceInfo( pmsg->mType, pmsg->mWearInfo );
					HERO->CalcStatus();
				}
				break;
			case NM_PLAYER_COSTUME_ACC_SYN:
				{
					MSG_SYN_PLAYER_COSTUME_ACC* pmsg = (MSG_SYN_PLAYER_COSTUME_ACC*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pmsg->mCharacterIdx );
					if( pPlayer )
						pPlayer->EquipCostumeAcceInfo( pmsg->mType, pmsg->mWearInfo );
				}
				break;
			case NM_PLAYER_EQUIP_WEAPON_RES:
				{
					MSG_RES_PLAYER_EQUIP_WEAPON* pmsg = (MSG_RES_PLAYER_EQUIP_WEAPON*)pMsg;

					HERO->EquipWeaponInfo( (eWEAPON_STATE)pmsg->mWeaponState, &pmsg->mWeaponInfo );

					//
					QUICKMAN->UpdateTutorial( ITEMMAN->GetActiveWeapon() );

					/// 퀵슬롯 세팅
					QUICKMAN->ChangeTabFS( ITEMMAN->GetActiveWeapon() );

					/// 퀵슬롯 스킬 검사
					QUICKMAN->CheckSkillIcon();
				}
				break;
			case NM_PLAYER_EQUIP_WEAPON_SYN:
				{
					MSG_SYN_PLAYER_EQUIP_WEAPON* pmsg = (MSG_SYN_PLAYER_EQUIP_WEAPON*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pmsg->mCharacterIdx );
					if( pPlayer )
						pPlayer->EquipWeaponInfo( (eWEAPON_STATE)pmsg->mWeaponState, &pmsg->mWeaponInfo );
				}
				break;
			case NM_PLAYER_MAPCHANGE_RES:
				{
					/// 맵이동 성공/실패 처리
					MSG_ERROR* pmsg = (MSG_ERROR*)pMsg;

					HERO->SetStopFlag( eSTOP_NONE );

					if( pmsg->ErrorCode == ERROR_RES_MAPCHANGE_SUCCESS )
					{
						Msg_MapChangeAck( (MSGROOT*)pMsg );
					}
					else
					{
						UIMAN->DestroyMsgBox( eMBEVENT_MAPCHANGEWAIT );

						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 98 ) );
					}
				}
				break;
			case NM_PLAYER_MONEY_RES:
				{
					MSG_RES_PLAYER_MONEY* msg = (MSG_RES_PLAYER_MONEY*)pMsg;

					/// 잘못된 데이터가 오는지 검사
					HERO->SetMoney( msg->mMoney );

					/// 얻은 금액 출력
					cStringT str;

					LPCTSTR addMoney = 0;
					if( msg->mAddMoney < 0 )
					{
						addMoney = cNumberEditBox::FormatNumber( (-msg->mAddMoney) );
						str.Format( GAMERESOURCEMAN->GetGameText( 54 ), addMoney );
					}
					else if( msg->mAddMoney > 0 )
					{
						addMoney = cNumberEditBox::FormatNumber( msg->mAddMoney );
						str.Format( GAMERESOURCEMAN->GetGameText( 53 ), addMoney );

						if( msg->mObjectType == eOBJECTTYPE_MONSTER || msg->mObjectType == eOBJECTTYPE_GATHERING )
						{
							NiTransform trans;
							trans.MakeIdentity();
							trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );

							HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_get_02.nif", &trans, false, true );
						}
					}
					else
					{
						assert(0);
						return;
					}

					CHATMANAGER->AddSystemMsg( eSYSTEM_GETMONEY, (LPCTSTR)str.Cstr() );

					/// 인벤 정보창에 셋팅
					cInventoryWindow* pWin = mpUI->GetInventoryWindow();
					if( pWin )
						pWin->SetMoney( msg->mMoney );
				}
				break;
			case NM_PLAYER_DEPOSIT_RES:
				{
					MSG_RES_PLAYER_DEPOSIT* msg = (MSG_RES_PLAYER_DEPOSIT*)pMsg;

					HERO->SetDeposit( msg->mDeposit );

					/// 창고 세팅
					cWareHouseWindow* pWin = mpUI->GetWareHouseWindow();
					if( pWin )
						pWin->SetMoney( msg->mDeposit );

				}
				break;
			case NM_PLAYER_TAROT_POINT_RES:
				{
					MSG_RES_PLAYER_TAROT_POINT* msg = (MSG_RES_PLAYER_TAROT_POINT*)pMsg;

					HERO->SetTarotPoint( msg->mPoint );

					cStringT str;
					LPCTSTR pointStr;
					if( msg->mAddPoint > 0 )
					{
						pointStr = cNumberEditBox::FormatNumber( msg->mAddPoint );
						str.Format( GAMERESOURCEMAN->GetGameText( 150 ), pointStr );
					}
					else if( msg->mAddPoint < 0 )
					{
						pointStr = cNumberEditBox::FormatNumber( (-msg->mAddPoint) );
						str.Format( GAMERESOURCEMAN->GetGameText( 151 ), pointStr );
					}
					else
					{
						assert(0);
						return;
					}

					CHATMANAGER->AddSystemMsg( eSYSTEM_GETMONEY, (LPCTSTR)str.Cstr() );
				}
				break;
			case NM_PLAYER_PVP_POINT_RES:
				{
					MSG_RES_PLAYER_PVP_POINT* msg = (MSG_RES_PLAYER_PVP_POINT*)pMsg;

					HERO->SetPVPPoint( msg->mPoint, msg->mForceType );

					cSocialWindow* pWin = mpUI->GetSocialWindow();
					if( pWin != NULL )
						pWin->UpdatePoint();

					cInventoryWindow* pInvenWin = mpUI->GetInventoryWindow();
					if( pInvenWin != NULL )
						pInvenWin->UpdateForcePoint();

					unsigned long typeIndex = 0;
					switch( msg->mForceType )
					{
					case eFORCETYPE_FIRE:	typeIndex = 124;	break;
					case eFORCETYPE_WATER:	typeIndex = 125;	break;
					case eFORCETYPE_WIND:	typeIndex = 126;	break;
					case eFORCETYPE_EARTH:	typeIndex = 127;	break;
					default:	assert(0);	return;
					}

					cStringT str;
					LPCTSTR pointStr;

					if( msg->mAddPoint > 0 )
					{
						pointStr = cNumberEditBox::FormatNumber( msg->mAddPoint );
						str.Format( GAMERESOURCEMAN->GetGameText( 152 ), pointStr, UIMAN->GetUIText( typeIndex ) );
					}
					else if( msg->mAddPoint < 0 )
					{
						pointStr = cNumberEditBox::FormatNumber( (-msg->mAddPoint) );
						str.Format( GAMERESOURCEMAN->GetGameText( 153 ), pointStr, UIMAN->GetUIText( typeIndex ) );
					}
					else
					{
						assert(0);
						return;
					}

					CHATMANAGER->AddSystemMsg( eSYSTEM_GETMONEY, (LPCTSTR)str.Cstr() );
				}
				break;
			case NM_PLAYER_HP_SYN:
				{
//#ifdef _GMTOOL
//					if( CHEATMAN->GetUndeadMode() )
//						break;
//#endif
					MSG_SYN_PLAYER_HP* msg = (MSG_SYN_PLAYER_HP*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( msg->mCharacterIdx );
					if( pPlayer )
					{
						if( msg->mDie == true )
							pPlayer->HPChangeDie( msg->mMaxHp, 0, eDAMAGE_NORMAL );
						else
							pPlayer->HPChange( msg->mHp, msg->mMaxHp );
					}
				}
				break;
			case NM_PLAYER_MAXHP_SYN:
				{
					MSG_SYN_PLAYER_MAXHP* msg = (MSG_SYN_PLAYER_MAXHP*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( msg->mCharacterIdx );
					if( pPlayer )
						pPlayer->SetMaxHP( msg->mMaxHp );
				}
				break;
			case NM_PLAYER_MP_SYN:
				{
					MSG_SYN_PLAYER_MP* msg = (MSG_SYN_PLAYER_MP*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( msg->mCharacterIdx );
					if( pPlayer )
						pPlayer->MPChange( msg->mMp, msg->mMaxMp );
				}
				break;
			case NM_PLAYER_MAXMP_SYN:
				{
					MSG_SYN_PLAYER_MAXMP* msg = (MSG_SYN_PLAYER_MAXMP*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( msg->mCharacterIdx );
					if( pPlayer )
						pPlayer->SetMaxMP( msg->mMaxMp );
				}
				break;
			case NM_PLAYER_RESURRECTION_RES:
				{
					MSG_RES_PLAYER_RESURRECTION* msg = (MSG_RES_PLAYER_RESURRECTION*)pMsg;

					cRevivalWindow* win = mpUI->GetReVivalWindow();
					if( win )
						win->NetUnLock();

					/// 부활실패
					if( msg->ErrorCode != ERROR_PLAYER_RESURRECTION_SUCCESS )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 903 ) );
						return;
					}

					if( HERO->GetState() != eOBJECT_STATE_DIE )
					{
						assert(NULL);
					}

					if( win && win->IsVisible() == true )
						win->Hide();

					cStringT str;

					HERO->SetState( eOBJECT_STATE_IDLE );
					HERO->HPChange( msg->mHP, msg->mMaxHP );
					HERO->MPChange( msg->mMP, msg->mMaxMP );
					HERO->SetExp( msg->mPlayerExp );
					HERO->SetLoseExp( msg->mExpDown );
					HERO->FixPos( msg->mPosX, msg->mPosY, true );
					HERO->UpdateHeroInfo();

					cStringT exp = cNumberEditBox::FormatNumber( msg->mExpDown );

					/// 부활 메세지
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 904 ) );

					/// 경험치 감소 메세지
					if( HERO->GetLevel() >= 10 && msg->mExpDown > 0 )
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 905 ), (LPCTSTR)exp.Cstr() );
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
					}
				}
				break;
			case NM_PLAYER_RESURRECTION_CASHEXP_RES:
				{
					MSG_RES_PLAYER_CASHEXP_RESURRECTION* msg = (MSG_RES_PLAYER_CASHEXP_RESURRECTION*)pMsg;

					cRevivalWindow* win = mpUI->GetReVivalWindow();
					if( win )
						win->NetUnLock();

					/// 부활실패
					if( msg->ErrorCode != ERROR_PLAYER_RESURRECTION_SUCCESS )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 903 ) );
						return;
					}

					if( HERO->GetState() != eOBJECT_STATE_DIE )
					{
						assert(NULL);
					}

					if( win && win->IsVisible() == true )
						win->Hide();

					cStringT str;

					HERO->SetState( eOBJECT_STATE_IDLE );
					HERO->HPChange( msg->mHP, msg->mMaxHP );
					HERO->MPChange( msg->mMP, msg->mMaxMP );
					HERO->SetExp( msg->mPlayerExp );
					HERO->SetLoseExp( msg->mExpDown );
					HERO->FixPos( msg->mPosX, msg->mPosY, true );
					HERO->UpdateHeroInfo();

					cStringT exp = cNumberEditBox::FormatNumber( msg->mExpDown );

					/// 부활 메세지
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 904 ) );

					/// 경험치 방지 메세지
					str.Format( GAMERESOURCEMAN->GetGameText( 12015 ) );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

					cInfluenceObject* pInf = SKILLMAN->GetInfluenceObject( msg->mInfluenceIdx );
					if( pInf != NULL )
					{
						pInf->SetGuardInfo( msg->mCashExpCnt, 0 );
					}
					else
						assert(NULL);
				}
				break;
			case NM_PLAYER_RESURRECTION_SYN:
				{
					MSG_SYN_PLAYER_RESURRECTION* msg = (MSG_SYN_PLAYER_RESURRECTION*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( msg->mCharacterIdx );
					if( pPlayer == NULL )
					{
						assert(NULL);
						return;
					}

					if( pPlayer->GetState() != eOBJECT_STATE_DIE )
					{
						assert(NULL);
						return;
					}

					pPlayer->SetState( eOBJECT_STATE_IDLE );
					pPlayer->HPChange( msg->mHP, msg->mMaxHP );
					pPlayer->MPChange( msg->mMP, msg->mMaxMP );

					NiPoint3 pos( msg->mPosX, msg->mPosY, 0.0f );
					if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
					{
						assert(0);
						return;
					}

					pPlayer->SetPos( pos );
				}
				break;
			case NM_PLAYER_EXP_RES:
				{
					cStringT str;

					MSG_RES_EXP* resMsg = (MSG_RES_EXP*)pMsg;
					HERO->SetExp( resMsg->mTotalExp );
					HERO->UpdateHeroInfo();

					cStringT exp = cNumberEditBox::FormatNumber( resMsg->mExp );

					str.Format( GAMERESOURCEMAN->GetGameText( 2902 ), (LPCTSTR)exp.Cstr() );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
				}
				break;
			case NM_PLAYER_SXP_RES:
				{
					cStringT str;

					MSG_RES_SXP* resMsg = (MSG_RES_SXP*)pMsg;
					HERO->SetSxp( resMsg->mTotalSxp );
					HERO->UpdateHeroInfo();

					cStringT sxp = cNumberEditBox::FormatNumber( resMsg->mSxp );

					str.Format( GAMERESOURCEMAN->GetGameText( 2903 ), (LPCTSTR)sxp.Cstr() );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
				}
				break;
			case NM_PLAYER_PARTY_EXP_RES:
				{
					cStringT str;

					MSG_RES_PARTY_EXP* resMsg = (MSG_RES_PARTY_EXP*)pMsg;
					HERO->SetExp( resMsg->mTotalExp );
					HERO->UpdateHeroInfo();

					cStringT baseExp = cNumberEditBox::FormatNumber( resMsg->mExp );
					cStringT partyExp = cNumberEditBox::FormatNumber( resMsg->mPartyExp );

					str.Format( GAMERESOURCEMAN->GetGameText( 2904 ), (LPCTSTR)baseExp.Cstr(), (LPCTSTR)partyExp.Cstr() );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
				}
				break;
			case NM_PLAYER_PARTY_SXP_RES:
				{
					cStringT str;

					MSG_RES_PARTY_SXP* resMsg = (MSG_RES_PARTY_SXP*)pMsg;
					HERO->SetSxp( resMsg->mTotalSxp );
					HERO->UpdateHeroInfo();

					cStringT baseSxp = cNumberEditBox::FormatNumber( resMsg->mSxp );
					cStringT partySxp = cNumberEditBox::FormatNumber( resMsg->mPartySxp );

					str.Format( GAMERESOURCEMAN->GetGameText( 2905 ), (LPCTSTR)baseSxp.Cstr(), (LPCTSTR)partySxp.Cstr() );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
				}
				break;
			case NM_PLAYER_LEVEL_RES:
				{
					MSG_RES_LEVEL* resMsg = (MSG_RES_LEVEL*)pMsg;

					HERO->SetExp( resMsg->mExp );
					HERO->SetLevel( resMsg->mLevel );
					HERO->HPChange( resMsg->mHP, resMsg->mMaxHP );
					HERO->MPChange( resMsg->mMP, resMsg->mMaxMP );

					cTargetWindow *pTragetWin = (cTargetWindow*)GAMEUI->GetTargetWindow();
					if( pTragetWin && pTragetWin->IsVisible() )
					{
						cBaseObject* pTargetPlayer = (cBaseObject*)HERO->GetTargetObject();

						if( pTargetPlayer && pTargetPlayer == HERO )
						{
							pTragetWin->UpdateWindow();
						}
					}
					
					// 레벨 10, 20 ...마다 트위터 이벤트 메시지 전송
					if( OPTIONMAN->IsAutoSendTwitterMsg()	== true  &&
						resMsg->mLevel % TWIT_EVENT_LEVELUP == 0	)
					{
						cTwitterWindow* pTwitWin = GAMEUI->GetTwitterWindow();
						if( pTwitWin )
						{
							if( pTwitWin->IsLogin() == true )
							{
								sTwitterEvent tEvent;
								tEvent.mEvent	= eTWITEVENT_LEVEL;
								tEvent.mExData	= (void*)resMsg->mLevel;

								pTwitWin->AddEvent( tEvent );
							}
							
						}
						else
						{
							assert(0);
						}						
					}					

					cStringT str;

					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2900 ) );
					if( resMsg->mStr != 0 )
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 75 ), resMsg->mStr );
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
					}
					if( resMsg->mDex != 0 )
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 76 ), resMsg->mDex );
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
					}
					if( resMsg->mCon != 0 )
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 77 ), resMsg->mCon );
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
					}
					if( resMsg->mInt != 0 )
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 78 ), resMsg->mInt );
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
					}
					if( resMsg->mWis != 0 )
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 79 ), resMsg->mWis );
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
					}

					/// 이펙트 생성
 					HERO->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_lvup_01.nif", 0 );
 					HERO->SetLinkdSound( 2001 );

					/// 인벤, 퀵슬롯 아이템 사용 조건 갱신
					ITEMMAN->CheckItemLimitAll();
				}
				break;
			case NM_PLAYER_LEVEL_SYN:
				{
					MSG_SYN_LEVEL* synMsg = (MSG_SYN_LEVEL*)pMsg;

					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( synMsg->mCharacterIdx );
					if( pPlayer ) 
					{ 
						pPlayer->SetLevel( synMsg->mLevel );
						pPlayer->HPChange( synMsg->mHP, synMsg->mMaxHP );
						pPlayer->MPChange( synMsg->mMP, synMsg->mMaxMP );
 						pPlayer->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_lvup_01.nif", 0 );
 						HERO->SetLinkdSound( 2001 );

						cTargetWindow *pTragetWin = GAMEUI->GetTargetWindow();
						if( pTragetWin && pTragetWin->IsVisible() )
						{
							cBaseObject* pTargetPlayer = (cBaseObject*)HERO->GetTargetObject();

							if( pTargetPlayer && pTargetPlayer == pPlayer )
							{
								pTragetWin->UpdateWindow();
							}
						}

						/// 파티 확인
						if( HERO->GetPartyIndex() > 0 && HERO->GetObjectID() != pPlayer->GetObjectID() )
						{
							if( HERO->GetPartyIndex() == pPlayer->GetPartyIndex() )
								PARTYMAN->UpdateLevel( synMsg->mCharacterIdx, synMsg->mLevel );
						}
					}
				}
				break;
			case NM_PLAYER_SKILLLEVEL_RES:
				{
					MSG_RES_SKILLLEVEL* resMsg = (MSG_RES_SKILLLEVEL*)pMsg;

					HERO->SetSxp( resMsg->mSkillExp );
					HERO->SetSkillLevel( resMsg->mSkillLevel );
					HERO->SetSkillPointRemain( resMsg->mSkillPoint, resMsg->mSkillPointTotal );
					
					mpUI->UpdateExpSxp();
					mpUI->UpdateSkillPoint();
					mpUI->UpdateHeroInfo();

					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2901 ) );
 					HERO->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_slvup_01.nif", 0 );
 					HERO->SetLinkdSound( 2002 );
				}
				break;
			case NM_PLAYER_SKILLLEVEL_SYN:
				{
					MSG_SYN_SKILLLEVEL* synMsg = (MSG_SYN_SKILLLEVEL*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( synMsg->mPlayerIdx );
					if( player )
					{
						player->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_slvup_01.nif", 0 );
						player->SetLinkdSound( 2002 );
					}
				}
				break;
			case NM_PLAYER_STATE_IDLE_SYN:
				{
					MSG_SYN_PLAYER_STATE_IDLE* synMsg = (MSG_SYN_PLAYER_STATE_IDLE*)pMsg;

					cPlayer* pPlayer = OBJECTMAN->GetPlayer( synMsg->mCharacterIdx );
					if( pPlayer )
					{
						pPlayer->SetIdleFlag( synMsg->mPlayerStateIdle );
					}
				}
				break;
			case NM_PLAYER_STATE_SITDOWN_RES:
				{
					MSG_RES_PLAYER_STATE_SITDWON* resMsg = (MSG_RES_PLAYER_STATE_SITDWON*)pMsg;

					if( resMsg->ErrorCode != 0 )
					{
						/// 서버 상태로 동기화
						if( resMsg->mSitDown == true )
							HERO->SitDown();
						else
							HERO->StandUp();
					}
				}
				break;
			case NM_PLAYER_STATE_SITDOWN_SYN:
				{
					MSG_SYN_PLAYER_STATE_SITDWON* synMsg = (MSG_SYN_PLAYER_STATE_SITDWON*)pMsg;

					cPlayer* p = OBJECTMAN->GetPlayer( synMsg->mCharacterIdx );
					if( p == 0 )
						return;

					if( synMsg->mSitDown == true )
						p->SitDown();
					else
						p->StandUp();
				}
				break;
			case NM_PLAYER_SP_RES:
				{
					MSG_RES_PLAYER_SP* resMsg = (MSG_RES_PLAYER_SP*)pMsg;

					HERO->SetSkillPointRemain( resMsg->mSP, resMsg->mTotalSP );
					HERO->SetJobUsedSkillPoint( resMsg->rowCount, resMsg->usedSP );

					/// 윈도우 세팅
					mpUI->UpdateSkillPoint();
				}
				break;
			case NM_PLAYER_COMMUNITYSKILL_RES:
				{
					MSG_RES_PLAYER_COMMUNITYSKILL* pmsg = (MSG_RES_PLAYER_COMMUNITYSKILL*)pMsg;
					if( pmsg->ErrorCode == 0 )
					{
						/// 
						sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( pmsg->mCommunitySkillIdx );
						if( pInfo )
						{
							if( pInfo->mType != 0 )
							{
								HERO->ChangeAnimation( (ePLAYER_ANIMATIONTYPE)pInfo->mRoleIndex );
							}
							else
							{
								assert(0);
							}
						}
					}
					else
					{
						/// check server & client player state!!
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6003 ) );
					}

					cSkillWindow* win = mpUI->GetSkillWindow();
					if( win )
						win->NetUnLockCommu();
				}
				break;
			case NM_PLAYER_COMMUNITYSKILL_SYN:
				{
					MSG_SYN_PLAYER_COMMUNITYSKILL* synMsg = (MSG_SYN_PLAYER_COMMUNITYSKILL*)pMsg;
					cPlayer* player = OBJECTMAN->GetPlayer( synMsg->mCharacterIdx );
					if( player )
					{
						sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( synMsg->mCommunitySkillIdx );
						if( pInfo )
						{
							if( pInfo->mType != 0 )
							{
								player->ChangeAnimation( (ePLAYER_ANIMATIONTYPE)pInfo->mRoleIndex );
							}
							else
							{
								assert(0);
							}
						}
					}
				}
				break;
			case NM_PLAYER_STATICFACE_SYN:
				{
					MSG_SYN_PLAYER_STATICFACE* synMsg = (MSG_SYN_PLAYER_STATICFACE*)pMsg;
					cPlayer* player = OBJECTMAN->GetPlayer( synMsg->mCharacterIdx );
					if( player )
						player->SetStaticFace( synMsg->mStaticFaceIdx );
				}
				break;
			case NM_PLAYER_FORCE_SYN:
				{
					/// HERO에게만 오는 메세지임
					MSG_SYN_PLAYER_FORCE* pmsg = (MSG_SYN_PLAYER_FORCE*)pMsg;

					cPlayer* pPlayer = OBJECTMAN->GetPlayer( pmsg->mPlayerIdx );
					if( pPlayer != NULL )
						pPlayer->SetForceType( pmsg->mForceType );

					ITEMMAN->CheckItemLimitAll();
					ITEMMAN->CheckEquipAbility();
				}	
				break;
			case NM_PLAYER_FORCEVALUE_SYN:
				{
					MSG_SYN_PLAYER_FORCEVALUE* pmsg = (MSG_SYN_PLAYER_FORCEVALUE*)pMsg;
					cSocialWindow* social = mpUI->GetSocialWindow();

					switch( pmsg->mForceType )
					{
					case eFORCETYPE_FIRE:
						{
							if( pmsg->mValueType == 1 )
							{
								HERO->SetFirePoint( pmsg->mValue );

								if( social )
									social->SetFirePoint( pmsg->mValue );
							}
							else if( pmsg->mValueType == 2 )
								HERO->SetFireFriendly( pmsg->mValue );
							else
								assert(NULL);
						}
						break;
					case eFORCETYPE_WATER:
						{
							if( pmsg->mValueType == 1 )
							{
								HERO->SetWaterPoint( pmsg->mValue );

								if( social )
									social->SetWaterPoint( pmsg->mValue );
							}
							else if( pmsg->mValueType == 2 )
								HERO->SetWaterFriendly( pmsg->mValue );
							else
								assert(NULL);
						}
						break;
					case eFORCETYPE_WIND:
						{
							if( pmsg->mValueType == 1 )
							{
								HERO->SetWindPoint( pmsg->mValue );

								if( social )
									social->SetWindPoint( pmsg->mValue );
							}
							else if( pmsg->mValueType == 2 )
								HERO->SetWindFriendly( pmsg->mValue );
							else
								assert(NULL);
						}
						break;
					case eFORCETYPE_EARTH:
						{
							if( pmsg->mValueType == 1 )
							{
								HERO->SetEarthPoint( pmsg->mValue );

								if( social )
									social->SetEarthPoint( pmsg->mValue );
							}
							else if( pmsg->mValueType == 2 )
								HERO->SetEarthFriendly( pmsg->mValue );
							else
								assert(NULL);
						}
						break;
					}
				}
				break;
			case NM_PLAYER_JOB_RES:
				{
					/// 전직 
					MSG_RES_JOB* pmsg = (MSG_RES_JOB*)pMsg;

					///
					HERO->SetJob( (ePLAYER_JOB)pmsg->mJob );

					cTargetWindow *pTragetWin = (cTargetWindow*)GAMEUI->GetTargetWindow();
					if( pTragetWin && pTragetWin->IsVisible() )
					{
						cBaseObject* pTargetPlayer = (cBaseObject*)HERO->GetTargetObject();

						if( pTargetPlayer && pTargetPlayer == HERO )
						{
							pTragetWin->UpdateWindow();
						}
					}

					for( int i = 0 ; i < pmsg->mSkillListCnt ; ++i )
						SKILLMAN->AddKeepSkill( pmsg->mSkillClassIdx[i], 0 );

					if( pmsg->mSkillListCnt != 0 )
					{
						/// 071024 PKH 스테이터스 재계산
						HERO->CalcStatus();

						/// 이펙트 추가
						NiTransform trans;
						trans.MakeIdentity();
						trans.m_Translate = NiPoint3( 0.0f, 0.0f, 30.0f );

 						HERO->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_jobchange_01.nif", &trans, false, true );
 						HERO->SetLinkdSound( 2003 );

					}

					/// 인벤, 퀵슬롯 아이템 사용 조건 갱신
					ITEMMAN->CheckItemLimitAll();
					ITEMMAN->CheckEquipAbility();
				}
				break;
			case NM_PLAYER_JOB_SYN:
				{
					MSG_SYN_JOB* pmsg = (MSG_SYN_JOB*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( pmsg->mPlayerIdx );
					if( player )
					{
						player->SetJob( (ePLAYER_JOB)pmsg->mJob );

						cTargetWindow *pTragetWin = GAMEUI->GetTargetWindow();
						if( pTragetWin && pTragetWin->IsVisible() )
						{
							cBaseObject* pTargetPlayer = (cBaseObject*)HERO->GetTargetObject();

							if( pTargetPlayer && pTargetPlayer == player )
							{
								pTragetWin->UpdateWindow();
							}
						}

						/// 이펙트 추가
						NiTransform trans;
						trans.MakeIdentity();
						trans.m_Translate = NiPoint3( 0.0f, 0.0f, 30.0f );
 						player->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/eff_pc_jobchange_01.nif", &trans, false, true );
 						player->SetLinkdSound( 2003 );
					}
				}
				break;
			case NM_PLAYER_TITLE_ADD_RES:
			case NM_PLAYER_TITLE_LIST_RES:
			case NM_PLAYER_TITLE_SET_RES:
			case NM_PLAYER_TITLE_SET_SYN:
				{
					TITLEMAN->NetworkMsgParser( pMsg );
				}
				break;
			case NM_PLAYER_PARTY_SET_SYN:
				{
					MSG_SYN_PLAYER_PARTY_SET* msg = (MSG_SYN_PLAYER_PARTY_SET*)pMsg;
					
					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIndex );
					if( player )
					{
						player->SetPartyIndex( msg->mPartyIndex );
					}
				}
				break;
			case NM_PLAYER_PARTYUNION_SET_SYN:
				{
					MSG_SYN_PLAYER_PARTYUNION_SET* msg = (MSG_SYN_PLAYER_PARTYUNION_SET*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIndex );
					if( player )
					{
						player->SetPartyUnionIndex( msg->mPartyUnionIndex );
					}
				}
				break;
			case NM_PLAYER_GUILD_SET_SYN:
				{
					MSG_SYN_PLAYER_GUILD_SET* msg = (MSG_SYN_PLAYER_GUILD_SET*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIndex );
					if( player )
					{
						if( msg->mGuildIndex > 0 )
							player->SetGuild( msg->mGuildIndex, msg->mPosition, msg->mName, msg->mMarkRegistDate, msg->mUseGuildMark );
						else
							player->ClearGuild();
					}
				}
				break;
			case NM_PLAYER_GUILDMARK_CHANGE_SYN:
				{
					MSG_SYN_PLAYER_GUILDMARK_CHANGE* msg = (MSG_SYN_PLAYER_GUILDMARK_CHANGE*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIndex );
					if( player )
					{
						player->ChangeGuildMarkRegistDate( msg->mMarkRegistDate, msg->mUseGuildMark );
					}

					if( player == HERO )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1254 ) );
					}
				}
				break;
			case NM_PLAYER_TODAYWORD_CHANGE_RES:
				{
					/// 오늘의 한마디 변경
					MSG_RES_PLAYER_TODAYWORD_CHANGE* msg = (MSG_RES_PLAYER_TODAYWORD_CHANGE*)pMsg;

					cTodayWordWindow* win = mpUI->GetTodayWordWindow();
					switch( msg->ErrorCode )
					{
					case ERROR_PLAYER_TODAYWORD_CHANGE_SUCCESS:
						{
							if( win )
								win->Hide();

							HERO->SetTodayWord( msg->mWord, msg->mWordColor );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 338 ) );
						}
						break;
					case ERROR_PLAYER_TODAYWORD_CHANGE_INIT:
						HERO->SetTodayWord( msg->mWord, msg->mWordColor );
						break;
					case ERROR_PLAYER_TODAYWORD_CHANGE_FAIL:
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 339 ) );
						break;
					}

					if( msg->ErrorCode != ERROR_PLAYER_TODAYWORD_CHANGE_INIT )
					{
						if( win )
							win->NetUnLock();
					}
				}
				break;
			case NM_PLAYER_TODAYWORD_CHANGE_SYN:
				{
					MSG_SYN_PLAYER_TODAYWORD_CHANGE* msg = (MSG_SYN_PLAYER_TODAYWORD_CHANGE*)pMsg;

					cPlayer* player = OBJECTMAN->GetPlayer( msg->mCharacterIndex );
					if( player )
					{
						player->SetTodayWord( msg->mWord, msg->mWordColor );
					}
				}
				break;
			case NM_PLAYER_SKILLRESET_RES:
				{
					MSG_RES_PLAYER_SKILLRESET* msg = (MSG_RES_PLAYER_SKILLRESET*)pMsg;

					/// 상태 복구
					HERO->SetStopFlag( eSTOP_NONE );

					///
					UIMAN->DestroyMsgBox( eMBEVENT_SKILLRESETPROCESS );
	
					if( msg->ErrorCode == ERROR_SKILLRESET_SUCCESS )
					{
						/// 스킬 포인트 셋팅
						HERO->SetSkillPointRemain( msg->mSkillPoint, msg->mSkillPoint );

						/// 스킬 사용 포인트 초기화
						HERO->InitJobUsedSkillPoint();

						/// 퀵슬롯 초기화
						QUICKMAN->SkillReset();

						/// 스킬 목록 초기화
						SKILLMAN->ClearAllKeepSkill();

						/// 윈도우 초기화
						cSkillWindow* pWin = GAMEUI->GetSkillWindow();
						if( pWin )
							pWin->InitSkillData();

						// 정보
						STATUSCALC->CalcPlayerInit();
						HERO->CalcStatus();
						
						/// 기본 공격 등록
						SKILLMAN->AddKeepSkill( NORMAL_ATTACK_SKILL, 0 );

						for( int i = 0 ; i < msg->mRowCount ; ++i )
						{
							sInitSkill* pSkill = &msg->mTable[i];
							if( pSkill != NULL )
								SKILLMAN->AddKeepSkill( pSkill->mSkillIdx, pSkill->mSkillStep );
						}

						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_SKILLRESETPROCESS, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 264 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);

					}
					else if( msg->ErrorCode == ERROR_SKILLRESET_STATE )
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6113 ) );
					else if( msg->ErrorCode == ERROR_SKILLRESET_ITEM )
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6111 ) );
					else if( msg->ErrorCode == ERROR_SKILLRESET_DBERROR )
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6114 ) );
					else if( msg->ErrorCode == ERROR_SKILLRESET_ITEM_LICENSE )
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6115 ) );

					///
					ITEMMAN->UnlockItemTry( ITEM_TRY_SKILLRESET );
				}
				break;
		
			case NM_PLAYER_EXP_RECOVER_RES:		// 경험치 복구카드 사용 결과 = 경험치만 복구
				{
					sHeroInfo* pInfo = HERO->GetHeroInfo();
					if( pInfo == NULL )
					{
						assert(0);
						return;
					}

					if( pInfo->mLoseExp == 0 )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 11902 ),
							GAMERESOURCEMAN->GetGameText( 301 ) );
						return;
					}

					MSG_RES_RECOVER_EXP* msg = (MSG_RES_RECOVER_EXP*)pMsg;

					cStringT str;
					cStringT restoreExp = cNumberEditBox::FormatNumber( msg->mExp );
					str.Format( GAMERESOURCEMAN->GetGameText( 11900 ), (LPTSTR)restoreExp.Cstr() );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPTSTR)str.Cstr() );
					
					HERO->SetExp( msg->mTotalExp );
					HERO->SetLoseExp( 0 );
					HERO->UpdateHeroInfo();

				}
				break;
			case NM_PLAYER_JUMP_RES:
				{
					/// 점프 취소
					TCHAR tempmsg[255] = {0,};
					_stprintf( tempmsg, _T("ジャンプ不可") );
					CHATMANAGER->AddSystemMsg(eSYSTEM_NORMAL, tempmsg);
				}
				break;
			case NM_PLAYER_JUMP_SYN:
				{
					MSG_SYN_PLAYER_JUMP* msg = (MSG_SYN_PLAYER_JUMP*)pMsg;

					/// player 점프
					cPlayer* pPlayer = OBJECTMAN->GetPlayer( msg->mPlayerIdx );
					if( pPlayer )
					{
						pPlayer->Jump();
					}					
				}
				break;
			case NM_PLAYER_INVEN_SIZE_SYN:
				{
					MSG_SYN_PLAYER_INVEN_SIZE* msg = (MSG_SYN_PLAYER_INVEN_SIZE*)pMsg;
					/// 플레이어 슬롯 크기 재 설정
					sHeroInfo* pHeroInfo = HERO->GetHeroInfo();
					if( pHeroInfo )
					{
						if( pHeroInfo->BagEnd != msg->mBag )
						{
							pHeroInfo->BagEnd = msg->mBag;
							/// 가방 개수 업데이트
							cInventoryWindow* pWin = GAMEUI->GetInventoryWindow();
							if( pWin )
							{
								if( pWin->SetBagEndSlot( pHeroInfo->BagEnd ) == false )
								{
									assert( 0 && "Fail to update Inventory bag slot");
								}
								else
								{		
									// 인벤토리 가방 추가로 인한 최대 가용 무게값 갱신
									if( HERO )
										HERO->CalcStatus();
								}								
							}
							else
								assert( 0 );
						}
						else if( pHeroInfo->WearEnd != msg->mWear )
						{
							pHeroInfo->WearEnd = msg->mWear;
							/// 카드 슬롯 업데이트
							cInventoryWindow* pWin = GAMEUI->GetInventoryWindow();
							if( pWin )
							{
								if( pWin->SetWearCardSlot( pHeroInfo->WearEnd ) == false )
									assert( 0 && "Fail to update character card slot");
							}
							else
								assert( 0 );

						}
						else if( pHeroInfo->WareHouseEnd != msg->mWareHouse )
						{
							pHeroInfo->WareHouseEnd = msg->mWareHouse;
							/// 창고 가방 슬롯 업데이트
							cWareHouseWindow* pWin = GAMEUI->GetWareHouseWindow();
							if( pWin )
							{
								if( pWin->SetWareBagEndSlot( pHeroInfo->WareHouseEnd ) == false )
									assert( 0 && "Fail to update warehouse bag slot");
							}
							else
								assert( 0 );
						}
						else if( pHeroInfo->MakeSkillEnd != msg->mMakeSkill )
						{
							pHeroInfo->MakeSkillEnd = msg->mMakeSkill;
						}
						else
						{
							assert( 0 && "Nothing change slots value" );
						}
					}
					else
					{
						assert( 0 && "Bad HeroInfo" );
					}
				}
				break;

			case NM_PLAYER_TRIAL_OPEN_RES:
				{
					MSG_RES_PLAYER_TRIAL_OPEN* resMsg = (MSG_RES_PLAYER_TRIAL_OPEN *)pMsg;
					switch ( resMsg ->ErrorCode )
					{
					case ERROR_PLAYER_TRIAL_OPEN_SUCCESS:
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11800 ) );
						break;
					case ERROR_PLAYER_TRIAL_OPEN_FAIL:
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11801 ) );
						break;
					case ERROR_PLAYER_TRIAL_OPEN_MAP: /// 신고기능을 사용 할 수 없는 맵
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11802 ) );
						break;
					case ERROR_PLAYER_TRIAL_OPEN_TIME: /// 재사용 시간(3분/1시간)이 지나지 않음
						{
							long min = ( resMsg->remainTime % HOUR ) / MINUTE;
							long sec = ( resMsg->remainTime % MINUTE ) / SECOND; 
							cStringT msg;
							cStringT second;
							if( min > 0 )
								msg.Format( GAMERESOURCEMAN->GetGameText( 71 ), min );
							if( sec > 0 )
							{
								second.Format( GAMERESOURCEMAN->GetGameText( 72 ), sec );
								msg.Append( second.Cstr() );
							}
							
							msg.Append( GAMERESOURCEMAN->GetGameText( 11803 ) );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
						}
						break;
					case ERROR_PLAYER_TRIAL_OPEN_LEVEL: /// 10레벨 이상 사용 가능
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11804 ) );
						break;
					case ERROR_PLAYER_TRIAL_OPEN_TRIAL: /// 신고자 또는 내가 심판중
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11805 ) );
						break;
					case ERROR_PLAYER_TRIAL_OPEN_TRIAL_REJECT: /// 신고자가 문제 푼지 5분이 안지났음.
						{
							long min = ( resMsg->remainTime % HOUR ) / MINUTE;
							long sec = ( resMsg->remainTime % MINUTE ) / SECOND; 
							cStringT msg;
							cStringT second;
							if( min > 0 )
								msg.Format( GAMERESOURCEMAN->GetGameText( 71 ), min );
							if( sec > 0 )
							{
								second.Format( GAMERESOURCEMAN->GetGameText( 72 ), sec );
								msg.Append( second.Cstr() );
							}

							if( min == 0 && sec == 0 )
							{
								msg = GAMERESOURCEMAN->GetGameText( 11820 );
							}

							msg.Append( GAMERESOURCEMAN->GetGameText( 11815 ) );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
						}
						break;
					case ERROR_PLAYER_TRIAL_OPEN_GM:
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11821 ) );
						break;
					}
				}
				break;
			case NM_PLAYER_TRIAL_OPEN_SYN:
				{
					MSG_SYN_PLAYER_TRIAL_OPEN* synMsg = (MSG_SYN_PLAYER_TRIAL_OPEN*)pMsg;
			
					if(	mpUI )
					{
						cCamaelWindow* camaelWin = mpUI->GetCamaelWindow();
						if( camaelWin )
						{
							if( camaelWin->SetCaptchaImage( synMsg->captcha, synMsg->GetMsgLength() ) )
							{
								camaelWin->SetTimeCheck( true );
								camaelWin->Show();
							}
						}
						else
						{
							assert(0);
							break;
						}
					}
					else
					{
						assert(0);
						break;
					}
				}
				break;
			case NM_PLAYER_TRIAL_ANSWER_RES:
				{					
					MSG_RES_PLAYER_TRIAL_ANSWER* resMsg = (MSG_RES_PLAYER_TRIAL_ANSWER*)pMsg;

					cCamaelWindow* camaelWin = 0;
					if( mpUI )
					{
						camaelWin = mpUI->GetCamaelWindow();
						if( !camaelWin )
						{
							assert(0);
							break;
						}
					}
					else
					{
						assert(0);
						break;
					}

					switch( resMsg->ErrorCode )
					{
					case ERROR_PLAYER_TRIAL_ANSWER_SUCCESS:
						{
							camaelWin->Hide();
							camaelWin->SetTimeCheck( false );
						}
						break;
					case ERROR_PLAYER_TRIAL_ANSWER_FAIL:
						{
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11809 ) );
						}
						break;
					case ERROR_PLAYER_TRIAL_ANSWER_WRONG: /// 정답 틀림
						{
							if( resMsg->trialCount > 0 )
							{
								cStringT msg;
								msg.Format( GAMERESOURCEMAN->GetGameText( 11806 ), resMsg->trialCount );
								CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
								camaelWin->SetRefreshButtonEnable( true );
							}
							else
							{
								///정답 횟수 0.
								UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_CAMAEL_KICK, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 11807 ),
									GAMERESOURCEMAN->GetGameText( 301 )
									);
								camaelWin->SetTimeCheck( false );
								camaelWin->Hide();
							}
						}
						break;
					case ERROR_PLAYER_TRIAL_ANSWER_TIME_OVER: /// 심판 시간 초과
						{
							UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_CAMAEL_KICK, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 11807 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
							camaelWin->SetTimeCheck( false );
							camaelWin->Hide();
						}
						break;
					case ERROR_PLAYER_TRIAL_ANSWER_TEXTLEN:
						{
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11808 ) );
						}
						break;
					default:
						assert(0);
					}
				}
				break;
			case NM_PLAYER_TRIAL_RESULT_SYN:
				{
					unsigned long errorIndex = 11810;
					MSG_SYN_PLAYER_TRIAL_RESULT* synMsg = (MSG_SYN_PLAYER_TRIAL_RESULT*)pMsg;
					switch( synMsg->ErrorCode )
					{
					case ERROR_PLAYER_TRIAL_RESULT_AUTO:     errorIndex = 11810;	break;
					case ERROR_PLAYER_TRIAL_RESULT_NOT_AUTO: errorIndex = 11811;	break;
					default: assert(0); break;
					}

					cStringT msg;
					msg.Format( GAMERESOURCEMAN->GetGameText( errorIndex ), synMsg->name );
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						(LPCTSTR)msg.Cstr(),
						GAMERESOURCEMAN->GetGameText( 301 )	// 알림
						);	
				}
				break;
			case NM_PLAYER_TRIAL_CAPTCHA_RES:
				{
					MSG_RES_PLAYER_TRIAL_CAPTCHA* synMsg = (MSG_RES_PLAYER_TRIAL_CAPTCHA*)pMsg;

					cCamaelWindow* camaelWin = GAMEUI->GetCamaelWindow();
					if( !camaelWin || camaelWin->IsVisible() == false )
					{
						assert(0);
						break;
					}
				
					switch( synMsg->ErrorCode )
					{
					case ERROR_PLAYER_TRIAL_CAPTCHA_SUCCESS:
						{
							if( camaelWin->SetCaptchaImage( synMsg->captcha, synMsg->GetMsgLength() ) == false )
							{
								assert(0);
								CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11810 ) );
							}

							if( synMsg->count == 0 )
							{
								camaelWin->SetRefreshButtonEnable( false );
								camaelWin->SetOverRefresh( true );
							}

							cStringT msg;
							msg.Format( GAMERESOURCEMAN->GetGameText( 11813 ), synMsg->count );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
						}
						break;
					case ERROR_PLAYER_TRIAL_CAPTCHA_FAIL:
						{
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11812 ) );
						}
						break;
					case ERROR_PLAYER_TRIAL_CAPTCHA_COUNT:
						{
							camaelWin->SetOverRefresh( true );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11814 ) );
						}
						break;
					case ERROR_PLAYER_TRIAL_CAPTCHA_TRIAL:
						{
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11819 ) );
						}
						break;
					}

					camaelWin->SetRefreshButtonEnable( true );
				}
				break;
			case NM_PLAYER_LOOK_OTHER_EQUIP_RES:
				{
					MSG_RES_PLAYER_LOOK_OTHER_EQUIP* msg = (MSG_RES_PLAYER_LOOK_OTHER_EQUIP*)pMsg;

					if( msg->ErrorCode == ERROR_PLAYER_LOOK_OTHER_EQUIP_SUCCESS )
					{
						cLookWindow* lookWin = GAMEUI->GetLookWindow();
						if( lookWin )
						{
							lookWin->Clear();

							//
							lookWin->AddInfo( msg->name, msg->level, msg->job, msg->wearEnd );
							lookWin->SwitchWeapon( (eItemActiveWeapon)(msg->activeWeapon) );

							// 정보 셋팅
							sInventory* inven = msg->Inventory;
							for( unsigned long i = 0; i < msg->RowCount; ++i, ++inven )
							{
								lookWin->AddInven( inven );
							}

							lookWin->Show();
						}
						else
						{
							assert(0);
						}
					}
					else
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 882 ) );
					}
				}
				break;
			case NM_PLAYER_PCITEM_GIVE_SYN:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 13900 ) );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 13901 ) );
				}
				break;
			} //switch( pTemMsg->Protocol )
		} //case NM_PLAYER:
		break;
		case NM_PLAYEREXT:
		{
			switch( pTemMsg->Protocol )
			{


			case NM_PLAYEREXT_DOORPUSH_SYN:
				{
					MSG_SYN_PLAYEREXT_DOORPUSH* pResMsg = (MSG_SYN_PLAYEREXT_DOORPUSH*)pMsg;

					cBaseObject* pTarget = OBJECTMANAGER->GetObject( &pResMsg->mTarget );
					if( pTarget != NULL )
						pTarget->SetSkillPushPull( pResMsg->mPosX, pResMsg->mPosY );
				}
				break;
			case NM_PLAYEREXT_AVOID_MON_LOOK_SYN:
				{
					MSG_SYN_PLAYEREXT_AVOID_MON_LOOK* pSynMsg = (MSG_SYN_PLAYEREXT_AVOID_MON_LOOK*)pMsg;
					if( pSynMsg )
					{
						HERO->SetAvoidMonLook( pSynMsg->mIsAvoidMonLookOn );
					}
				}
				break;
			case NM_PLAYEREXT_RUSHMOVESTOP_SYN:
				{
					MSG_SYN_MOVE_STOP* pmsg = (MSG_SYN_MOVE_STOP*)pMsg;

					cPlayer* player = OBJECTMANAGER->GetPlayer( pmsg->mCharacterIdx );
					if( player )
					{
						NiPoint3 pos = NiPoint3( pmsg->mCharacterPosX, pmsg->mCharacterPosY, 0.0f );
						if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) )
						{
							player->SetPos( pos );
						}

						player->RushSkillStop();
						if( !(player->GetState() == eOBJECT_STATE_DIE || player->GetState() == eOBJECT_STATE_STOP) )
							player->SetState( eOBJECT_STATE_IDLE );
					}
				}
				break;
			}			
		}
		break; //case NM_PLAYEREXT:
	case NM_TOTEM:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_TOTEM_SIGHT_IN_RES:
				{
					MSG_TOTEM_INFO* pmsg = (MSG_TOTEM_INFO*)pMsg;
					OBJECTMAN->AddTotem( pmsg->mObjectIdx, pmsg->mCreateInfClassIdx, NiPoint3(pmsg->mPosX, pmsg->mPosY, 0.0f), pmsg->mDirection );
				}
				break;
			case NM_TOTEM_SIGHT_OUT_RES:
				{
					MSG_TOTEMIDX* pmsg = (MSG_TOTEMIDX*)pMsg;
					OBJECTMAN->RemoveTotem( pmsg->mObjectIdx );
				}
				break;
			} //switch( pTemMsg->Protocol )
		} //case NM_TOTEM:
		break;
	case NM_SKILL:
		{
			SKILLMAN->NetworkMsgParser(pMsg);
		}
		break;
	case NM_CHAT:
		{
			CHATMANAGER->NetworkMsgParser(pMsg, length);
		}
		break;
	case NM_TAROT:
		{
			TAROTMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_ITEM:
		{
			ITEMMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_TRADE:
		{
			ITEMMAN->NetworkMsgParserTrade( pMsg );
		}
		break;
	case NM_CHEAT:
		{
			CHEATMAN->NetworkMsgParser( pTemMsg );
		}
		break;
	case NM_PARTY:
		{
			PARTYMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_QUEST:
		{
			QUESTMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_PVP:
		{
			PVPMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_DUEL:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_DUEL_ADD_RES:
				{
					MSG_RES_DUEL_ADD* pSynMsg = (MSG_RES_DUEL_ADD*)pMsg;
					switch( pSynMsg->ErrorCode )
					{
					case ERR_DUEL_ADD_SUCCESS:
						{
							/// HERO 결투 수락창 출력
							MSG_RES_DUEL_ADD* pResMsg = (MSG_RES_DUEL_ADD*)pMsg;
							HERO->ViewMsgDuelReq( pResMsg->mAttackerIdx );
						}
						break;
					case ERR_DUEL_ADD_FAIL:
						{
							/// 실패
							/// 결투 신청 불가 - 결투 요청자임
							cStringT Message;
							Message.Format( GAMERESOURCEMAN->GetGameText( 805 ) );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
						}
						break;
					case ERR_DUEL_ADD_OPTIONREFUSE:
						{
							/// 옵션으로 인한 거절 - 결투 요청자임
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2702 ) );
						}
						break;
					}
				}
				break;     
			case NM_DUEL_ACCEPT_RES:
				{
					MSG_RES_DUEL_ACCEPT* pResMsg = (MSG_RES_DUEL_ACCEPT*)pMsg;
					if( pResMsg->ErrorCode == ERR_DUEL_ACCEPT_SUCCESS )
					{
						/// duel count start
						DUELMAN->StartCount( pResMsg->mEnemyIdx, pResMsg->mCenterPos );
						HERO->SetDuelEffect();

						///
						if( HERO->GetState() == eOBJECT_STATE_MOVE )
						{
							if( HERO->IsFollowing() )
								HERO->ReleaseFollowPlayer();

							HERO->ClearMoveFlag();
						}

						/// enemy info
						cPlayer* player = OBJECTMAN->GetPlayer( pResMsg->mEnemyIdx );
						if( player )
							player->SetDuelEffect();
					}
					else
					{
						HERO->DuelNoStartMsg( pResMsg->ErrorCode );
					}
				}
				break;
			case NM_DUEL_ACCEPT_SYN:
				{
					MSG_SYN_DUEL_ACCEPT* pResMsg = (MSG_SYN_DUEL_ACCEPT*)pMsg;
					cPlayer* player = OBJECTMAN->GetPlayer( pResMsg->mAttackerIdx );
					if( player )
						player->SetDuelEffect();

					player = OBJECTMAN->GetPlayer( pResMsg->mTargetIdx );
					if( player )
						player->SetDuelEffect();
				}
				break;
			case NM_DUEL_START_RES:
				{
					DUELMAN->StartDuel();
				}
				break;
			case NM_DUEL_FIGHT_OUTRANGE_RES:
				{
					DUELMAN->SetOutofBound( true );
				}
				break;
			case NM_DUEL_FIGHT_INRANGE_RES:
				{
					DUELMAN->SetOutofBound( false );
				}
				break;
			case NM_DUEL_WINNER_SYN:
				{
					MSG_SYN_DUEL_WINNER* pSynMsg = (MSG_SYN_DUEL_WINNER*)pMsg;

					///
					HERO->ClearDuelEffect();
					HERO->SetState( eOBJECT_STATE_IDLE );
					if( HERO->IsTransformMonster() )
						HERO->ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
					else
						HERO->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) ANITYPE_DUELWIN );

					cPlayer* player = OBJECTMAN->GetPlayer( pSynMsg->mLoserIdx );
					if( player )
					{
						player->ClearDuelEffect();
						player->SetState( eOBJECT_STATE_IDLE );

						if( player->IsTransformMonster() )
							player->ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
						else
							player->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) ANITYPE_DUELLOSE );
					}

					/// 결투 설정 해제
					DUELMAN->EndDuel();

					/// 결투 종료 메세지 처리
					cStringT Message;
					Message.Format( GAMERESOURCEMAN->GetGameText( 807 ), pSynMsg->mWinnerName, pSynMsg->mLoserName, 
						pSynMsg->mWinnerName, pSynMsg->mStraightWin );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );

					/// 결투 승리 사운드 재생
					SOUNDSYS->Play2DSound( 2042 );

				}
				break;
			case NM_DUEL_LOSER_SYN:
				{
					MSG_SYN_DUEL_LOSER* pSynMsg = (MSG_SYN_DUEL_LOSER*)pMsg;

					///
					HERO->ClearDuelEffect();
					HERO->SetState( eOBJECT_STATE_IDLE );
					if( HERO->IsTransformMonster() )
						HERO->ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
					else
						HERO->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) ANITYPE_DUELLOSE );

					cPlayer* player = OBJECTMAN->GetPlayer( pSynMsg->mWinnerIdx );
					if( player )
					{
						player->ClearDuelEffect();
						player->SetState( eOBJECT_STATE_IDLE );

						if( player->IsTransformMonster() )
							player->ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
						else
							player->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) ANITYPE_DUELWIN );
					}

					/// 결투 설정 해제
					DUELMAN->EndDuel();

					/// 결투 종료 메세지 처리
					cStringT Message;
					Message.Format( GAMERESOURCEMAN->GetGameText( 807 ), pSynMsg->mWinnerName, pSynMsg->mLoserName, 
						pSynMsg->mWinnerName, pSynMsg->mStraightWin );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );

					/// 결투 패배 사운드 재생
					SOUNDSYS->Play2DSound( 2043 );

				}
				break;
			case NM_DUEL_END_SYN:
				{
					MSG_SYN_DUEL_END* pSynMsg = (MSG_SYN_DUEL_END*)pMsg;

					///
					cPlayer* player = OBJECTMAN->GetPlayer( pSynMsg->mWinnerIdx );
					if( player )
					{
						player->ClearDuelEffect();
						player->SetState( eOBJECT_STATE_IDLE );

						if( player->IsTransformMonster() )
							player->ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
						else
							player->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) ANITYPE_DUELWIN );
					}

					player = OBJECTMAN->GetPlayer( pSynMsg->mLoserIdx );
					if( player )
					{
						player->ClearDuelEffect();
						player->SetState( eOBJECT_STATE_IDLE );

						if( player->IsTransformMonster() )
							player->ChangeAnimation_TransMon( (eMONSTER_ANIMATIONTYPE)M_ANITYPE_IDLE1 );
						else
							player->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) ANITYPE_DUELLOSE );
					}

					/// 결투 종료 메세지 처리
					cStringT Message;
					Message.Format( GAMERESOURCEMAN->GetGameText( 807 ), pSynMsg->mWinnerName, pSynMsg->mLoserName, 
						pSynMsg->mWinnerName, pSynMsg->mStraightWin );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
				}
				break;
			}
		}
		break;
	case NM_GUILD:
		{
			GUILDMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_MAKESKILL:
		{
			ITEMMIXMAN->NetworkMsgParserGame( pMsg );
		}
		break;
	case NM_THEME:
		{
			THEMEMAN->NetworkMsgParser( pMsg );	
		}
		break;
	case NM_PARTYUNION:
		{
			PARTYUNIONMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_OBSTRUCTION:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_OBSTRUCTION_LIST_SYN:
				{
					MSG_SYN_OBSTRUCTION_LIST *pSynMsg = (MSG_SYN_OBSTRUCTION_LIST*)pMsg;

					if( pSynMsg->mCount > 0)
					{
						for( unsigned int i=0; i<pSynMsg->mCount; ++i )
						{
							SCENEMAN->CreateDoor( pSynMsg->mObstructionIdx[i] );
						}
					}
				}
				break;
			case NM_OBSTRUCTION_DELETE_SYN:
				{
					MSG_SYN_OBSTRUCTION_DELETE *pSynMsg = (MSG_SYN_OBSTRUCTION_DELETE*)pMsg;
					cDoorSceneNode* pNode = (cDoorSceneNode*)SCENEMAN->GetManagedStaticNode( cSceneNode::eDOOR, pSynMsg->mObstructionIdx );
					if( pNode )
					{
						/// 삭제 처리
						pNode->Remove( THEAPP->GetWorldAccumTime() );
					}
				}
				break;
			default:
				assert(0);
			}
		}
		break;
	case NM_USERPORTAL:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_USERPORTAL_REGEN:
				{
					MSG_USERPORTAL_CREATE* pSynMsg = (MSG_USERPORTAL_CREATE*)pMsg;
					OBJECTMANAGER->AddUserPortal(pSynMsg->mObjectIdx, pSynMsg->mPlayerIdx, 
						pSynMsg->mMapChangePosIdx, NiPoint3(pSynMsg->mPosX, pSynMsg->mPosY, 0.0f) );
				}
				break;
			case NM_USERPORTAL_DELETE:
				{
					MSG_USERPORTAL_DELETE* pSynMsg = (MSG_USERPORTAL_DELETE*)pMsg;
					OBJECTMANAGER->RemoveUserPortal( pSynMsg->mObjectIdx );
				}
				break;
			case NM_USERPORTAL_SIGHT_IN_RES:
				{
					MSG_USERPORTAL_CREATE* pSynMsg = (MSG_USERPORTAL_CREATE*)pMsg;
					OBJECTMANAGER->AddUserPortal(pSynMsg->mObjectIdx, pSynMsg->mPlayerIdx, 
						pSynMsg->mMapChangePosIdx, NiPoint3(pSynMsg->mPosX, pSynMsg->mPosY, 0.0f) );
				}
				break;
			case NM_USERPORTAL_SIGHT_OUT_RES:
				{
					MSG_USERPORTAL_DELETE* pSynMsg = (MSG_USERPORTAL_DELETE*)pMsg;
					OBJECTMANAGER->RemoveUserPortal( pSynMsg->mObjectIdx );
				}
				break;
			default:
				assert(0);
			}
		}
		break;
	case NM_TUTORIAL:
		{
			TUTORIALMAN->NetworkMsgParser( pMsg );
		}
		break;
	case NM_TELEPORTPORTAL:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_TELEPORTPORTAL_LIST_SYN:
				{
					MSG_SYN_TELEPORTPORTAL_LIST* pSynMsg = (MSG_SYN_TELEPORTPORTAL_LIST*)pMsg;

					if( HERO )
					{
						unsigned short heroMap = HERO->GetMapNum();
						for( unsigned int i=0;i<pSynMsg->mCount;i++ )
						{
							sStageChangePos* p = STAGESCRIPT->GetPosScriptInfo( pSynMsg->mMapChangePosIdx[i] );
							if( p )
							{
								if( heroMap != p->mMapNumber )
								{
									assert(0);
									continue;
								}

								NiPoint3 pos = NiPoint3( p->mPosX, p->mPosY,0.0f );
								if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
								{
									assert(0);
									continue;
								}

								NiMatrix3 rot;
								rot.FromEulerAnglesXYZ( 0, 0, p->mRotAngle );
								SCENEMAN->CreateMapPortal( p->mPosIdx, pos, rot );
							}
						}
					}
					else
					{
						assert(0);
					}
				}
				break;
			case NM_TELEPORTPORTAL_CREATE_SYN:
				{
					MSG_SYN_TELEPORTPORTAL_CREATE* pSynMsg = (MSG_SYN_TELEPORTPORTAL_CREATE*)pMsg;

					if( HERO )
					{
						unsigned short heroMap = HERO->GetMapNum();
						sStageChangePos* p = STAGESCRIPT->GetPosScriptInfo( pSynMsg->mMapChangePosIdx );
						if( p )
						{
							if( heroMap == p->mMapNumber )
							{
								NiPoint3 pos = NiPoint3( p->mPosX, p->mPosY,0.0f );
								if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == true )
								{
									NiMatrix3 rot;
									rot.FromEulerAnglesXYZ( 0, 0, p->mRotAngle );
									SCENEMAN->CreateMapPortal( p->mPosIdx, pos, rot );
								}
								else
								{
									assert(0);
								}
							}
							else
							{
								assert(0);
							}
						}
					}
					else
					{
						assert(0);
					}
				}
				break;
			case NM_TELEPORTPORTAL_USE_RES:
				{
					/// 위치 이동
					MSG_RES_TELEPORTPORTAL_USE* pSynMsg = (MSG_RES_TELEPORTPORTAL_USE*)pMsg;

					/// 높이값 계산
					if( WORLDMAN == 0 )
					{
						assert(0);
						return;
					}

					///
					NiPoint3 pos( (float)pSynMsg->mDestX, (float)pSynMsg->mDestY, 0.0f );
					if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
					{
						assert(0);
						return;
					}

					//HERO->SetState( eOBJECT_STATE_IDLE );
					//HERO->SetPos_ResetCam( pos );

					HERO->SetPos_ResetCam( pos );
					if( HERO->GetState() != eOBJECT_STATE_DIE )
						HERO->SetStopFlag( eSTOP_NONE );
				}
				break;
			case NM_TELEPORTPORTAL_USE_SYN:
				{
					/// 위치 이동
					MSG_SYN_TELEPORTPORTAL_USE* pSynMsg = (MSG_SYN_TELEPORTPORTAL_USE*)pMsg;
					cPlayer* player = OBJECTMAN->GetPlayer( pSynMsg->mObjectIdx );
					if( player == 0 )
						return;

					/// 높이값 계산
					if( WORLDMAN == 0 )
					{
						assert(0);
						return;
					}

					///
					NiPoint3 pos( (float)pSynMsg->mDestX, (float)pSynMsg->mDestY, 0.0f );
					if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
					{
						assert(0);
						return;
					}

					player->SetState( eOBJECT_STATE_IDLE );
					player->SetPos( pos );
				}
				break;
			default:
				assert(0);
			}
		}
		break;
	case NM_NOTICE:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_NOTICE_NOTICE_SERVER_SYN:
				{
					MSG_SYN_NOTICE_SERVER* syn = (MSG_SYN_NOTICE_SERVER*)pMsg;

					CHATMANAGER->AddNoticeMsg( syn->mMessage, COLOR_YELLOW );
				}
				break;
			case NM_NOTICE_NOTICE_CHANNEL_SYN:
				{
					MSG_SYN_NOTICE_CHANNEL* syn = (MSG_SYN_NOTICE_CHANNEL*)pMsg;

					CHATMANAGER->AddNoticeMsg( syn->mMessage, COLOR_YELLOW );
				}
				break;
			case NM_NOTICE_NOTICE_AREA_SYN:
				{
					MSG_SYN_NOTICE_AREA* syn = (MSG_SYN_NOTICE_AREA*)pMsg;

					CHATMANAGER->AddNoticeMsg( syn->mMessage, COLOR_YELLOW );
				}
				break;
			case NM_NOTICE_DOWN_COUNT_NOTICE_SERVER_SYN:
				{
					MSG_SYN_NOTICE_DOWN_COUNT_SERVER* syn = (MSG_SYN_NOTICE_DOWN_COUNT_SERVER*)pMsg;
					charT msgText[16] = { 0,};
					_itow_s( syn->mCount, msgText, sizeof(msgText)/sizeof(charT), 10 );

					CHATMANAGER->AddNoticeMsg( msgText, COLOR_YELLOW );
				}
				break;
			case NM_NOTICE_DOWN_COUNT_NOTICE_CHANNEL_SYN:
				{
					MSG_SYN_NOTICE_DOWN_COUNT_CHANNEL* syn = (MSG_SYN_NOTICE_DOWN_COUNT_CHANNEL*)pMsg;
					charT msgText[16] = { 0,};
					_itow_s( syn->mCount, msgText, sizeof(msgText)/sizeof(charT), 10 );

					CHATMANAGER->AddNoticeMsg( msgText, COLOR_YELLOW );
				}
				break;
			}
		}
	}

	length = 0;
}

void cGameStage::Msg_MapChangeAck( MSGROOT* pMsg )
{
	MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsg;
	mpStageManager->SetMapIdx( pmsg->MapNum );
	mpStageManager->SetMapMode( pmsg->MapMode );

	mpStageManager->ChangeStage( eStage_MapChange );

	/// 스테이지 변경후 HERO객체 생성
	OBJECTMANAGER->AddHeroFromServerMsg( pMsg );

// #ifdef _GMTOOL
// 	/// GMTool 에서 일반 맵이 아닌경우 맵 이동 버튼 enable 시키기
// 	if( pmsg->IndunType != _E_ST_NORMAL_MAP_ )
// 		GMTOOL->SetMapTeleportEnable( false );
// 	else
// 		GMTOOL->SetMapTeleportEnable( true );
// #endif
}

void cGameStage::Msg_NpcGenerateInfo( MSGROOT* pMsg )
{
	MSG_NPC_INFO* pMessage = (MSG_NPC_INFO*)pMsg;

	for( int i = 0 ; i < (int)pMessage->mCount ; ++i )
	{
		/// 061122 PKH  서버에서 날라온 몬스터 갯수만큼 몬스터 추가
		sNpcData* npc = &pMessage->mNpcInfo[i];
		if( npc )
			OBJECTMANAGER->AddNpc( npc );
	}
}

void cGameStage::Msg_MonsterGenerateInfo( MSGROOT* pMsg )
{
	MSG_MONSTER_INFO* pMessage = (MSG_MONSTER_INFO*)pMsg;

	sMonsterData* monster = &pMessage->mMonsterInfo;
	sMonsterInfluence* pInfluence = pMessage->mInfluence;

	if( monster )
	{
		if( OBJECTMANAGER->AddMonster( monster ) == false )
		{
			assert(0);
			OBJECTMANAGER->RemoveMonster( monster->mMonsterIdx );
			return;
		}

		cMonster* pMonster = OBJECTMAN->GetMonster( monster->mMonsterIdx );
		if( pMonster )
		{
			for( unsigned long i = 0 ; i < pMessage->mCount ; ++i )
			{
				cInfluenceObject* p = SKILLMAN->CreateInfluenceObject( pInfluence[i].mUniqueIdx, 
					pInfluence[i].mInfluenceClassIdx, 0, pMonster, true );
				if( p )
				{
					p->StartProcess( -1, true );
				}
				else
				{
					assert(0);
					return;
				}
			}
		}
	}
}

void cGameStage::Msg_MonsterGenerateTeamInfo( MSGROOT* pMsg )
{
	MSG_MONSTERTEAM_INFO* pMessage = (MSG_MONSTERTEAM_INFO*)pMsg;

	sMonsterData* monster = &pMessage->mMonsterInfo;
	sMonsterInfluence* pInfluence = pMessage->mInfluence;

	if( monster )
	{
		if( OBJECTMANAGER->AddMonster( monster ) == false )
		{
			assert(0);
			OBJECTMANAGER->RemoveMonster( monster->mMonsterIdx );
			return;
		}

		cMonster* pMonster = OBJECTMAN->GetMonster( monster->mMonsterIdx );
		if( pMonster )
		{
			pMonster->SetPvPTeamType( pMessage->mTeamType );

			for( unsigned long i = 0 ; i < pMessage->mCount ; ++i )
			{
				cInfluenceObject* p = SKILLMAN->CreateInfluenceObject( pInfluence[i].mUniqueIdx, 
					pInfluence[i].mInfluenceClassIdx, 0, pMonster, true );
				if( p )
				{
					p->StartProcess( -1, true );
				}
				else
				{
					assert(0);
					return;
				}
			}
		}
	}
}

///
void cGameStage::MsgBoxEventParser( unsigned int event, unsigned int result, void* exData )
{
	switch( event )
	{
	case eMBEVENT_NONE:
		return;
		/// 친구추가
	case eMBEVENT_FRIEND_ADD:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 친추 확인
			cUIWindow* pWin = (cUIWindow*)UIMAN->GetTopMsgBox();
			if( !(pWin && pWin->IsKindof( eUINODE_MESSAGEBOX )) )
				return;

			cMsgBox* pBox = (cMsgBox*)pWin;
			/// 이름이 없으면 리턴
			if( pBox->GetLength() == 0 )
				return;

			/// 자기자신 이름과 동일하면 리턴
			int result = ::_tcscmp( HERO->GetName(), pBox->GetEditText() );
			if( result == 0 )
			{
				cStringT Message;
				Message.Format( GAMERESOURCEMAN->GetGameText( 42 ) );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
			}
			else
			{
				/// 서버에 친구 추가 요청
				MSG_REQ_FRIEND_ADD msg;
				::memset( &msg, 0, sizeof( msg ) );

				msg.Category = NM_CHAT;
				msg.Protocol = NM_CHAT_FRIEND_ADD_REQ;
				Sstrncpy( msg.Name, MAX_NAME_BUFFER_SIZE, pBox->GetEditText(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
		}
		break;
	case eMBEVENT_FRIEND_ADD_REQ:
		{
			cFriendWindow* pFriend = (cFriendWindow*)mpUI->GetFriendWindow();
			if( !pFriend )
			{
				assert(0);
				return;
			}

			/// 상대방으로부터 친구추가 요청
			if( result == eMESSAGE_BUTTON_YES )
			{
				/// 수락
				MSG_REQ_FRIEND_REP msg;
				::memset( &msg, 0, sizeof( msg ) );

				msg.Category = NM_CHAT;
				msg.Protocol = NM_CHAT_FRIEND_REP_REQ;
				msg.Idx = (unsigned long)exData;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			}
			else
			{
				/// 거부
				MSG_REQ_FRIEND_DEL msg;
				::memset( &msg, 0, sizeof( msg ) );

				msg.Category = NM_CHAT;
				msg.Protocol = NM_CHAT_FRIEND_DEL_REQ;
				msg.Idx = (unsigned long)exData;

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}	
		}
		break;
		/// 친구 삭제
	case eMBEVENT_FRIEND_DELETE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 삭제 확인
			cFriendWindow* pFriend = mpUI->GetFriendWindow();
			if( pFriend )
			{
				/// 서버에 친구 삭제 요청
				MSG_REQ_FRIEND_DEL msg;
				::memset( &msg, 0, sizeof( msg ) );

				msg.Category = NM_CHAT;
				msg.Protocol = NM_CHAT_FRIEND_DEL_REQ;
				msg.Idx = (long)exData;

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
		}
		break;
	case eMBEVENT_TAROTSEEKER_START:
		if( result == eMESSAGE_BUTTON_YES )
		{
			eTarotType type = TAROTMAN->GetTarotType();
			if( type == eTAROT_USER )
			{
				/// 점괘보기시작
				MSG_REQ_TAROT_SEEKER_JOIN msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_SEEKER_JOIN_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				cTarotWindow* win = mpUI->GetTarotWindow();
				if( win )
					win->NetLock( ePAGE_VIEW );
			}
			else if( type == eTAROT_SELF )
			{
				/// 점괘보기시작
				MSG_REQ_TAROT_SELF_JOIN msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_SELF_JOIN_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				cTarotWindow* win = mpUI->GetTarotWindow();
				if( win )
					win->NetLock( ePAGE_SELFVIEW );
			}
			else if( type == eTAROT_TUTORIAL )
			{
				MSG_REQ_TAROT_TUTORIAL_SEEKER_JOIN msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_JOIN_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				cTarotWindow* win = mpUI->GetTarotWindow();
				if( win )
					win->NetLock( ePAGE_VIEW );
			}
			else
				assert(0);
		}
		break;
	case eMBEVENT_TAROTSEEKER_EXIT:
		if( result == eMESSAGE_BUTTON_YES )
		{
			// 이용자
			eTarotType type = TAROTMAN->GetTarotType();
			if( type == eTAROT_USER )
			{
				MSG_REQ_TAROT_SEEKER_CLOSE msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_SEEKER_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			else if( type == eTAROT_SELF )
			{
				MSGROOT msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_SELF_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			else if( type == eTAROT_TUTORIAL )
			{
				MSG_REQ_TAROT_TUTORIAL_SEEKER_CLOSE msg;
				msg.Category = NM_TAROT;
				msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_CLOSE_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			else
			{
				assert(0);
				return;
			}

			cTarotWindow* win = mpUI->GetTarotWindow();
			if( win )
			{
				win->NetLock( ePAGE_SELECT );
				win->NetLock( ePAGE_SELFVIEW );
				win->NetLock( ePAGE_RESULT );
			}
		}
		break;
	case eMBEVENT_TAROTLEADER_EXIT:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 개설자가 타로점 종료시
			/// 점집 닫기
			MSG_REQ_TAROT_READER_CLOSE msg;
			msg.Category = NM_TAROT;
			msg.Protocol = NM_TAROT_READER_CLOSE_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			cTarotControlWindow* win = mpUI->GetTarotControlWindow();
			if( win )
				win->NetLock();
		}
		break;
	case eMBEVENT_NPCTAROT_START:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 참여
			MSG_REQ_NPC_TAROT_JOIN msg;
			msg.Category = NM_NPC;
			msg.Protocol = NM_NPC_TAROT_JOIN_REQ;
			msg.npcIndex = HERO->GetNpcIdx();
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			cTarotWindow* win = mpUI->GetTarotWindow();
			if( win )
				win->NetLock( ePAGE_NPCVIEW );
		}
		break;
	case eMBEVENT_NPCTAROT_EXIT:
		if( result == eMESSAGE_BUTTON_YES )
		{
			cTarotWindow* win = mpUI->GetTarotWindow();
			if( win )
				win->Hide();

			HERO->CloseNpcTalk();
			TAROTMAN->ChangePage( ePAGE_NONE );
		}
		break;
	case eMBEVENT_SKILL_BUY:
		{
			sExtraData* data = (sExtraData*)exData;
			if( data == 0 )
			{
				assert(0);
				return;
			}
			if( result == eMESSAGE_BUTTON_YES )
			{
				if( TUTORIALMAN->IsTutorialMode() == true )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6112 ) );
					return;
				}

				if( SKILLMAN->CheckBuySkill( data->skillIdx, data->skillStep ) == false )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6102 ) );
					assert(0);
					return;
				}

				/// packet
				MSG_REQ_SKILL_ADD msg;
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_ADD_REQ;
				msg.mSkillClassIdx = data->skillIdx;
				msg.mSkillStep = data->skillStep;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				cSkillWindow* win = mpUI->GetSkillWindow();
				if( win )
					win->NetLock();
			}
		}
		break;
	case eMBEVENT_PARTY_ADD_REQ:
		{
			/// 파티 가입요청 메세지에 대한 답 
			char reply = PARTY_REP_ACCEPT;
			if( result == eMESSAGE_BUTTON_YES )
			{
				reply = PARTY_REP_ACCEPT;
			}
			else if( result == eMESSAGE_AUTONO )
			{
				// 거절 - 응답시간 초과
				reply = PARTY_REP_REJECTTIME;
			}
			else if( result == eMESSAGE_BUTTON_NO )
			{
				reply = PARTY_REP_REJECT;
			}
			else
			{
				assert(0);
			}

			/// 가입요청 수락
			MSG_REQ_PARTY_REP msg;
			msg.Category = NM_PARTY;
			msg.Protocol = NM_PARTY_REP_REQ;
			msg.Reply = reply;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eMBEVENT_PARTY_CHANGELEADER_REQ:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 파티장 위임 메시지
			PARTYMAN->RequestChangeLeader();
		}
		break;
	case eMBEVENT_PARTY_THROW:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 파티원 추방
			PARTYMAN->RequestThrow();
		}
		break;
	case eMBEVENT_PARTY_OUT:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 파티 탈퇴
			PARTYMAN->RequestPartyOut();
		}
		break;
	case eMBEVENT_QUEST_ADDBYITEM:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 의뢰아이템 사용 여부
			QUESTMAN->QuestAddByItem( (unsigned short)exData );
		}
		break;
	case eMBEVENT_QUEST_DELETE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 퀘스트 포기 여부
			QUESTMAN->QuestDelete( (unsigned int)exData );
		}
		break;
	case eMBEVENT_QUEST_REWARDJOB:
	case eMBEVENT_QUEST_REWARDFORCE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 퀘스트 전직 & 세력 선택 확인
			QUESTMAN->QuestComplete();
		}
		break;
	case eMBEVENT_TRADE_REQUEST:
		/// 거래요청
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 승낙
			ITEMMAN->TryToTradeAccept();
		}
		else if( result == eMESSAGE_BUTTON_NO )
		{
			/// 거절
			ITEMMAN->TryToTradeReject( TYPE_ITEM_EXCHANGE_REP_REJECT );
		}
		else if( result == eMESSAGE_AUTONO )
		{
			// 응답시간 초과
			ITEMMAN->TryToTradeReject( TYPE_ITEM_EXCHANGE_REP_REJECTTIME );
		}
		else
			assert(0);
		break;
	case eMBEVENT_TRADE_MONEYIN:
		{
			cMoneyWindow* win = mpUI->GetMoneyWindow();
			if( !win )
				return;

			/// 창고에 입금할때 보유 한도 금액 초과시 
			if( result == eUIID_MONEY_OK )
			{
				if( win->GetMoney() > HERO->GetMoney() )
					win->SetMoney( HERO->GetMoney() );

				ITEMMAN->TryToTradeMoneyAdd( win->GetMoney() );
				win->NetLock();
			}
		}
		break;
	case eMBEVENT_MAIL_MONEYIN:
		{
			cMoneyWindow* win = mpUI->GetMoneyWindow();
			if( !win )
				return;

			/// 거래금액 입력
			if( result == eUIID_MONEY_OK )
			{
				if( win->GetMoney() > HERO->GetMoney() )
					win->SetMoney( HERO->GetMoney() );

				cMailWindow* mailWin = GAMEUI->GetMailWindow();
				if( mailWin )
				{
					cMailSendSheet* sheet = mailWin->GetSendSheet();
					if( sheet )
					{
						sheet->SetMoney( win->GetMoney() );
					}
				}
			}
		}
		break;
	case eMBEVENT_WAREHOUSE_MONEYIN:
		{
			cMoneyWindow* win = mpUI->GetMoneyWindow();
			if( !win )
				return;

			/// 창고 입금
			if( result == eUIID_MONEY_OK )
			{
				unsigned long nHaveWareMoney = HERO->GetDeposit();

				if( nHaveWareMoney + win->GetMoney() > MAX_HAVE_MONEY)
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11202 ), 
						GAMERESOURCEMAN->GetGameText(301)
						);

					return;
				}

				if( win->GetMoney() > HERO->GetMoney() )
					win->SetMoney( HERO->GetMoney() );

				if( win->GetMoney() == 0 )
					return;

				MSG_REQ_NPC_DEPOSIT msg;
				::memset( &msg, 0, sizeof(msg) );

				msg.Category = NM_NPC;
				msg.Protocol = NM_NPC_DEPOSIT_REQ;
				msg.npcIdx = HERO->GetNpcIdx();
				msg.money = win->GetMoney();
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 락
				win->NetLock( eMONEY_IN );
			}
		}
		break;
	case eMBEVENT_WAREHOUSE_MONEYOUT:
		{
			cMoneyWindow* win = mpUI->GetMoneyWindow();
			if( !win )
				return;

			/// 창고 출금
			if( result == eUIID_MONEY_OK )
			{

				if( win->GetMoney() > MAX_TRADE_MONEY )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11215 ), 
						GAMERESOURCEMAN->GetGameText(301)
						);

					return;

				}

				unsigned long nHaveInvenMoney = HERO->GetMoney();

				if( nHaveInvenMoney + win->GetMoney() > MAX_HAVE_MONEY)
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11216 ), 
						GAMERESOURCEMAN->GetGameText(301)
						);

					return;
				}

				if( win->GetMoney() > HERO->GetDeposit() )
					win->SetMoney( HERO->GetDeposit() );

				if( win->GetMoney() == 0 )
					return;

				MSG_REQ_NPC_WITHDRAW msg;
				::memset( &msg, 0, sizeof(msg) );

				msg.Category = NM_NPC;
				msg.Protocol = NM_NPC_WITHDRAW_REQ;
				msg.npcIdx = HERO->GetNpcIdx();
				msg.money = win->GetMoney();
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 락
				win->NetLock( eMONEY_OUT );
			}
		}
		break;
	case eMBEVENT_USERSELL_ITEMMONEYADD:
		{
			cMoneyWindow* moneyWin = mpUI->GetMoneyWindow();
			if( !moneyWin )
				return;

			cUserSellWindow* sellWin = mpUI->GetUserSellWindow();
			if( !sellWin )
				return;

			/// 아이템 추가
			if( result == eUIID_MONEY_OK )
			{
				if( moneyWin->GetMoney() == 0 )
					return;

				ITEMMAN->TryToUserSellAdd( moneyWin->GetSlotIndex(), moneyWin->GetMoney() );
			}	
		}
		break;
	case eMBEVENT_USERSELL_ITEMMONEYMODIFY:
		{
			cUserSellWindow* sellWin = mpUI->GetUserSellWindow();
			if( !sellWin )
				return;

			cMoneyWindow* moneyWin = mpUI->GetMoneyWindow();
			if( !moneyWin )
				return;

			unsigned short invenSlot = sellWin->GetSelectInvenSlot();
			if( invenSlot == USHRT_MAX )
				return;

			if( result == eUIID_MONEY_MODIFY )
			{
				if( moneyWin->GetMoney() == 0 )
				{
					/// 삭제
					ITEMMAN->TryToUserSellDelete( invenSlot );
				}
				else
				{
					/// 가격 변경
					ITEMMAN->TryToUserSellAdd( invenSlot, moneyWin->GetMoney() );
				}
			}
			else if( result == eUIID_MONEY_DELETE )
			{
				/// 삭제
				ITEMMAN->TryToUserSellDelete( invenSlot );
			}
		}
		break;
	case eMBEVENT_USERSELL_ITEMMONEYCANCEL:
		{
			cUserSellWindow* sellWin = mpUI->GetUserSellWindow();
			if( !sellWin )
				return;

			unsigned short invenSlot = sellWin->GetSelectInvenSlot();
			if( invenSlot == USHRT_MAX )
				return;

			sUserSellData* data = sellWin->GetSelectSellData();
			if( data )
			{
				/// 가격 원래대로 변경
				ITEMMAN->TryToUserSellAdd( invenSlot, data->mPrice );
			}
		}
		break;
	case eMBEVENT_ITEM_SELL:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			if( win && result == eUIID_ITEMMSG_YES )
			{
				ITEMMAN->TryToSellItem( (unsigned short)win->GetSlotIndex0(), win->GetCount() );
			}
		}
		break;
	case eMBEVENT_ITEM_DUMP:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			if( win && result == eUIID_ITEMMSG_YES )
				ITEMMAN->TryToDumpItem( (unsigned short)win->GetSlotIndex0() );
		}	
		break;
	case eMBEVENT_ITEM_BUY:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			if( win && result == eUIID_ITEMMSG_YES )
				ITEMMAN->TryToBuyItem( win->GetItemIndex(), win->GetCount() );
		}
		break;
	case eMBEVENT_ITEM_USERSELLBUY:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			if( win && result == eUIID_ITEMMSG_YES )
				ITEMMAN->TryToUserSellGet( (unsigned short)win->GetSlotIndex0(), win->GetCount() );
		}
		break;
	case eMBEVENT_ITEM_NPCDEALBUY:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			cNpcDealWindow* dealWin = mpUI->GetNpcDealWindow();
			if( dealWin )
			{
				cNpcDealSearchSheet* search = dealWin->GetSearchSheet();
				if( win && search && result == eUIID_ITEMMSG_YES )
				{
					search->SendDealBuy( win->GetSlotIndex0(), win->GetCount() );
				}
			}
		}
		break;
	case eMBEVENT_ITEMCOUNT_BUY:
		{
			cItemCountWindow* win = mpUI->GetItemCountWindow();
			if( win && result == eUIID_ITEMCOUNT_OK )
				mpUI->ShowItemBuyAskWindow( win->GetItemIndex(), (unsigned short)(win->GetCount()) );
		}
		break;
	case eMBEVENT_ITEMCOUNT_SELL:
		{
			cItemCountWindow* win = mpUI->GetItemCountWindow();
			if( win && result == eUIID_ITEMCOUNT_OK )
				mpUI->ShowItemSellAskWindow( (unsigned short)win->GetSlotIndex0(), (unsigned short)(win->GetCount()) );
		}
		break;
	case eMBEVENT_ITEMCOUNT_SPLIT:
		{
			cItemCountWindow* win = mpUI->GetItemCountWindow();
			if( win && result == eUIID_ITEMCOUNT_OK )
				ITEMMAN->TryToSplitItem( (unsigned short)win->GetSlotIndex0(), (unsigned short)win->GetSlotIndex1(), (unsigned short)(win->GetCount()) );
		}
		break;
	case eMBEVENT_ITEMCOUNT_USERSELLBUY:
		{
			cItemCountWindow* win = mpUI->GetItemCountWindow();
			if( win && result == eUIID_ITEMCOUNT_OK )
			{
				if( win->GetCount() == 0 )
					return;

				mpUI->ShowItemUserSellBuyAskWindow( (unsigned short)win->GetSlotIndex0(), win->GetItemIndex(), win->GetCount(), win->GetPrice() );
			}
		}
		break;
	case eMBEVENT_ITEMCOUNT_NPCDEALBUY:
		{
			cItemCountWindow* win = mpUI->GetItemCountWindow();
			if( win && result == eUIID_ITEMCOUNT_OK )
			{
				if( win->GetCount() == 0 )
					return;

				mpUI->ShowItemNpcDealBuyAskWindow( win->GetSlotIndex0(), win->GetItemIndex(), win->GetCount(), win->GetPrice() );
			}
		}
		break;
	case eMBEVENT_ITEM_SEAL:
		if( result == eMESSAGE_BUTTON_YES )
		{
			ITEMMAN->TryToUnSeal( (unsigned short)exData );
		}
		break;
	case eMBEVENT_ITEM_PUTCARD_NEW:
	case eMBEVENT_ITEM_PUTCARD_UPDATE:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			cPutCardWindow* put = mpUI->GetPutCardWindow();
			if( win && put && result == eUIID_ITEMMSG_YES )
				ITEMMAN->TryToPutCard( put->GetItemSlotIndex(), (unsigned short)win->GetSlotIndex0(), (unsigned short)win->GetSlotIndex1() );
		}
		break;
	case eMBEVENT_ITEM_CARDBOOK_TAROTNEW:
	case eMBEVENT_ITEM_CARDBOOK_TAROTRETURN:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			if( win && result == eUIID_ITEMMSG_YES )
			{
				ITEMMAN->TryToMoveItem( (unsigned short)win->GetSlotIndex0(), (unsigned short)win->GetSlotIndex1(), 0 );
			}
		}
		break;
	case eMBEVENT_ITEM_CARDBOOK_SPREADNEW:
	case eMBEVENT_ITEM_CARDBOOK_SPREADUPDATE:
		{
			cItemMsgWindow* win = mpUI->GetItemMsgWindow();
			if( win && result == eUIID_ITEMMSG_YES )
				ITEMMAN->TryToCardBookAdd( (unsigned short)win->GetSlotIndex0(), (unsigned short)win->GetSlotIndex1() );
		}
		break;
	case eMBEVENT_MAPCHANGE:
		{
			if( result == eMESSAGE_BUTTON_YES )
			{
				/// 자동사냥 신고 창이 뜬 도중이라면 무시
				cUIWindow* camaelWin = (cUIWindow*)GAMEUI->GetCamaelWindow();
				if( camaelWin && camaelWin->IsVisible() == true )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11817 ) );
					break;
				}

				HERO->CancelByMapchange();

				//
				MSG_REQ_PLAYER_MAPCHANGE Msg;
				Msg.Category = NM_PLAYER;
				Msg.Protocol = NM_PLAYER_MAPCHANGE_REQ;
				Msg.mMapChangePosIndex = (unsigned long)exData;

				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

				HERO->SetStopFlag( eSTOP_MAPCHANGE );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_MAPCHANGEWAIT, eSKIN_NONE,
					GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
					GAMERESOURCEMAN->GetGameText( 2 ) // 알림
					);
			}
			else
			{
				HERO->SetMapPortalSkipTime( 3000 );
			}
		}
		break;
	case eMBEVENT_USERMAPCHANGE:
		{
			if( result == eMESSAGE_BUTTON_YES )
			{
				HERO->CancelByMapchange();

				MSG_REQ_USERPORTAL_MAPCHANGE Msg;
				Msg.Category = NM_USERPORTAL;
				Msg.Protocol = NM_USERPORTAL_ENTER_REQ;
				Msg.mUserPortalObjectIdx = (unsigned long)exData;

				NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

				HERO->SetStopFlag( eSTOP_MAPCHANGE );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_MAPCHANGEWAIT, eSKIN_NONE,
					GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
					GAMERESOURCEMAN->GetGameText( 2 ) // 알림
					);
			}
			else
			{
				HERO->SetMapPortalSkipTime( 3000 );
			}
		}
		break;
	case eMBEVENT_REVIVAL_STAND:
		if( result == eMESSAGE_BUTTON_YES )
		{
			cRevivalWindow* win = mpUI->GetReVivalWindow();
			if( win )
				win->NetLock();

			/// 제자리부활
			MSG_REQ_PLAYER_RESURRECTION msg;
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_RESURRECTION_REQ;
			msg.mFlag = eRESURRECTIONTYPE_STAND;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eMBEVENT_REVIVAL_SKILL:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 스킬부활
			MSG_REQ_PLAYER_RESURRECTION msg;
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_RESURRECTION_REQ;
			msg.mFlag = eRESURRECTIONTYPE_SKILL;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
		/// 거래 불가 옵션 아이템 획득 경고
	case eMBEVENT_ITEM_NOTEXCHANGE_GET:
		if( result == eMESSAGE_BUTTON_YES )
		{
			cLootingWindow* looting = mpUI->GetLootingWindow();
			if( looting )
			{
				if( (unsigned int)exData == ITEM_GET_ALL )
				{
					/// 전체습득
					ITEMMAN->TryToGetItem( (unsigned int)ITEM_GET_ALL, 0 ); 
				}
				else
				{
					/// 개별습득
					sLootData* data = looting->GetSelectData();
					if( data )
					{
						ITEMMAN->TryToGetItem( data->itemDefineIndex, data->count );
					}
				}
			}
		}
		break;
	case eMBEVENT_GUILD_CREATE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_GUILD_INPUTNAME, eSKIN_EDIT,
				GAMERESOURCEMAN->GetGameText( 1234 ),	// 길드요청 메시지
				GAMERESOURCEMAN->GetGameText( 1233 ),	// 길드 생성
				(void*)exData, 0, 0, false, false, 1, MAX_GUILDNAME_SIZE );
		}		
		else
		{
			HERO->CloseNpcTalk();
		}
		break;
	case eMBEVENT_GUILD_INPUTNAME:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 길드생성 테스트
			cUIWindow* pWin = (cUIWindow*)UIMAN->GetTopMsgBox();
			if( pWin && pWin->IsKindof( eUINODE_MESSAGEBOX ) )
			{
				cMsgBox* pBox = (cMsgBox*)pWin;
				if( pBox->GetLength() > 0 )
				{
					/// 필터 검사
					if( FILTERMAN->CheckPartMatch( pBox->GetEditText() ) == true )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 336 ),
							GAMERESOURCEMAN->GetGameText( 301 ),
							0, (cUIWindow*)GAMEUI->GetTodayWordWindow() );
						
						///
						HERO->CloseNpcTalk();
						GUILDMAN->ClearTempData();
						return;
					}

					GUILDMAN->SetTempData( pBox->GetEditText(), (unsigned long)exData );

					/// 재확인
					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( 1235 ), pBox->GetEditText() );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_GUILD_CHECKNAME, eSKIN_YESNO,
						(LPCTSTR)str.Cstr(),					// 길드명 재확인
						GAMERESOURCEMAN->GetGameText( 1233 )	// 길드 생성
						);
				}			
				else
				{
					HERO->CloseNpcTalk();
					GUILDMAN->ClearTempData();
				}
			}
			
		}
		else
		{
			HERO->CloseNpcTalk();
			GUILDMAN->ClearTempData();
		}
		break;
	case eMBEVENT_GUILD_CHECKNAME:
		if( result == eMESSAGE_BUTTON_YES )
		{
			if( GUILDMAN->GuildCreate() == false )
				GUILDMAN->ClearTempData();
		}

		/// npc back 닫기
		HERO->CloseNpcTalk();
		break;
	case eMBEVENT_PVP_DM_NOTREADY:
		if( result == eMESSAGE_BUTTON_YES )
		{
			HERO->CancelByMapchange();

			/// pvp out 요청
			MSGROOT msg;
			msg.Category = NM_PVP;
			msg.Protocol = NM_PVP_DM_END_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			HERO->SetStopFlag( eSTOP_MAPCHANGE );

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_MAPCHANGEWAIT, eSKIN_NONE,
				GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
				GAMERESOURCEMAN->GetGameText( 2 ) // 알림
				);
		}
		break;
	case eMBEVENT_GUILD_ADD:
		{
			// 길드초대 받음
			char reply = GUILD_REPLAY_SUCCESS;
			if( result == eMESSAGE_BUTTON_YES )
			{
				reply = GUILD_REPLAY_SUCCESS;
			}
			else if( result == eMESSAGE_AUTONO )
			{
				// 응답 시간 초과
				reply = GUILD_REPLAY_REFUSETIME;
			}
			else if( result == eMESSAGE_BUTTON_NO )
			{
				reply = GUILD_REPLAY_REFUSE;
			}
			else
				assert(0);

			MSG_REQ_GUILD_REPLY msg;
			msg.Category = NM_GUILD;
			msg.Protocol = NM_GUILD_REPLY_REQ;
			msg.reply = reply;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eMBEVENT_GUILD_MEMBERDROP:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 길드원제명
			GUILDMAN->GuildCut( (unsigned long)exData );
		}
		break;
	case eMBEVENT_GUILD_OUT:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 길드 탈퇴
			GUILDMAN->GuildOut();
		}
		break;
	case eMBEVENT_GUILD_BREAK:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 길드 삭제
			GUILDMAN->GuildDelete();
		}
		break;
	case eMBEVENT_GUILD_ADDUSERNAME:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 길드초대할 유저명 입력
			cUIWindow* pWin = (cUIWindow*)UIMAN->GetTopMsgBox();
			if( pWin && pWin->IsKindof( eUINODE_MESSAGEBOX ) )
			{
				cMsgBox* pBox = (cMsgBox*)pWin;
				if( pBox->GetLength() > 0 )
				{
					GUILDMAN->GuildAdd( pBox->GetEditText() );
				}			
				else
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 30 ) );
				}
			}
		}
		break;
	case eMBEVENT_GUILD_GIVEPOSITION:
		if( result == eMESSAGE_BUTTON_YES )
		{
			/// 길드 직위 변경
			cGuildWindow* win = mpUI->GetGuildWindow();
			if( win )
			{
				sGuildUserInfo* info = win->GetSelectUserInfo();
				if( info )
				{
					GUILDMAN->GuildGivePosition( info->characterIdx, info->position, (char)exData );
				}
				else
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1242 ) );
				}
			}
		}
		break;
	case eMBEVENT_TODAYWORD_EXIT:
		if( result == eMESSAGE_BUTTON_YES )
		{
			mpUI->HideTodayWordWindow();
		}
		break;
	case eMBEVENT_ITEMMIX_GIVEUP:
		if( result == eMESSAGE_BUTTON_YES )
		{
			ITEMMIXMAN->ItemMixCancel( false );
		}
		break;
	case eMBEVENT_ITEMMIX_DELETE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			ITEMMIXMAN->ItemMixDelete( (unsigned char)exData );
		}
		break;
	case eMBEVENT_PLAYER_KICKED:
		{
			/// 프로그램 종료
			::PostMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	case eMBEVENT_SKILLRESET:
		if( result == eMESSAGE_BUTTON_YES )
		{
			ITEMMAN->TryToUseSkillReset( (unsigned short)exData );
		}
		break;
	case eMBEVENT_SKILLRESETPROCESS:
		break;
	case eMBEVENT_NPCDEAL_REGISTCANCEL:
		if( result == eMESSAGE_BUTTON_YES )
		{
			ITEMMAN->TryToNpcDealRegistDelete( (unsigned int)exData );
		}
		break;
	case eMBEVENT_NPCDEAL_ITEM_GET_FAIL:
		if( result == eMESSAGE_BUTTON_YES )
		{
			// 목록 재 요청
			cNpcDealWindow* dealWin = GAMEUI->GetNpcDealWindow();
			if( dealWin )
			{
				cNpcDealSearchSheet* searchSheet = dealWin->GetSearchSheet();
				if( searchSheet )
					searchSheet->SendUpdateList(0);
			}
		}
		break;
	case eMBEVENT_MAIL_GETMONEY:
		if( result == eMESSAGE_BUTTON_YES )
		{
			// 돈찾기
			cMailWindow* mailWin = GAMEUI->GetMailWindow();
			if( mailWin )
			{
				cMailReadSheet* mailReadSheet = mailWin->GetReadSheet();
				if( mailReadSheet )
					mailReadSheet->SendMoneyDownload();
			}
		}
		break;
	case eMBEVENT_MAIL_DELETE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			cMailWindow* mailWin = GAMEUI->GetMailWindow();
			if( mailWin )
			{
				cMailReadSheet* mailReadSheet = mailWin->GetReadSheet();
				if( mailReadSheet )
					mailReadSheet->SendDelete();
			}
		}	
		break;
	case eMBEVENT_MAIL_SELECT_DELETE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			cMailWindow* mailWin = GAMEUI->GetMailWindow();
			if( mailWin )
			{
				cMailListSheet* mailListSheet = mailWin->GetListSheet();
				if( mailListSheet )
					mailListSheet->SendSelectDelete();
			}
		}	
		break;
	case eMBEVENT_PVP_REMOVE_NPC:
		{
			if( HERO )
			{
				HERO->SetState( eOBJECT_STATE_IDLE );
				HERO->CloseNpcTalk();
			}
		}
		break;
	case eMBEVENT_PVP_READY_CANCEL:
		if( result == eMESSAGE_BUTTON_YES )
		{
			// 입장 취소
			MSGROOT msg;
			::memset( &msg, 0, sizeof(msg) );
			msg.Category = NM_PVP;
			msg.Protocol = NM_PVP_READYCANCEL_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eMBEVENT_PVP_REVIVAL:
		if( result == eMESSAGE_BUTTON_YES )
		{
			// 부활 요청
			MSGROOT msg;
			msg.Category = NM_PVP;
			msg.Protocol = NM_PVP_RESURRECTION_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eMBEVENT_SHORTCUT_OK:
		{
			if( result == eMESSAGE_BUTTON_YES )
			{
				SHORTCUTMAN->UpdateAllIconByKeyboard();

				if( mpUI )
				{
					cUIWindow* shortcutWin = (cUIWindow*)mpUI->GetShortcutWindow();
					if( shortcutWin )
						shortcutWin->Hide();
				}
			}
		}
		break;
	case eMBEVENT_SHORTCUT_CANCEL:
		{
			///
			if( result == eMESSAGE_BUTTON_YES )
			{
				cUIWindow* shortcutWindow = (cUIWindow*)mpUI->GetShortcutWindow();
				if( shortcutWindow && shortcutWindow->IsVisible() == true )
					shortcutWindow->Hide();
			}
		}
		break;
	case eMBEVENT_TUTORIAL_EXIT:
		if( result == eMESSAGE_BUTTON_YES )
		{
			// 기존 상태 캔슬
			HERO->CancelByMapchange();

			//
			MSGROOT msg;
			msg.Category = NM_TUTORIAL;
			msg.Protocol = NM_TUTORIAL_GIVEUP_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			// 맵이동 대기
			HERO->SetStopFlag( eSTOP_MAPCHANGE );

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_MAPCHANGEWAIT, eSKIN_NONE,
								GAMERESOURCEMAN->GetGameText( 67 ), // 인증 처리 진행
								GAMERESOURCEMAN->GetGameText( 2 ) // 알림
								);
		}
		break;
	case eMBEVENT_TUTORIAL_REVIVAL:
		{
			MSGROOT msg;
			msg.Category = NM_TUTORIAL;
			msg.Protocol = NM_TUTORIAL_REVIVAL_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eMBEVENT_VIDEO_CAPTURE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			if( RENDERSYS )
				RENDERSYS->ToggleVideoCapture();
		}
		break;
	case eMBEVENT_CAMAEL_KICK:
		{
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	case eMBEVENT_DELETE_CASHBUFF:
		if( result == eMESSAGE_BUTTON_YES )
		{
			MSG_REQ_INFLUENCE_DELETE Msg;
			Msg.Category = NM_SKILL;
			Msg.Protocol = NM_SKILL_INFLUENCE_DELETE_REQ;
			Msg.mUniqueIdx = (unsigned long)exData;
			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof( Msg ) );

			cDuelWindow* duelWin = GAMEUI->GetDuelWindow();
			if( duelWin )
				duelWin->NetLock();

			cBuffWindow* buffWin = GAMEUI->GetBuffWindow();
			if( buffWin )
				buffWin->NetLock();

			cBuffWindow* cashBuffWin = GAMEUI->GetCashBuffWindow();
			if( cashBuffWin )
				cashBuffWin->NetLock();
		}	
		break;
	case eMBEVENT_EXP_RECOVER:	// 경험치 복구 요청
		if( result == eMESSAGE_BUTTON_YES )		
		{
			ITEMMAN->TryToUseExpRecover( (unsigned short)exData );
		}
		break;
	case eMBEVENT_CASH_ITEM_USE:	// 소모성 캐시 아이템 사용
		if( result == eMESSAGE_BUTTON_YES )
		{
			ITEMMAN->TryToUseCashItem( (unsigned short)exData );
		}
		break;
	case eMBEVENT_ITEM_USE_INVENTORY:
		if( result == eMESSAGE_BUTTON_YES )
		{
			ITEMMAN->TryToUseInventory( (unsigned short)exData );
		}
		break;
	case eMBEVENT_THEME_RESET:
		if( result == eMESSAGE_BUTTON_YES )
		{
			cThemeResetWindow* resetWin = GAMEUI->GetThemeResetWindow();
			if( resetWin )
			{
				unsigned short slot = (unsigned short)exData;
				sPlayerThemeList* p = resetWin->GetSelectData();
				if( p )
				{
					if( ITEMMAN->TryToThemeReset( slot, p->mMapNumber, p->mThemeMode ) == false )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 225 ),
							GAMERESOURCEMAN->GetGameText( 301 ) );
					}
				}
				else
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 12010 ),
						GAMERESOURCEMAN->GetGameText( 301 ) );
				}
			}
			else
				assert(0);
		}
		break;
	case eMBEVENT_TWITTER_ERROR_LIST_UPDATE:
		break;
	case eMBEVENT_ITEM_PARTYGIVE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			cItemDivideWindow* win = GAMEUI->GetItemDivideWindow();
			if( !win )
			{
				assert(0);
				return;
			}

			if( ITEMMAN->TryToGiveParty( win->GetUserIndex(), win->GetItemIndex() ) == false )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 168 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}

			// 초기화
			win->Clear();
		}
		break;
	case eMBEVENT_NPCEVENT_ITEM_GET_FAIL:
		break;
/*	case eMBEVENT_ITEM_CASH_PULLCARD:
		{
			cPutCardWindow* put = mpUI->GetPutCardWindow();

			if( put && result == eUIID_ITEMMSG_YES )
			{
//				ITEMMAN->TryToCashPullCard( put->GetItemSlotIndex(), put->GetCardSlotNumber(), put->GetCashCardPullSlotIndex() );
			}
		}
		break;
*/
	default:
		assert(0);
		return;
	}
}
