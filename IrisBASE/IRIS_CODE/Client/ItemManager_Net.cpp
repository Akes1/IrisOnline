#include "stdafx.h"
#include "ItemManager.h"
#include "CoolTimeManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameUIManager.h"
#include "ChatManager.h"
#include "GameResourceManager.h"
#include "UIMsgboxEvent.h"
#include "NumberEditBox.h"
#include "Npc_Common.h"
#include "Monster.h"
#include "Player_Common.h"
#include "Trade_Common.h"
#include "UIRootNode.h"
//
#include "LootingWindow.h"
#include "NpcDisjointWindow.h"
#include "EnhancedWindow.h"
#include "TradeWindow.h"
#include "MoneyWindow.h"
#include "UserSellWindow.h"
#include "PutCardWindow.h"
#include "CardBookWindow.h"
#include "TarotSheetWindow.h"
#include "NpcDealWindow.h"
#include "NpcDealRegistSheet.h"
#include "NpcDealSearchSheet.h"
#include "ItemReceiptWindow.h"
#include "CashFreeMoveWindow.h"
#include "MegaphoneWindow.h"
#include "TwitterWindow.h"
#include "ItemMixWindow.h"
#include "MailWindow.h"
#include "SummonWindow.h"
//
#include "SoundSceneNode.h"
#include "SoundSystem.h"
#include "NaviMesh.h"
#include "OptionManager.h"
#include "SceneManager.h"

void cItemManager::NetworkMsgParserTrade( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
	case NM_TRADE_POST_SUMMON_OPEN_RES:
		{
			MSG_ERROR* msgData = (MSG_ERROR*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_TRADE_POST_SUMMON_OPEN_SUCCESS:
				{
					MSG_RES_TRADE_POST_SUMMON_OPEN* msgData = (MSG_RES_TRADE_POST_SUMMON_OPEN*)msg;

					if( HERO )
					{
						/// 아이템 갱신
						sInventory* inv = &msgData->inventory;
						if( inv->count > 0 )
						{
							NetSetItem( inv );													
							UpdateItem();
						}
						else
						{
							NetDumpItem( inv->number );
						}						

						/// 캐릭터 상태 변경
						HERO->SetStopFlag( eSTOP_SUMMON_PAPER );

						/// 우편함 열기 
						cMailWindow* pMailWin = GAMEUI->GetMailWindow();
						if( pMailWin == NULL )
						{
							assert(0);
							return;
						}

						/// npcidx == 0 주문서 사용에 의해 메일창 Open
						pMailWin->Show( 0 );	

					}
					else
					{
						assert(0);
						break;
					}
				}
				break;
			case ERROR_TRADE_POST_SUMMON_OPEN_USEFUL_SUCCESS:
				{
					if( HERO )
					{
						cSummonWindow* pSummonWin = GAMEUI->GetSummonWindow();
						if( pSummonWin )
						{
							pSummonWin->StartCoolTime();
						}
						else 
							assert(0);	

						/// 캐릭터 상태 변경
						HERO->SetStopFlag( eSTOP_SUMMON_PAPER );

						/// 우편함 열기 
						cMailWindow* pMailWin = GAMEUI->GetMailWindow();
						if( pMailWin == NULL )
						{
							assert(0);
							return;
						}

						/// npcidx == 0 주문서 사용에 의해 메일창 Open
						pMailWin->Show( 0 );	
					}
					else
					{
						assert(0);
						break;
					}					
				}
				break;
			case ERROR_TRADE_POST_SUMMON_OPEN_FAIL:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12800 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_TRADE_POST_SUMMON_OPEN_LICENSE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 293 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;	
			case ERROR_TRADE_POST_SUMMON_OPEN_ALREADY:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12083 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			}

			UnlockItemTry( ITEM_TRY_SUMMON );
			
			cSummonWindow* pSummonWin = GAMEUI->GetSummonWindow();
			if( pSummonWin )
			{
				pSummonWin->NetUnLock();
				pSummonWin->Clear();
				pSummonWin->Hide();
			}
		}
		break;
	case NM_TRADE_POST_SUMMON_OPEN_SYN:
		{
			MSG_SYN_TRADE_POST_SUMMON_OPEN* msgData = (MSG_SYN_TRADE_POST_SUMMON_OPEN*)msg;

			cPlayer* pPlayer = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( pPlayer != NULL )
			{								
				pPlayer->SetStopFlag( eSTOP_SUMMON_PAPER );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_TRADE_POST_SUMMON_CLOSE_SYN:
		{
			MSG_SYN_TRADE_POST_SUMMON_CLOSE* msgData = (MSG_SYN_TRADE_POST_SUMMON_CLOSE*)msg;			

			cPlayer* pPlayer = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( pPlayer != NULL )
			{	
				pPlayer->SetStopFlag( eSTOP_NONE );
			}
			else
			{
				assert(0);
			}
		}
		break;

	case NM_TRADE_AGENT_SUMMON_OPEN_RES:
		{
			MSG_ERROR* msgData = (MSG_ERROR*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_TRADE_AGENT_SUMMON_OPEN_SUCCESS:
				{
					MSG_RES_TRADE_AGENT_SUMMON_OPEN* msgData = (MSG_RES_TRADE_AGENT_SUMMON_OPEN*)msg;

					if( HERO )
					{			
						/// 아이템 갱신
						sInventory* inv = &msgData->inventory;
						if( inv->count > 0 )
						{
							NetSetItem( inv );													
							UpdateItem();
						}
						else
						{
							NetDumpItem( inv->number );
						}

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

						regist->Clear();

						// hero가 등록한 아이템 
						NPC_ITEM_AGENT_OPEN_RESULT* data = msgData->Agent;
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

						/// 캐릭터 상태 변경
						HERO->SetStopFlag( eSTOP_SUMMON_PAPER );	

						GAMEUI->UpdateInvenWithUI( (cUIWindow*)GAMEUI->GetNpcDealWindow() );						
						GAMEUI->ShowInventory();					
						GAMEUI->ShowNpcDealWindow();

					}
					else
					{
						assert(0);
						break;
					}
				}
				break;
			case ERROR_TRADE_AGENT_SUMMON_OPEN_USEFUL_SUCCESS:
				{
					if( HERO )
					{	
						cSummonWindow* pSummonWin = GAMEUI->GetSummonWindow();
						if( pSummonWin )
						{
							pSummonWin->StartCoolTime();
						}
						else 
							assert(0);	

						/// 캐릭터 상태 변경
						HERO->SetStopFlag( eSTOP_SUMMON_PAPER );	

						GAMEUI->UpdateInvenWithUI( (cUIWindow*)GAMEUI->GetNpcDealWindow() );						
						GAMEUI->ShowInventory();	
						GAMEUI->ShowNpcDealWindow();
					}
					else
					{
						assert(0);
						break;
					}					
				}
				break;			
			case ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12801 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_TRADE_AGENT_SUMMON_OPEN_LICENSE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 293 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_TRADE_AGENT_SUMMON_OPEN_ALREADY:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12084 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;

			}

			UnlockItemTry( ITEM_TRY_SUMMON );

			cSummonWindow* pSummonWin = GAMEUI->GetSummonWindow();
			if( pSummonWin )
			{
				pSummonWin->NetUnLock();
				pSummonWin->Clear();
				pSummonWin->Hide();
			}
		}
		break;
	case NM_TRADE_AGENT_SUMMON_OPEN_SYN:
		{
			MSG_SYN_TRADE_AGENT_SUMMON_OPEN* msgData = (MSG_SYN_TRADE_AGENT_SUMMON_OPEN*)msg;

			cPlayer* pPlayer = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( pPlayer != NULL )
			{								
				pPlayer->SetStopFlag( eSTOP_SUMMON_PAPER );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_TRADE_AGENT_SUMMON_CLOSE_SYN:
		{
			MSG_SYN_TRADE_AGENT_SUMMON_CLOSE* msgData = (MSG_SYN_TRADE_AGENT_SUMMON_CLOSE*)msg;

			cPlayer* pPlayer = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( pPlayer != NULL )
			{	
				pPlayer->SetStopFlag( eSTOP_NONE );
			}
			else
			{
				assert(0);
			}
		}
		break;

	}
}

void cItemManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
/*
	case NM_ITEM_INVENTORY_OPEN_RES:
		{
			MSG_ERROR* msgData = (MSG_ERROR*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_INVENTORY_OPEN_SUCCESS:

				{				
				}
				break;
			case ERROR_ITEM_INVENTORY_OPEN_FAIL:	
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 13117 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_INVENTORY_OPEN_ITEMPWD_REG:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_MAKE_CERTIFY_NUMBER, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 13104 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_INVENTORY_OPEN_ITEMPWD_CHECK:
				{
					if( HERO )
					{
						eITEMPWD_STATE nCertifyState = HERO->GetCertifyState();

						if( nCertifyState == eITEMPWD_NO_CHECK && HERO->IsCertifyUse() )
						{
							cCertifyWindow* pCertifyWin = GAMEUI->GetCertifyWindow();
							if( pCertifyWin == NULL )
							{
								assert(0);
								return;
							}

							pCertifyWin->Show( eCertify_State_Check );
						}
					}					
				}				
				break;
			}
		}
		break;
*/
	case NM_ITEM_MOV_INVENTORY_RES:
		{
			MSG_RES_ITEM_MOV_INVENTORY* msgData = (MSG_RES_ITEM_MOV_INVENTORY*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_MOV_INVENTORY_SUCCESS:
				NetTryItem( NM_ITEM_MOV_INVENTORY_RES );
				break;
			case ERROR_ITEM_MOV_INVENTORY_FAIL:
			case ERROR_ITEM_MOV_INVENTORY_STATE:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 222 ) );
				}
				break;
			case ERROR_ITEM_MOV_INVENTORY_NOTYET:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 223 ) );
				}
				break;
			case ERROR_ITEM_MOV_INVENTORY_MAX_ITEM:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 220 ) );
				}
				break;
			case ERROR_ITEM_MOV_INVENTORY_MERGE:
				{
					/// move 잠금 풀고 merge 잠금 걸기
					if( mTry == ITEM_TRY_MOVE )
					{
						mLocked = false;
						mTry = ITEM_TRY_NULL;
					}
					else
					{
						assert(0);
					}

					///
					mLocked = true;
					mTry = ITEM_TRY_MERGE;
				}
				break;
			default:
				{
					assert(0);
				}
				break;
			}

			switch( mTry )
			{
			case ITEM_TRY_MOVE:
				UnlockItemTry( ITEM_TRY_MOVE );
				break;
			case ITEM_TRY_EQUIP:
				UnlockItemTry( ITEM_TRY_EQUIP );
				break;
			case ITEM_TRY_UNEQUIP:
				UnlockItemTry( ITEM_TRY_UNEQUIP );
				break;
			case ITEM_TRY_SPLIT:
				UnlockItemTry( ITEM_TRY_SPLIT );
				break;
			}
		}
		break;
	case NM_ITEM_MERGE_INVENTORY_RES:
		{
			MSG_RES_ITEM_MERGE_INVENTORY* msgData = (MSG_RES_ITEM_MERGE_INVENTORY*)msg;

			if( msgData->ErrorCode == ERROR_ITEM_MERGE_INVENTORY_SUCCESS )
				NetTryItem( NM_ITEM_MERGE_INVENTORY_RES );
			else
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 224 ) );

			UnlockItemTry( ITEM_TRY_MERGE );
		}
		break;
	case NM_ITEM_USE_INVENTORY_RES:
		{
			MSG_RES_ITEM_USE_INVENTORY* msgData = (MSG_RES_ITEM_USE_INVENTORY*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_USE_INVENTORY_SUCCESS:
				NetTryItem( NM_ITEM_USE_INVENTORY_RES );
				break;
			case ERROR_ITEM_USE_INVENTORY_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 225 ));
				break;
			case ERROR_ITEM_USE_INVENTORY_NOTYET:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 223 ));
				break;
			case ERROR_ITEM_USE_INVENTORY_COOLTIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 226 ));
				break;
			case ERROR_ITEM_USE_INVENTORY_TITLE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 68 ));
				break;
			case ERROR_ITEM_USE_INVENTORY_SLOT:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 3102 ));
				break;
			case ERROR_ITEM_USE_SKILL_SYSTEM:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 225 ));
				break;
			case ERROR_ITEM_USE_SKILL_STATE:		// 대상의 상태가 안맞음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 274 ));
				break;
			case ERROR_ITEM_USE_SKILL_GROUPORDER:	// 효력그룹이 낮음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 275 ));
				break;
			case ERROR_ITEM_USE_INVENTORY_LICENSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ));
				break;
			case ERROR_ITEM_USE_EXP_RECOVER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11902 ));
				break;
			case ERROR_ITEM_USE_SKILL_DUPLICATE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12006 ),
					GAMERESOURCEMAN->GetGameText( 301 ) );
				break;
			case ERROR_ITEM_USE_SKILL_PCOVER:		// 일반 버프가 PC버프를 덮을수 없음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12004 ));
				break;
			case ERROR_ITEM_USE_SKILL_PCSCRIPT:		// PC방 버프끼리 중복불가 설정 스크립트 오류
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 225 ));
				break;
			default:
				assert(0);
				break;
			}

			UnlockItemTry( ITEM_TRY_USE );
		}
		break;
	case NM_ITEM_DEL_INVENTORY_RES:
		{
			MSG_RES_ITEM_DEL_INVENTORY* msgData = (MSG_RES_ITEM_DEL_INVENTORY*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_DEL_INVENTORY_SUCCESS:
				NetTryItem( NM_ITEM_DEL_INVENTORY_RES );
				break;
			case ERROR_ITEM_DEL_INVENTORY_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 227 ) );
				break;
			case ERROR_ITEM_DEL_INVENTORY_NOTYET:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 223 ) );
				break;
			default:
				assert(0);
				break;
			}

			UnlockItemTry( ITEM_TRY_DUMP );
		}
		break;
	case NM_ITEM_SWI_INVENTORY_RES:
		{
			MSG_RES_ITEM_SWI_INVENTORY* msgData = (MSG_RES_ITEM_SWI_INVENTORY*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_SWI_INVENTORY_SUCCESS:
				NetSwitchWeapon( msgData->activeWeapon );
				break;
			case ERROR_ITEM_SWI_INVENTORY_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 228 ) );
				HERO->SetWeaponChangeFlag( false );
				break;
			case ERROR_ITEM_SWI_INVENTORY_NOTYET:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 223 ) );
				HERO->SetWeaponChangeFlag( false );
				break;
			default:
				assert(0);
				break;
			}

			UnlockItemTry( ITEM_TRY_SWITCH );
		}
		break;
	case NM_ITEM_DIVIDE_INVENTORY_RES:
		{
			MSG_RES_ITEM_DIVIDE_INVENTORY* msgData = (MSG_RES_ITEM_DIVIDE_INVENTORY*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_DIVIDE_INVENTORY_SUCCESS:
				NetTryItem( NM_ITEM_DIVIDE_INVENTORY_RES, msgData );
				break;
			case ERROR_ITEM_DIVIDE_INVENTORY_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 229 ) );
				break;
			case ERROR_ITEM_DIVIDE_INVENTORY_NOTYET:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 223 ) );
				break;
			default:
				assert(0);
				break;
			}

			UnlockItemTry( ITEM_TRY_SPLIT );
		}
		break;
	case NM_ITEM_ENHANCED_START_RES:
		{
			MSG_ERROR* msgData = (MSG_ERROR*)msg;

			cEnhancedWindow* win = GAMEUI->GetEnhancedWindow();
			if( win )
				win->NetUnLock();

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_ENHANCED_START_SUCCESS:
				{
					if( win )
						win->StartEnhanced();
				}
				break;
			case ERROR_ITEM_ENHANCED_START_FAIL:
				{
					HERO->SetStopFlag( eSTOP_NONE );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 254 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, win	);
				}
				break;
			case ERROR_ITEM_ENHANCED_START_STATE:
				{
					HERO->SetStopFlag( eSTOP_NONE );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 272 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, win	);
				}
				break;
			case ERROR_ITEM_ENHANCED_START_LICENSE:
				{
					HERO->SetStopFlag( eSTOP_NONE );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 293 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, win	);
				}
				break;
			case ERROR_ITEM_ENHANCED_START_EQUIP_LEVEL_BY_SAFE:
				{
					HERO->SetStopFlag( eSTOP_NONE );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 12100 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, win	);

				}
				break;
			case ERROR_ITEM_ENHANCED_START_EQUIP_LEVEL_BY_CARD:
				{
					HERO->SetStopFlag( eSTOP_NONE );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 12500 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, win	);
				}
				break;
			}

			UnlockItemTry( ITEM_TRY_ENHANCEDSTART );
		}
		break;
	case NM_ITEM_ENHANCED_START_SYN:
		{
			MSG_SYN_ITEM_ENHANCED_START* msgData = (MSG_SYN_ITEM_ENHANCED_START*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( p != NULL )
			{
				p->SetStopFlag( eSTOP_ENHANCED );
			}
		}
		break;
	case NM_ITEM_ENHANCED_END_SYN:
		{
			/// 강화
			MSG_SYN_ITEM_ENHANCED_END* msgData = (MSG_SYN_ITEM_ENHANCED_END*)msg;

			HERO->SetStopFlag( eSTOP_NONE );
			HERO->ClearItemEnhanceEffect();

			cEnhancedWindow* enhance = GAMEUI->GetEnhancedWindow();
			if( enhance )
			{
				enhance->NetUnLock();
				enhance->StopEnhanced();
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_ENHANCED_END_SUCCESS:
				{
					NetEnhancedItem( (MSG_SYN_ITEM_ENHANCED_END*)msg );
				}
				break;
			case ERROR_ITEM_ENHANCED_END_FAIL:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 254 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, enhance	);
				}
				break;
			case ERROR_ITEM_ENHANCED_END_NOTYET:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 255 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, enhance	);
				}
				break;
			case ERROR_ITEM_ENHANCED_END_EQUIP_LEVEL_BY_SAFE:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 12100 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, enhance	);
				}
				break;
			case ERROR_ITEM_ENHANCED_END_EQUIP_LEVEL_BY_CARD:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 12500 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, enhance	);
				}
				break;
			}

			/// DB 대기중이었으면 풀기
			if( mTry == ITEM_TRY_ENHANCEDBWORK )
			{
				UnlockItemTry( ITEM_TRY_ENHANCEDBWORK );
			}
		}
		break;
	case NM_ITEM_ENHANCED_OTHEREND_SYN:
		{
			MSG_SYN_ITEM_ENHANCED_OTHEREND* msgData = (MSG_SYN_ITEM_ENHANCED_OTHEREND*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( p != NULL )
			{
				p->SetStopFlag( eSTOP_NONE );

				switch( msgData->ErrorCode )
				{
				case ERROR_ITEM_ENHANCED_SYN_SUCCESS: // 성공 
					{
						NiTransform trans;
						trans.MakeIdentity();
						trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );
						p->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_enchant_02.nif", &trans, false, true );
						p->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) 817 );
					}
					break;
				case ERROR_ITEM_ENHANCED_SYN_DOWN:		// 다운
				case ERROR_ITEM_ENHANCED_SYN_DESTROY:	// 파괴 
				case ERROR_ITEM_ENHANCED_SYN_KEEP:		// 변화없음
					{
						NiTransform trans;
						trans.MakeIdentity();
						trans.m_Translate = NiPoint3( 0.0f, 0.0f, 20.0f );
						p->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_enchant_03.nif", &trans, false, true );
						p->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) 818 );
					}
					break;
				case ERROR_ITEM_ENHANCED_SYN_ENDFAIL:	// 강화 안함 ( 취소됨 )
					break;
				}
			}
		}
		break;
	case NM_ITEM_ENHANCED_CANCEL_RES:
		{
			MSG_ERROR*	pmsg = (MSG_ERROR*)msg;

			if( pmsg->ErrorCode == ERROR_ITEM_ENHANCED_CANCEL_DBWORK )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 273 ) );
				ITEMMAN->LockItemTry( ITEM_TRY_ENHANCEDBWORK );
			}
		}
		break;
	case NM_ITEM_ENHANCED_CANCEL_SYN:
		{
			MSG_SYN_ITEM_ENHANCED_CANCEL* msgData = (MSG_SYN_ITEM_ENHANCED_CANCEL*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( p && p->GetStopFlag() == eSTOP_ENHANCED )
			{
				p->SetStopFlag( eSTOP_NONE );
			}
			else
				assert(0);
		}
		break;
	case NM_ITEM_ENHANCED_CANCELFORCE_SYN:
		{
			// 강제 취소
			if( HERO->GetStopFlag() == eSTOP_ENHANCED )
			{
				HERO->SetStopFlag( eSTOP_NONE );
				HERO->ClearItemEnhanceEffect();

				cEnhancedWindow* win = GAMEUI->GetEnhancedWindow();
				if( win )
				{
					win->StopEnhanced();
					win->Hide();
				}
			}
		}
		break;
	case NM_ITEM_DISJOINT_INVENTORY_RES:
		{
			/// 분해
			MSG_RES_ITEM_DISJOINT_INVENTORY* msgData = (MSG_RES_ITEM_DISJOINT_INVENTORY*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_DISJOINT_INVENTORY_SUCCESS:
				{
					NetTryItem( NM_ITEM_DISJOINT_INVENTORY_RES, msgData );
				}
				break;
			case ERROR_ITEM_DISJOINT_INVENTORY_FAIL:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 251 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case ERROR_ITEM_DISJOINT_INVENTORY_NOTYET:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 252 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case ERROR_ITEM_DISJOINT_INVENTORY_MAX_ITEM:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 220 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case ERROR_ITEM_DISJOINT_INVENTORY_LICENSE:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 293 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			}


			/// 아이템 분해 사운드 재생 취소하기.
			HERO->ClearItemDisjointSound();

			/// 윈도우 클리어
			cNpcDisjointWindow* p = GAMEUI->GetNpcDisjointWindow();
			if( p )
			{
				p->Clear();	/// 내부에서 netunlock
			}
			else
			{
				assert(0);
			}

			UnlockItemTry( ITEM_TRY_DISJOINT );
		}
		break;
	case NM_ITEM_DROP_IN_SYN:
		{
			MSG_ITEM_INFO*	msgData = (MSG_ITEM_INFO*)msg;
			if(	CreateDropItem( msgData ) == false )
			{
				assert(0);
			}
			else
			{
				/// 드랍된 아이템에 따른 사운드 재생.
				NiPoint3 trans;
				trans.x = msgData->itemDrop.xPos;
				trans.y = msgData->itemDrop.yPos;

				NAVIMESH->CalcHeight( &trans.z, trans.x, trans.y );

				unsigned long soundIndex = 2013;

				if( msgData->rare >= eTOOLTIP_RARE )
					soundIndex = 2014;

				/// 아이템 드랍 시 사운드 연출.
				sSoundInfo* pInfo = SOUNDSYS->GetSoundInfo( soundIndex );
				if( pInfo == 0 )
					return ;

				cSoundSceneNodeParam soundParam;
				if( pInfo->mRadius <= 0.0f )
				{
					soundParam.mRadius = 2000.0f;
				}
				else
				{
					soundParam.mRadius = pInfo->mRadius;
				}
				soundParam.mLoopCount = 1;
				soundParam.mVolumeRatio = OPTIONMAN->GetEffVolume() * pInfo->mVolume;

				/// 기본 속성 인자들을 설정
				soundParam.mPathName = pInfo->mFileName;
				soundParam.mTranslate = trans;
				soundParam.mpOwnerSceneNode = NULL;

				//cSoundSceneNode* pnode = SCENEMAN->CreateEffectSound( soundParam );
				SCENEMAN->CreateEffectSound( soundParam );
			}

		}
		break;
	case NM_ITEM_SIGHT_IN_RES:
		{
			MSG_ITEM_INFO*	msgData = (MSG_ITEM_INFO*)msg;
			if(	CreateDropItem( msgData ) == false )
			{
				assert(0);
			}
		}
		break;
	case NM_ITEM_DROP_OUT_SYN:
	case NM_ITEM_SIGHT_OUT_RES:
		{
			MSG_ITEMIDX* msgData = (MSG_ITEMIDX*)msg;
			DeleteDropItem( msgData->idx );
		}
		break;
	case NM_ITEM_GET_OPEN_RES:
		{
			/// 아이템 리스트 요청
			MSG_RES_ITEM_GET_OPEN* msgData = (MSG_RES_ITEM_GET_OPEN*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_GET_OPEN_SUCCESS:
				{
					/// UI 에 아이템 리스트 세팅
					cLootingWindow* looting = GAMEUI->GetLootingWindow();
					if( looting )
					{
						/// 데이터 삭제
						looting->Clear();
						looting->SetDropIndex( msgData->openItemIdx );

						sItemData* data = msgData->itemData;
						for( int i = 0; i < msgData->rowCount; ++i, ++data )
						{
							looting->AddItem( data );
						}

						GAMEUI->ShowLootingWindow();
					}
					else
					{
						assert(0);
					}
				}
				break;

				/// 상태... idle
			case ERROR_ITEM_GET_OPEN_FAIL:
				{
					/// 실패
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 210 ) );
				}
				break;
			case ERROR_ITEM_GET_OPEN_OWNER:
			case ERROR_ITEM_GET_OPEN_PARTY:
				{
					/// 소유권오류
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 211 ) );
				}
				break;
			case ERROR_ITEM_GET_OPEN_DISTANCE:
				{
					/// 거리오류
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 212 ) );
				}
				break;
			case ERROR_ITEM_GET_OPEN_EXIST:
				{
					/// 열람중오류
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 213 ) );
				}
				break;
			default:
				assert(0);
				break;
			}

			UnlockItemTry( ITEM_TRY_PICK );
		}
		break;
	case NM_ITEM_GET_OPEN_SYN:
		{
			MSG_SYN_ITEM_GET_OPEN* msgData = (MSG_SYN_ITEM_GET_OPEN*)msg;
			cPlayer* player = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( player )
				player->SetState( ePLAYER_STATE_ITEMPICK );
		}
		break;
	case NM_ITEM_GET_CLOSE_RES:
		{
			/// 무조건 상자 닫기
			cLootingWindow* looting = GAMEUI->GetLootingWindow();
			if( looting )
			{
				/// 닫기만한다 ( 닫기 패킷은 안보냄 )
				looting->Close();
				looting->NetUnLock( eLOCK_CLOSE );

				/// 상태... idle
				if( HERO->GetState() == ePLAYER_STATE_ITEMPICK )
					HERO->SetState( eOBJECT_STATE_IDLE );
			} 
			else
				assert(0);
		}
		break;
	case NM_ITEM_GET_CLOSE_SYN:
		{
			MSG_SYN_ITEM_GET_CLOSE* msgData = (MSG_SYN_ITEM_GET_CLOSE*)msg;

			cPlayer* player =OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( player )
			{
				/// 상태... idle
				if( player->GetState() == ePLAYER_STATE_ITEMPICK )
					player->SetState( eOBJECT_STATE_IDLE );
			}
		}
		break;
	case NM_ITEM_GET_RES:
		{
			/// 아이템 획득
			MSG_RES_ITEM_GET* msgData = (MSG_RES_ITEM_GET*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_GET_SUCCESS:
				NetTryItem( NM_ITEM_GET_RES, msgData );
				break;
			case ERROR_ITEM_GET_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_GETITEM, GAMERESOURCEMAN->GetGameText( 234 ) );
				break;
			case ERROR_ITEM_GET_AUTO:	/// 자동습득인 경우 처리
				NetGetItem( (MSG_RES_ITEM_GET*)msg );
				return;
			default:
				assert( 0 && "network error!" );
				break;
			}

			UnlockItemTry( ITEM_TRY_GET );

			cLootingWindow* win = GAMEUI->GetLootingWindow();
			if( win )
			{
				win->NetUnLock( eLOCK_GETITEM );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_ITEM_GET_LEAVE_RES:
		{
			/// 아이템 획득후, 루팅창에 남은 상태 
			MSG_RES_ITEM_GET_LEAVE* msgData = (MSG_RES_ITEM_GET_LEAVE*)msg;

			/// 남은 내용 세팅
			cLootingWindow* looting = GAMEUI->GetLootingWindow();
			if( looting )
			{
				/// 데이터 삭제
				looting->Clear();

				sItemData* data = msgData->itemData;
				for( int i = 0; i < msgData->rowCount; ++i, ++data )
				{
					looting->AddItem( data );
				}

				if( msgData->rowCount <= 0 )
				{
					looting->Hide();
					if( HERO->GetState() == ePLAYER_STATE_ITEMPICK )
						HERO->SetState( eOBJECT_STATE_IDLE );
				}
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_ITEM_GATHERING_OPEN_RES:
		{
			/// 채집 상자열기
			MSG_RES_ITEM_GATHERING_OPEN* msgData = (MSG_RES_ITEM_GATHERING_OPEN*)msg;

			/// UI 에 아이템 리스트 세팅
			cLootingWindow* looting = GAMEUI->GetLootingWindow();
			if( looting )
			{
				/// 데이터 삭제
				looting->Clear();

				sItemData* data = msgData->itemData;
				for( int i = 0; i < msgData->rowCount; ++i, ++data )
				{
					looting->AddItem( data );
				}

				GAMEUI->ShowLootingWindow();
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_ITEM_USE_RES:
		{
			/// 아이템이 사용됨 ( 스킬에 의한 사용등 )
			MSG_RES_ITEM_USE* msgData = (MSG_RES_ITEM_USE*)msg;

			sInventory* inv = &msgData->inventory;
			if( inv->count > 0 )
			{
				NetSetItem( inv );

				/// 쿨타임매니저에 사용 등록
				cItem& item = mItemArray[inv->number];
				if( item.mDefine )
				{
					cItemAbility* itemAbil = item.mDefine->GetAbililty();
					if( itemAbil && itemAbil->GetCoolTime() > 0 )
					{
						COOLMAN->StartCoolTime( item.mIndex );
					}
				}

				/// 아이템관련 정보 갱신
				UpdateItem();
			}
			else
			{
				NetDumpItem( inv->number );
			}

			/// 스킬사용으로 인한 아이템 사용이므로, unlock 하면 안됨!!! 
			/// lock, unlock 은 인벤토리,창고 등에서 아이템 에 관한 내용일때만 사용
		}
		break;
	case NM_ITEM_REWARD_RES:
		{
			/// 퀘스트로 인한 LOCK은 퀘스트에서 푼다
			/// 에러메세지는 퀘스트 보상에서 처리
			/// 아이템 획득 - 퀘스트 보상, 신규퀘스트시 아이템 제공
			MSG_RES_ITEM_REWARD* msgData = (MSG_RES_ITEM_REWARD*)msg;
			NetTryItem( NM_ITEM_REWARD_RES, msgData );
		}
		break;
	case NM_ITEM_TAKE_RES:
		{
			/// 퀘스트로 인한 LOCK은 퀘스트에서 푼다
			/// 아이템 회수 
			MSG_RES_ITEM_TAKE* msgData = (MSG_RES_ITEM_TAKE*)msg;
			NetTryItem( NM_ITEM_TAKE_RES, msgData );
		}
		break;
	case NM_ITEM_GET_QUEST_RES:
		{
			/// 아이템 자동습득이므로 lock 안함
			MSG_RES_ITEM_GET_QUEST* msgData = (MSG_RES_ITEM_GET_QUEST*)msg;
			bool isItemGet = false;

			sInventory* inv = msgData->Inventory;
			for ( long i = 0; i < msgData->RowCount; i++, inv++ )
			{
				/// 줍기전의 갯수 확인하기
				cItem& item = mItemArray[inv->number];

				/// 추가된 갯수 구하기
				int addcount = inv->count - item.mCount;

				/// 갯수 및 무게 추가 안함
				NetSetItem( inv );

				/// 아이템 획득메세지 쏘기 
				cItemDefine* itemDefine = GetItemDefine( item.mIndex );
				if( itemDefine )
				{
					isItemGet = true;
					cStringT str;
					LPCTSTR countStr = cNumberEditBox::FormatNumber( addcount );

					str.Format( GAMERESOURCEMAN->GetGameText( 240 ), countStr );
					CHATMANAGER->AddItemMsg( item.mIndex, inv->number, GAMERESOURCEMAN->GetGameText( 245 ), (LPCTSTR)str.Cstr(), true );
				}
			}

			/// 퀘스트 아이템 습득 이펙트 처리
			if( isItemGet )
			{
				NiTransform trans;
				trans.MakeIdentity();
				trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );

				HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_itemget_03.nif", &trans, false, true );
				SOUNDSYS->Play2DSound( 2017 );
			}

			/// 아이템관련 정보 갱신
			UpdateItem();
		}
		break;
	case NM_ITEM_EXCHANGE_ASK_RES:
		{
			cStringT str;

			/// 거래신청 결과
			MSG_RES_ITEM_EXCHANGE_ASK* msgData = (MSG_RES_ITEM_EXCHANGE_ASK*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_EXCHANGE_ASK_SUCCESS:	// 성공 - 신청후 대기중
				str.Format( GAMERESOURCEMAN->GetGameText( 2000 ), mTradeName.Cstr() );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
				break;
			case ERROR_ITEM_EXCHANGE_ASK_FAIL:		// 실패 - 대상이 없거나 오류인경우
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2015 ) );
				break;
			case ERROR_ITEM_EXCHANGE_ASK_ERROR:		// 실패 - 거래할수 없는 상태
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2020 ) );
				break;
			case ERROR_ITEM_EXCHANGE_ASK_DISTANCE:	// 실패 - 거리
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2017 ) );
				break;
			case ERROR_ITEM_EXCHANGE_ASK_OPTIONREFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2023 ) );
				break;
			case ERROR_ITEM_EXCHANGE_ASK_TARGETOPTIONREFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2700 ) );
				break;
			case ERROR_ITEM_EXCHANGE_ASK_TARGETERROR: // 대상 상태 오류
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2016 ) );
				break;
			case ERROR_ITEM_EXCHANGE_ASK_REPLAY:	// 답변요청
				{
					/// 상대방으로부터 거래요청이 옴 
					/// 거래요청 메세지 박스 띄우기
					cPlayer* target = OBJECTMAN->GetPlayer( msgData->characterIdx );
					if( !target )
					{
						assert(0);
						return;
					}

					/// 
					str.Format( GAMERESOURCEMAN->GetGameText( 2001 ), target->GetName() );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_TRADE_REQUEST, eSKIN_YESNO,
						(LPCTSTR)str.Cstr(),			
						GAMERESOURCEMAN->GetGameText( 301 ),
						0, 0, TRADE_WAIT_TIME, false, false );

					/// 상대정보세팅
					mTradeTarget = msgData->characterIdx;
					mTradeName = target->GetName();
				}
				break;
			}
		}
		break;
	case NM_ITEM_EXCHANGE_REP_RES:
		{
			/// 거래요청 응답에 관한 결과
			MSG_RES_ITEM_EXCHANGE_REP* msgData = (MSG_RES_ITEM_EXCHANGE_REP*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_EXCHANGE_REP_SUCCESS:
				{
					/// 거래 시작 
					cTradeWindow* trade = GAMEUI->GetTradeWindow();
					if( trade )
					{	
						cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
						if( inven && inven->IsVisible() )
							UIMAN->SetRestoreShow( inven );

						trade->SetNames( (LPCTSTR)mTradeName.Cstr() );
						trade->Show();

						// 순서 주의
						GAMEUI->ShowInventory();
					}
					else
					{
						assert(0);
					}
				}
				break;
			case ERROR_ITEM_EXCHANGE_REP_REFUSE:
				break;
			case ERROR_ITEM_EXCHANGE_REP_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2020 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_ERROR:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2002 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_TARGETERROR:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2016 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_NOTEXIST:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2004 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_DISTANCE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2017 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_OPTIONREFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2023 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_TARGETOPTIONREFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2700 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_TIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2005 ) );
				break;
			}
		}
		break;
	case NM_ITEM_EXCHANGE_REP_SYN:
		{
			/// 거래 신청자에게 결과 옴( 상대방의 응답관련 )
			MSG_RES_ITEM_EXCHANGE_REP* msgData = (MSG_RES_ITEM_EXCHANGE_REP*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_EXCHANGE_REP_SUCCESS:
				{
					/// 거래 시작 
					cTradeWindow* trade = GAMEUI->GetTradeWindow();
					if( trade )
					{	
						cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
						if( inven && inven->IsVisible() )
							UIMAN->SetRestoreShow( inven );

						trade->SetNames( (LPCTSTR)mTradeName.Cstr() );
						trade->Show();

						// 순서 주의
						GAMEUI->ShowInventory();
					}
					else
					{
						assert(0);
					}
				}
				break;
			case ERROR_ITEM_EXCHANGE_REP_TARGETREFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2003 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_FAIL:
			case ERROR_ITEM_EXCHANGE_REP_ERROR:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2016 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_TARGETERROR:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2020 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_NOTEXIST:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2002 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_DISTANCE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2017 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_OPTIONREFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2700 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_TARGETOPTIONREFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2023 ) );
				break;
			case ERROR_ITEM_EXCHANGE_REP_TIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2005 ) );
				break;
			}
		}
		break;
	case NM_ITEM_EXCHANGE_ADD_RES:
		{
			/// 거래아이템 추가 결과
			MSG_RES_ITEM_EXCHANGE_ADD* msgData = (MSG_RES_ITEM_EXCHANGE_ADD*)msg;

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( !trade )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_EXCHANGE_ADD_SUCCESS:		/// 등록완료
				if( mTry == ITEM_TRY_TRADEADD )
				{
					/// 아이템 등록이었으면,
					if( trade && trade->AddMyItem( mSlotIndex0 ) == false )
					{
						assert(0);
					}

					mSlotIndex0 = 0;
				}
				else
				{
					/// 금액 등록이었으면,
					if( trade )
						trade->AddMyMoney( mTradeMoney );
				}
				break;
			case ERROR_ITEM_EXCHANGE_ADD_FAIL:	/// 실패
				if( mTry == ITEM_TRY_TRADEADD )
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2008 ) );
				else
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2018 ) );
				break;

			case ERROR_ITEM_EXCHANGE_ADD_TRADE_MONEY: /// 트레이드 시도시 최대 거래 한도(10억) 초과시
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11206 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_EXCHANGE_ADD_EXCHANGE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 2019 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
				break;
			}

			/// 락풀기
			if( mTry == ITEM_TRY_TRADEADD )
			{
				UnlockItemTry( ITEM_TRY_TRADEADD );
				if( trade )
					trade->NetUnLock();
			}
			else
			{
				/// 금액 윈도우 락 풀기
				cMoneyWindow* win = GAMEUI->GetMoneyWindow();
				if( win )
					win->NetUnLock();
			}
		}
		break;
	case NM_ITEM_EXCHANGE_ADD_SYN:
		{
			/// 상대방 아이템 추가
			MSG_SYN_ITEM_EXCHANGE_ADD* msgData = (MSG_SYN_ITEM_EXCHANGE_ADD*)msg;

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( trade )
			{
				/// 아이템 등록
				if( msgData->type == TYPE_ITEM_EXCHANGE_ADD_ITEM )
				{
					trade->AddThatItem( &msgData->inventory );
				}
				/// 금액 등록
				else
				{
					trade->AddThatMoney( msgData->money );
				}
			}
			else
				assert(0);
		}
		break;
	case NM_ITEM_EXCHANGE_DEL_RES:
		{
			/// 아이템 삭제 결과
			MSG_RES_ITEM_EXCHANGE_DEL* msgData = (MSG_RES_ITEM_EXCHANGE_DEL*)msg;

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( !trade )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_EXCHANGE_DEL_SUCCESS:
				if( mTry == ITEM_TRY_TRADEDELETE )
				{
					/// 아이템 삭제
					if( trade && trade->DeleteMyItem( (unsigned short)mTempIndex ) == false )
					{
						assert(0);
						return;
					}
				}
				break;
			case ERROR_ITEM_EXCHANGE_DEL_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2009 ) );
				break;
			}

			UnlockItemTry( ITEM_TRY_TRADEDELETE );
			if( trade )
				trade->NetUnLock();
		}
		break;
	case NM_ITEM_EXCHANGE_DEL_SYN:
		{
			/// 상대방 아이템 삭제 
			MSG_SYN_ITEM_EXCHANGE_DEL* msgData = (MSG_SYN_ITEM_EXCHANGE_DEL*)msg;

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( trade )
			{
				/// 아이템 삭제
				trade->DeleteThatItem( &msgData->inventory );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_ITEM_EXCHANGE_OK_RES:
		{
			/// 거래물품 확인 
			MSG_RES_ITEM_EXCHANGE_OK* msgData = (MSG_RES_ITEM_EXCHANGE_OK*)msg;

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( trade )
			{
				if( msgData->ErrorCode != ERROR_ITEM_EXCHANGE_OK_PLAYER )
					trade->NetUnLock();

				switch( msgData->ErrorCode )
				{
				case ERROR_ITEM_EXCHANGE_OK_SUCCESS:
					{
						/// 물품확인 처리
						trade->SetStatus( ItemExchangeOk, true );
					}
					break;
				case ERROR_ITEM_EXCHANGE_OK_FAIL:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2010 ) );

						/// 물품수정상태 처리
						trade->SetStatus( ItemExchangeBegin, true );
					}
					break;
				case ERROR_ITEM_EXCHANGE_OK_PLAYER:		/// 상대방의 물품 확인 됨
					{
						trade->SetStatus( ItemExchangeOk, false );
					}
					break;
				}
			}
			else
				assert(0);
		}
		break;
	case NM_ITEM_EXCHANGE_RETRY_RES:
		{
			/// 거래물품 수정
			MSG_RES_ITEM_EXCHANGE_RETRY* msgData = (MSG_RES_ITEM_EXCHANGE_RETRY*)msg;

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( trade )
			{
				if( msgData->ErrorCode != ERROR_ITEM_EXCHANGE_RETRY_PLAYER )
					trade->NetUnLock();

				switch( msgData->ErrorCode )
				{
				case ERROR_ITEM_EXCHANGE_RETRY_SUCCESS:
					{
						trade->SetStatus( ItemExchangeBegin, true );
						trade->SetStatus( ItemExchangeBegin, false );
					}
					break;
				case ERROR_ITEM_EXCHANGE_RETRY_FAIL:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2011 ) );
						trade->SetStatus( ItemExchangeOk, true );
					}
					break;
				case ERROR_ITEM_EXCHANGE_RETRY_PLAYER:	/// 상대방의 물품 수정상태 됨
					{
						trade->SetStatus( ItemExchangeBegin, true );
						trade->SetStatus( ItemExchangeBegin, false );
					}
					break;
				}	
			}
			else
				assert(0);
		}
		break;
	case NM_ITEM_EXCHANGE_CANCEL_RES:
		{
			MSG_RES_ITEM_EXCHANGE_CANCEL* msgData = (MSG_RES_ITEM_EXCHANGE_CANCEL*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_EXCHANGE_CANCEL_SUCCESS:
				break;
			case ERROR_ITEM_EXCHANGE_CANCEL_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2007 ) );
				break;
			case ERROR_ITEM_EXCHANGE_CANCEL_PLAYER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2006 ) );
				break;
			}

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( !trade )
			{
				assert(0);
			}

			if( msgData->ErrorCode != ERROR_ITEM_EXCHANGE_CANCEL_FAIL )
			{
				/// 거래 닫기
				if( trade )
					trade->Hide();

				/// 클리어
				mTradeName.Clear();
				mTradeTarget = 0;
			}

			if( trade && msgData->ErrorCode != ERROR_ITEM_EXCHANGE_CANCEL_PLAYER )
				trade->NetUnLock();
		}
		break;
	case NM_ITEM_EXCHANGE_END_RES:
		{
			/// 거래 종료
			MSG_RES_ITEM_EXCHANGE_END* msgData = (MSG_RES_ITEM_EXCHANGE_END*)msg;

			cTradeWindow* trade = GAMEUI->GetTradeWindow();
			if( !trade )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_EXCHANGE_END_SUCCESS:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2012 ) );

					/// 거래 아이템 삽입 및 거래된 아이템 삭제
					NetTradeItem( msgData );

					/// 거래윈도우 닫기
					if( trade )
						trade->Hide();
				}
				break;
			case ERROR_ITEM_EXCHANGE_END_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2013 ) );
				break;
			case ERROR_ITEM_EXCHANGE_END_THATMONEY_FAIL:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						(LPCTSTR)GAMERESOURCEMAN->GetGameText(11207),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, trade );					
				}				
				break;
			case ERROR_ITEM_EXCHANGE_END_MYMONEY_FAIL:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						(LPCTSTR)GAMERESOURCEMAN->GetGameText(11201),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, trade );
				}
				break;
			case ERROR_ITEM_EXCHANGE_END_INVENTORY:
				{
					/// max 인벤토리로 인한 에러
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2021 ) );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 2021 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, trade );
				}
				break;
			case ERROR_ITEM_EXCHANGE_END_WAIT:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2014 ) );
				break;
			case ERROR_ITEM_EXCHANGE_END_MY_MAX_ITEM:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 2025 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, trade );
				}
				break;
			case ERROR_ITEM_EXCHANGE_END_THAT_MAX_ITEM:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 2026 ),
						GAMERESOURCEMAN->GetGameText( 301 ), 0, trade );
				}
				break;
			}

			if( trade )
				trade->NetUnLock();

			if( msgData->ErrorCode == ERROR_ITEM_EXCHANGE_END_WAIT )
			{
				if( trade )
					trade->SetStatus( ItemExchangeEnd, true );
			}
		}
		break;
	case NM_ITEM_STALL_SELL_OPEN_RES:
		{
			/// 노점상 오픈 요청 결과
			MSG_RES_ITEM_STALL_SELL_OPEN* msgData = (MSG_RES_ITEM_STALL_SELL_OPEN*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_OPEN_SUCCESS:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2500 ) );

					/// 판매노점상 open 상태로 함
					if( userSell )
					{
						cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
						if( inven && inven->IsVisible() )
							UIMAN->SetRestoreShow( inven );

						userSell->SetStatus( eUSERSELL_SELL );
						userSell->SetOpen();
						userSell->SetName( (LPTSTR)mUserSellName.Cstr() );
						userSell->Show();

						// 순서 주의
						GAMEUI->ShowInventory();
					}

					/// 상태 변경
					HERO->SetStopFlag( eSTOP_OPENSTALL );
					HERO->ActiveUserSellBubble( (LPTSTR)mUserSellName.Cstr(), COLOR_GREEN );
				}
				break;
			case ERROR_ITEM_STALL_SELL_OPEN_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2501 ) );
				break;
			case ERROR_ITEM_STALL_SELL_OPEN_EXIST:		/// 이미 오픈중
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2502 ) );
				break;
			}

			if( userSell )
				userSell->NetUnLock( eLOCK_REGIST );
		}
		break;
	case NM_ITEM_STALL_SELL_OPEN_SYN:
		{
			/// 다른유저가 노점상 열고 있음
			MSG_SYN_ITEM_STALL_SELL_OPEN* msgData = (MSG_SYN_ITEM_STALL_SELL_OPEN*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( p )
			{
				p->SetStopFlag( eSTOP_OPENSTALL );
				p->ActiveUserSellBubble( msgData->title, COLOR_GREEN );
			}
		}
		break;
	case NM_ITEM_STALL_SELL_RENAME_RES:
		{
			/// 노점명 변경결과
			MSG_RES_ITEM_STALL_SELL_RENAME* msgData = (MSG_RES_ITEM_STALL_SELL_RENAME*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_RENAME_SUCCESS:
				{
					/// 노점명 변경 성공
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2515 ) );

					/// 노점면 변경
					if( userSell )
						userSell->SetName( (LPTSTR)mUserSellName.Cstr() );

					/// 상태 변경
					HERO->ActiveUserSellBubble( (LPTSTR)mUserSellName.Cstr(), COLOR_GREEN );
				}
				break;
			case ERROR_ITEM_STALL_SELL_RENAME_FAIL:
				{
					/// 변경 실패
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2516 ) );
				}
				break;
			}

			if( userSell )
				userSell->NetUnLock( eLOCK_RENAME );
		}
		break;
	case NM_ITEM_STALL_SELL_RENAME_SYN:
		{
			/// 다른 유저의 노점명 변경됨
			MSG_SYN_ITEM_STALL_SELL_RENAME* msgData = (MSG_SYN_ITEM_STALL_SELL_RENAME*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( p )
			{
				if( p->GetState() == eOBJECT_STATE_STOP && p->GetStopFlag() == eSTOP_OPENSTALL )
				{
					p->ActiveUserSellBubble( msgData->title, COLOR_GREEN );

					/// 현재 보고있는 상점의 이름이 변경되었으면 재 세팅
					if( msgData->characterIdx == mUserSellTarget )
					{
						cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
						if( userSell )
							userSell->SetName( msgData->title );
					}
				}
				else
				{
					assert(0);
				}
			}
		}
		break;
	case NM_ITEM_STALL_SELL_CLOSE_RES:
		{
			/// 노점닫기 요청
			MSG_RES_ITEM_STALL_SELL_CLOSE* msgData = (MSG_RES_ITEM_STALL_SELL_CLOSE*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_CLOSE_SUCCESS:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2503 ) );

					if( userSell )
						userSell->Hide();

					HERO->DeActiveUserSellBubble();
					HERO->StandUp();
				}
				break;
			case ERROR_ITEM_STALL_SELL_CLOSE_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2504 ) );
				break;
			}

			if( userSell )
				userSell->NetUnLock( eLOCK_END );
		}
		break;
	case NM_ITEM_STALL_SELL_CLOSE_SYN:	
		{
			/// 다른유저의 노점상 닫힘
			MSG_SYN_ITEM_STALL_SELL_CLOSE* msgData = (MSG_SYN_ITEM_STALL_SELL_CLOSE*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( p )
			{
				/// 상태 체크
				if( p->GetState() != eOBJECT_STATE_STOP )
				{
					assert(0);
				}
				if( p->GetStopFlag() != eSTOP_OPENSTALL )
				{
					assert(0);
				}

				p->SetStopFlag( eSTOP_NONE );
				p->DeActiveUserSellBubble();
				p->StandUp();

				/// hero가 이용중인경우,
				if( msgData->characterIdx == GetUserSellTarget() )
				{
					ITEMMAN->ClearUserSellTarget();
					HERO->SetStopFlag( eSTOP_NONE );

					cUserSellWindow* usersell = GAMEUI->GetUserSellWindow();
					if( usersell )
						usersell->Hide();
				}
			}
		}
		break;
	case NM_ITEM_STALL_SELL_ADD_RES:
		{
			/// 노점 아이템 등록
			MSG_RES_ITEM_STALL_SELL_ADD* msgData = (MSG_RES_ITEM_STALL_SELL_ADD*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_ADD_SUCCESS:
				if( mTry == ITEM_TRY_SELLITEMADD )
				{
					/// 아이템 등록 성공
					if( userSell && userSell->AddSellItem( mSlotIndex0, mPrice ) == false )
					{
						assert(0);
					}

					mSlotIndex0 = 0;
				}
				break;
			case ERROR_ITEM_STALL_SELL_ADD_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2506 ) );
				break;				
			case ERROR_ITEM_STALL_SELL_ADD_TRADE_MONEY:
				{
					// 노점 개당 판매 금액을 새로 등록 또는 수정하려고 할때 최대 거래 금액 보다 크게 설정하려고 할 경우

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11206 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);

					if( mSlotIndex0 <= MAX_INVENTORY )
					{
						if( userSell->IsRegistSellSlot( mSlotIndex0 ) != -1 )
						{
							cItem& item = mItemArray[ mSlotIndex0 ];
							item.SetLock( true );
							mSlotIndex0 = 0;
						}				
					}

				}				
				break;
			case ERROR_ITEM_STALL_SELL_ADD_EXCHANGE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 2019 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			}

			UnlockItemTry( ITEM_TRY_SELLITEMADD );

			if( userSell )
				userSell->NetUnLock( eLOCK_ADD );
		}
		break;
	case NM_ITEM_STALL_SELL_ADD_SYN:
		{
			/// 상대 노점상의 아이템 추가됨
			MSG_SYN_ITEM_STALL_SELL_DAT* msgData = (MSG_SYN_ITEM_STALL_SELL_DAT*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			///
			sStallSellData data;
			data.status = StallSellItemStart;
			data.inventory = msgData->inventory;
			data.price = msgData->price;

			if( userSell && userSell->AddJoinItem( &data ) == false )
			{
				assert(0);
			}
		}
		break;
	case NM_ITEM_STALL_SELL_DEL_RES:
		{
			/// 노점 아이템 삭제
			MSG_RES_ITEM_STALL_SELL_DEL* msgData = (MSG_RES_ITEM_STALL_SELL_DEL*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_DEL_SUCCESS:
				if( mTry == ITEM_TRY_SELLITEMDELETE )
				{
					/// mSlotIndex0 : 노점 슬롯을 사용한다
					if( userSell && userSell->DelSellItem( mSlotIndex0 ) == false )
					{
						assert(0);
					}
				}
				break;
			case ERROR_ITEM_STALL_SELL_DEL_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2507 ) );
				break;
			}

			UnlockItemTry( ITEM_TRY_SELLITEMDELETE );

			if( userSell )
				userSell->NetUnLock( eLOCK_DELETE );
		}
		break;
	case NM_ITEM_STALL_SELL_DEL_SYN:
		{
			MSG_SYN_ITEM_STALL_SELL_DAT* msgData = (MSG_SYN_ITEM_STALL_SELL_DAT*)msg;

			sInventory* inv = &msgData->inventory;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( userSell )
				userSell->DelJoinItem( inv );
		}
		break;
	case NM_ITEM_STALL_SELL_MOD_RES:
		{
			/// 노점아이템 정보 변경 요청
			MSG_RES_ITEM_STALL_SELL_MOD* msgData = (MSG_RES_ITEM_STALL_SELL_MOD*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_MOD_SUCCESS:
				{
					/// 변경 윈도우 띄우기
					GAMEUI->ShowMoneyModifyWindow( eMONEY_NONE, eMBEVENT_USERSELL_ITEMMONEYMODIFY );
				}
				break;
			case ERROR_ITEM_STALL_SELL_MOD_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2508 ) );
				break;

			}

			cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
			if( sell )
				sell->NetUnLock( eLOCK_ITEMUPDATE );
		}
		break;
	case NM_ITEM_STALL_SELL_MOD_SYN:
		{
			/// 노점 아이템 정보 수정중임
			MSG_SYN_ITEM_STALL_SELL_DAT* msgData = (MSG_SYN_ITEM_STALL_SELL_DAT*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( userSell )
				userSell->PauseJoinItem( &msgData->inventory );
		}
		break;
	case NM_ITEM_STALL_SELL_COM_RES:
		{
			/// 아이템 구입됨
			MSG_RES_ITEM_STALL_SELL_COM* msgData = (MSG_RES_ITEM_STALL_SELL_COM*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			sInventory* inv = &msgData->inventory;

			cItem& item = mItemArray[inv->number];
			int sellCount = item.mCount - inv->count; /// 팔린갯수
			if( item.mDefine )
			{
				/// 판매정보 출력
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 248 ), msgData->characterName, item.mDefine->GetName(), sellCount );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
			}
			else
			{
				assert(0);
			}

			/// 판매자 목록에서 삭제
			if( inv->count == 0 )
			{
				if( userSell )
					userSell->DelSellItem( inv->number, true );

				/// 인벤토리 삭제	
				NetDumpItem( inv->number );
			}
			else
			{	
				/// 자기 목록 개수 갱신
				if( userSell )
					userSell->ModSellItem( inv->number, inv->count, true );

				// 갱신후 갯수 초기화
				item.SetCount( inv->count );

				/// 아이템관련 정보 갱신
				UpdateItem();				
			}	
		}
		break;
	case NM_ITEM_STALL_SELL_JOIN_RES:
		{
			/// 상대방의 노점상 보기
			MSG_RES_ITEM_STALL_SELL_JOIN* msgData = (MSG_RES_ITEM_STALL_SELL_JOIN*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_JOIN_SUCCESS:
				{
					/// 참여 상태로 변경
					if( userSell )
					{
						userSell->SetStatus( eUSERSELL_JOIN );
						userSell->SetName( msgData->title );
						userSell->SetTargetIndex( mUserSellTarget );
					}

					/// 노점상 정보 세팅
					sStallSellData* data = msgData->stallSellData;
					for( int i = 0; i < msgData->rowCount; ++i, ++data )
					{
						if( userSell && userSell->AddJoinItem( data ) == false )
						{
							assert(0);
							continue;
						}
					}

					HERO->SetStopFlag( eSTOP_USESTALL );

					if( userSell )
					{
						cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
						if( inven && inven->IsVisible() )
							UIMAN->SetRestoreShow( inven );

						// 순서 주의
						userSell->Show();
						GAMEUI->ShowInventory();
					}
				}
				break;
			case ERROR_ITEM_STALL_SELL_JOIN_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2510 ) );
				break;
			case ERROR_ITEM_STALL_SELL_JOIN_OVER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2511 ) );
				break;
			}
		}
		break;
	case NM_ITEM_STALL_SELL_LEAVE_RES:
		{
			/// 닫기
			MSG_RES_ITEM_STALL_SELL_LEAVE* msgData = (MSG_RES_ITEM_STALL_SELL_LEAVE*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_LEAVE_SUCCESS:
				break;
			case ERROR_ITEM_STALL_SELL_LEAVE_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2504 ) );
				break;
			case ERROR_ITEM_STALL_SELL_LEAVE_CLOSED:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2512 ) );
				break;
			}

			if( userSell )
				userSell->Hide();

			/// 보고잇는 노점 정보 초기화
			mUserSellTarget = 0;

			HERO->SetStopFlag( eSTOP_NONE );

			if( userSell )
				userSell->NetUnLock( eLOCK_JOINEND );
		}
		break;
	case NM_ITEM_STALL_SELL_GET_RES:	// 구입결과
		{
			MSG_RES_ITEM_STALL_SELL_GET* msgData = (MSG_RES_ITEM_STALL_SELL_GET*)msg;

			cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
			if( !userSell )
			{
				assert(0);
			}

			/// 
			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_STALL_SELL_GET_SUCCESS:
				{
					/// 구입 아이템관련 정보 갱신
					NetSetItem( &msgData->Inventory );
					UpdateItem();

					/// 노점 구매시 사운드 재생
					SOUNDSYS->Play2DSound( 2044 );
				}
				break;
			case ERROR_ITEM_STALL_SELL_GET_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2513 ) );
				break;
			case ERROR_ITEM_STALL_SELL_LEAVE_MAX_ITEM:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 220 ) );
				break;

			case ERROR_ITEM_STALL_SELL_LEAVE_HAVE_MONEY:
				//
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11207 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;

			}

			UnlockItemTry( ITEM_TRY_SELLGET );

			if( userSell )
				userSell->NetUnLock( eLOCK_GET );
		}
		break;
	case NM_ITEM_PUT_CARD_INVENTORY_RES:	/// 아이템에 카드 장착 처리
		{
			MSG_RES_ITEM_PUT_CARD_INVENTORY* msgData = (MSG_RES_ITEM_PUT_CARD_INVENTORY*)msg;

			cPutCardWindow* putCard = GAMEUI->GetPutCardWindow();
			if( !putCard )
			{
				assert(0);
			}

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_PUT_CARD_INVENTORY_SUCCESS:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 2603 ) );

					/// 삽입한 카드 삭제
					if( mTry == ITEM_TRY_PUTCARD )
						NetDumpItem( mSlotIndex0 );

					sInventory& inv = msgData->Inventory;

					/// 해당 아이템 정보 갱신
					unsigned short slot = inv.number;
					cItem& item = mItemArray[slot];

					/// 장착 카드정보 세팅
					item.mCardIndex1 = inv.cardSlot1;
					item.mCardIndex2 = inv.cardSlot2;
					item.mCardIndex3 = inv.cardSlot3;
					item.mCardIndex4 = inv.cardSlot4;
					item.mCardIndex5 = inv.cardSlot5;

					/// 장착 정보 갱신
					if( putCard )
						putCard->AddCard( msgData->Inventory );

					/// 카드 장착 사운드 재생
					SOUNDSYS->Play2DSound( 3035 );

				}
				break;
			case ERROR_ITEM_PUT_CARD_INVENTORY_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 2604 ) );
				break;
			case ERROR_ITEM_PUT_CARD_INVENTORY_NOTYET:
				CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 2605 ) );
				break;
			case ERROR_ITEM_PUT_CARD_INVENTORY_LICENSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 293 ) );
				break;
			}

			UnlockItemTry( ITEM_TRY_PUTCARD );

			if( putCard )
				putCard->NetUnLock();
		}
		break;
	case NM_ITEM_CHANGE_INVENTORY_RES:		/// 아이템 체인지 
		{
			MSG_RES_ITEM_CHANGE_INVENTORY* msgData = (MSG_RES_ITEM_CHANGE_INVENTORY*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_CHANGE_INVENTORY_SUCCESS:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 243 ));
				NetTryItem( NM_ITEM_CHANGE_INVENTORY_RES, msgData );
				break;
			case ERROR_ITEM_CHANGE_INVENTORY_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 244 ));
				break;
			case ERROR_ITEM_CHANGE_INVENTORY_NOTYET:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 223 ));
				break;
			case ERROR_ITEM_CHANGE_INVENTORY_MAX_ITEM:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 192 ));
				break;
			case ERROR_ITEM_CHANGE_INVENTORY_HAVE_MONEY:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11217 ), 
					GAMERESOURCEMAN->GetGameText(301)
					);
				break;
			case ERROR_ITEM_CHANGE_INVENTORY_LICENSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ));
				break;
			}

			UnlockItemTry( ITEM_TRY_CHANGE );
		}
		break;
	case NM_ITEM_COLLECT_RES:
		{
			MSG_RES_ITEM_COLLECT* msgData = (MSG_RES_ITEM_COLLECT*)msg;

			cCardBookWindow* cardbook = (cCardBookWindow*)GAMEUI->GetCardBookWindow();
			if( !cardbook )
			{
				assert(0);
				return;
			}

			cTarotSheetWindow* sheet = cardbook->GetTarotSheet();
			if( !sheet )
			{
				assert(0);
				return;
			}

			// 락풀기 먼저
			UnlockItemTry( ITEM_TRY_CARDBOOKADD );

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_COLLECT_SUCCESS:
				{
					sInventory* inv = msgData->Inventory;
					for( unsigned long i = 0; i < msgData->RowCount; ++i, ++inv )
					{
						cItem& item = mItemArray[inv->number];

						// 등록된 스프레드 : 스프레드 슬롯
						if( inv->number >= INVENTORY_SPREAD_NORMAL_BEGIN && inv->number <= INVENTORY_SPREAD_NORMAL_END )
						{
							if( inv->count > 0 )
							{
								// 초기화
								if( item.mCount > 0 )
									item.SetCount( 0 );

								NetSetItem( inv );

								int i = inv->number - INVENTORY_TAROT_NORMAL_BEGIN;
								if( i >= 0 )
									sheet->UpdateData( i );

								item.SetUsedFlag( false, false );
							}
							else
								assert(0);
						}
						else
						{
							// 남은 아이템
							item.SetCount( inv->count );

							ChangeSelectSlot( inv->number );
						}
					}

					UpdateItem();

					/// 카드 놓는 사운드 재생
					SOUNDSYS->Play2DSound( 3035 );

					// 카드북 보여주기
					if( cardbook )
						cardbook->Show();
				}
				break;
			case ERROR_ITEM_COLLECT_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 342 ) );
				break;
			case ERROR_ITEM_COLLECT_NOTYET:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 342 ) );
				break;
			case ERROR_ITEM_COLLECT_LICENSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
				break;
			}
		}
		break;
	case NM_ITEM_MAKESKILL_RES:
	case NM_ITEM_RECIPE_RES:
		{
			/// 조합 전문기술 아이템 & 레시피 아이템
			MSG_RES_MAKESKILL_ITEM* msgData = (MSG_RES_MAKESKILL_ITEM*)msg;
			NetMixSkillItem( msgData );
		}
		break;
	case NM_ITEM_MIXED_RES:
		{
			/// 조합 아이템 결과
			MSG_RES_ITEM_MIXED* msgData = (MSG_RES_ITEM_MIXED*)msg;
			NetMixItem( msgData );

			/// DB 대기중이었으면 풀기
			if( mTry == ITEM_TRY_ITEMMIXDBWORK )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2823 ) );
				UnlockItemTry( ITEM_TRY_ITEMMIXDBWORK );
			}
		}
		break;
	case NM_ITEM_SKILLRESET_RES:
		{
			MSG_RES_ITEM_SKILLRESET* msgData = (MSG_RES_ITEM_SKILLRESET*)msg;
			NetSkillResetItem( msgData );
		}
		break;
	case NM_ITEM_LICENSE_RES:
		{
			// 물품인증 결과
			MSG_RES_ITEM_LICENSE*	msgData = (MSG_RES_ITEM_LICENSE*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_LICENSE_SUCCESS:
				{
					// 인벤슬롯 검사
					if( msgData->number <= MAX_INVENTORY )
					{
						cItem& item = mItemArray[msgData->number];

						// 인증 셋팅
						item.SetSeal( !(msgData->seal == 0) );

						/// 선택 슬롯 갱신
						ChangeSelectSlot( msgData->number );

						UpdateItem();
					}
					else
					{
						// 에러
						assert(0);
					}
				}
				break;
			case ERROR_ITEM_LICENSE_FAIL:			// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 277 ) );
				break;
			case ERROR_ITEM_LICENSE_TYPE:			// 미인증 아이템이 아님
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 278 ) );
				break;
			case ERROR_ITEM_LICENSE_ALREADY:		// 이미 인증된 아이템
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 279 ) );
				break;
			}

			// lock풀기
			UnlockItemTry( ITEM_TRY_SEAL );
		}
		break;
	case NM_ITEM_CASH_LIST_RES:
		{
			MSG_RES_ITEM_CASH_LIST* msgData = (MSG_RES_ITEM_CASH_LIST*)msg;

			cItemReceiptWindow* receipt = GAMEUI->GetItemReceiptWindow();

			//
			ClearCashInven();

			if( receipt )
			{
				receipt->ClearTempInven();

				// 인벤데이터 저장
				sCashInventory* inv = msgData->Inventory;
				for( long i = 0; i < msgData->RowCount; ++i, ++inv )
				{
					receipt->AddTempInven( inv );
				}

				receipt->SaveResult( msgData->ErrorCode );
				receipt->NetUnLock();
			}
			else
				assert(0);

			UnlockItemTry( ITEM_TRY_LISTRECEIPT );
		}
		break;
	case NM_ITEM_BILL_SYN:
		{
			MSG_SYN_ITEM_BILL* msgData = (MSG_SYN_ITEM_BILL*)msg;

			sItemBill* bill = msgData->itemBill;
			for( long i = 0; i < msgData->rowCount; ++i, ++bill )
			{
				NetSetItemBill( bill );
			}
		}
		break;
	case NM_ITEM_BILL_ADD_SYN:
		{
			MSG_SYN_ITEM_BILL_ADD* msgData = (MSG_SYN_ITEM_BILL_ADD*)msg;

			if( NetSetItemBill( &msgData->itemBill ) == false )
			{
				assert(0);
			}
		}
		break;
	case NM_ITEM_BILL_DEL_SYN:
		{
			// 시간제 아이템의 시간이 끝나면 온다
			MSG_SYN_ITEM_BILL_DEL* msgData = (MSG_SYN_ITEM_BILL_DEL*)msg;

			sItemBillEx* bill = (sItemBillEx*)mItemBillMap.GetAt( msgData->idx );
			if( bill )
			{
				// 아이템 검색 & 삭제
				for( unsigned short i = INVENTORY_EQUIP_BEGIN; i <= INVENTORY_BAG_END; ++i )
				{
					cItem& item = mItemArray[i];
					if( item.mDBIndex == (unsigned long)bill->inventoryIdx )
					{
						/// 삭제 메시지 출력
						if( item.GetDefine() )
						{
							cStringT msg;
							msg.Format( GAMERESOURCEMAN->GetGameText( 12300 ), item.GetDefine()->GetName() );
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
						}

						//
						cCashFreeMoveWindow* cashFreeMoveWin = GAMEUI->GetCashFreeMoveWindow();
						if( cashFreeMoveWin )
						{
							if( cashFreeMoveWin->GetLinkSlot() == i )
								cashFreeMoveWin->Hide();
						}

						cMegaphoneWindow* megaWin = GAMEUI->GetMegaphoneWindow();
						if( megaWin )
						{
							if( megaWin->GetInventoryIndex() == item.mDBIndex )
								megaWin->Hide();
						}

						// 원피스인 경우,
						if( item.mDefine && item.mDefine->GetType() == ITEM_WEAR && item.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
						{
							// 착용된 아이템인 경우 나머지 슬롯도 같이 삭제
							if( i == INVENTORY_COSTUME_BODY1 )
							{
								NetDumpItem( INVENTORY_COSTUME_BODY2 );
								NetDumpItem( INVENTORY_COSTUME_HAND );
								NetDumpItem( INVENTORY_COSTUME_FOOT );
							}
						}

						NetDumpItem( i );
						UIMAN->HideTip();
						break;
					}
				}
			}
		}
		break;
	case NM_ITEM_THEMERESET_RES:
		{
			MSG_RES_ITEM_THEMERESET* msgData = (MSG_RES_ITEM_THEMERESET*)msg;

			UIMAN->DestroyMsgBox( eMBEVENT_THEME_RESETWAIT );

			switch( msgData->ErrorCode )
			{
			case ERROR_THEMERESET_SUCCESS:
				{
					sInventory& inv = msgData->mInventory;
					cItem& item = mItemArray[inv.number];

					if( inv.count == 0 )
						item.SetCount( 0 );
					else
						NetSetItem( &inv );

					UpdateItem();
					ChangeSelectSlot( inv.number );

					//
					LPCTSTR modeName = UIMAN->GetUIText( 26500 + msgData->mThemeMode );
					LPCTSTR mapName = GAMERESOURCEMAN->GetMapName( msgData->mThemeNum );

					cStringT msg;
					msg.Format( GAMERESOURCEMAN->GetGameText( 12012 ), mapName, modeName );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, msg.Cstr() );
				}
				break;
			case ERROR_THEMERESET_STATE:			// 캐릭터 상태
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12011 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_THEMERESET_ITEM:				// 아이템 인증에러
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12013 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_THEMERESET_DBERROR:			// db처리 에러
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 8089 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_THEMERESET_ITEM_LICENSE:		// 미인증 아이템 사용
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12014 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_THEMERESET_READY:			// 던전 초기화 대기 상태
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12007 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			}

			cUIWindow* themeWin = (cUIWindow*)GAMEUI->GetThemeResetWindow();
			if( themeWin )
			{
				themeWin->Hide();
				themeWin->NetUnLock();
			}

			//
			if( HERO->GetState() != eOBJECT_STATE_DIE )
			{
				HERO->SetStopFlag( eSTOP_NONE );
			}

			UnlockItemTry( ITEM_TRY_THEMERESET );
		}
		break;
	case NM_ITEM_PARTY_GIVE_RES:
		{
			// 파티원 삽입
			MSG_RES_ITEM_PARTY_GIVE* msgData = (MSG_RES_ITEM_PARTY_GIVE*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_PARTY_GIVE_SUCCESS:		// 성공
				break;
			case ERROR_ITEM_PARTY_GIVE_FAIL:		// 실패
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 168 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_PARTY_GIVE_TYPE:		// 실패 - 파티장 분배가 아님
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 169 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_PARTY_GIVE_OWNER:		// 실패 - 지급 권한 없음
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 168 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_PARTY_GIVE_TARGET:		// 실패 - 지급할 캐릭터 없음
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 171 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_PARTY_GIVE_PARTY:		// 실패 - 소유권 오류
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 211 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;	
			case ERROR_ITEM_PARTY_GIVE_MAP:			// 실패 - 동일 맵 오류
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 167 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_PARTY_GIVE_INVEN_FULL:	 // 실패 - 인벤토리 부족
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 166 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_PARTY_GIVE_MAX_ITEM:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 172 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_PARTY_GIVE_WEIGHT:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 173 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			}

			cUIWindow* win = (cUIWindow*)GAMEUI->GetItemDivideWindow();
			if( win )
				win->NetUnLock();

			cUIWindow* looting = (cUIWindow*)GAMEUI->GetLootingWindow();
			if( looting )
				looting->NetUnLock();

			UnlockItemTry( ITEM_TRY_GIVEPARTY );
		}
		break;
	case NM_ITEM_PARTY_GIVE_SYN:
		{
			MSG_SYN_ITEM_PARTY_GIVE* msgData = (MSG_SYN_ITEM_PARTY_GIVE*)msg;
			NetGivePartyItem( msgData );
		}
		break;

	case NM_ITEM_WAREHOUSE_SUMMON_OPEN_RES:
		{
			MSG_ERROR* msgData = (MSG_ERROR*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_SUCCESS:
				{
					MSG_RES_ITEM_WAREHOUSE_SUMMON_OPEN* msgData = (MSG_RES_ITEM_WAREHOUSE_SUMMON_OPEN*)msg; 					

					if( HERO )
					{	
						/// 아이템 갱신
						sInventory* inv = &msgData->inventory;
						if( inv->count > 0 )
						{
							NetSetItem( inv );													
							UpdateItem();
						}
						else
						{
							NetDumpItem( inv->number );
						}	

						/// 캐릭터 상태 변경
						HERO->SetStopFlag( eSTOP_SUMMON_PAPER );

						GAMEUI->UpdateInvenWithWare();
						GAMEUI->ShowWarehouse();
						GAMEUI->ShowInventory();
					}
					else
					{
						assert(0);
						return;
					}
				}
				break;
			case ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_USEFUL_SUCCESS:
				{
					if( HERO )
					{
						cSummonWindow* pSummonWin = GAMEUI->GetSummonWindow();
						if( pSummonWin )
						{
							pSummonWin->StartCoolTime();
						}
						else 
							assert(0);						

						/// 캐릭터 상태 변경
						HERO->SetStopFlag( eSTOP_SUMMON_PAPER );

						GAMEUI->UpdateInvenWithWare();
						GAMEUI->ShowWarehouse();
						GAMEUI->ShowInventory();
					}
					else
					{
						assert(0);
						break;
					}	
				}
				break;
			case ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12802 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_LICENSE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 293 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_ALREADY:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 293 ),
					GAMERESOURCEMAN->GetGameText( 12085 )
					);
				break;
			}	

			UnlockItemTry( ITEM_TRY_SUMMON );

			cSummonWindow* pSummonWin = GAMEUI->GetSummonWindow();
			if( pSummonWin )
			{
				pSummonWin->NetUnLock();
				pSummonWin->Clear();
				pSummonWin->Hide();
			}
		}
		break;
	case NM_ITEM_WAREHOUSE_SUMMON_OPEN_SYN:
		{
			MSG_SYN_ITEM_WAREHOUSE_SUMMON_OPEN* msgData = (MSG_SYN_ITEM_WAREHOUSE_SUMMON_OPEN*)msg;

			cPlayer* pPlayer = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( pPlayer != NULL )
			{
				pPlayer->SetStopFlag( eSTOP_SUMMON_PAPER );
			}
			else
			{	
				assert(0);
			}
		}
		break;
	case NM_ITEM_WAREHOUSE_SUMMON_CLOSE_SYN:
		{
			MSG_SYN_ITEM_WAREHOUSE_SUMMON_CLOSE* msgData = (MSG_SYN_ITEM_WAREHOUSE_SUMMON_CLOSE*)msg;

			cPlayer* pPlayer = OBJECTMAN->GetPlayer( msgData->characterIdx );
			if( pPlayer != NULL )
			{
				pPlayer->SetStopFlag( eSTOP_NONE );
			}
			else
			{	
				assert(0);
			}
		}
		break;

	default:
		assert( 0 && "invalid network message" );
		break;
	}
}

void cItemManager::NetSetItem( sInventory* inv, bool setCool )
{
	unsigned short slotIndex = inv->number;

	if( slotIndex > MAX_INVENTORY )
	{
		assert( 0 && "index out of range" );
		return;
	}

	cItem& item = mItemArray[slotIndex];

	if( item.mCount > 0 )
	{
		if(item.mDBIndex != inv->idx || item.mIndex != inv->ItemIndex )
		{
			assert( 0 && "item already exist" );
			return;
		}
	}

	cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );

	if( itemDefine == 0 )
	{
		assert( 0 && "failed to find item define" );
		return;
	}

	item.mDBIndex = inv->idx;
	item.mIndex = inv->ItemIndex;
	item.mCount = inv->count;
	item.mDefine = itemDefine;
	item.mEnhanced = inv->enhanced;
	item.mEquipType = itemDefine->mEquipType;
	item.mFlag = itemDefine->mFlag;
	item.mSeal = (inv->seal == 1);
	item.mCash = (inv->cash == 1);
	item.mCardIndex1 = inv->cardSlot1;
	item.mCardIndex2 = inv->cardSlot2;
	item.mCardIndex3 = inv->cardSlot3;
	item.mCardIndex4 = inv->cardSlot4;
	item.mCardIndex5 = inv->cardSlot5;

	if( item.mIcon )
		item.BindIcon( item.mIcon );

	/// 아이템 세팅, 외부조건에 의한 아이템 사용, 분리, 아이템 획득, 아이템 구입
	/// 쿨타임 복사기능을 해야하면
	if( setCool == true )
		COOLMAN->CopyCoolTime( item.mIndex );

	/// 퀘스트용 아이템이면 이동불가 처리
	eItemIn in = GetItemIn( slotIndex );
	if( in == ITEM_IN_INVEN_QUEST || in == ITEM_IN_CARDBOOK_NORMAL_SPREAD )
		item.SetUsedFlag( false, false );

	/// 아이템의 제한조건 체크
	int ret0 = CheckLimit( item.GetIndex() );
	int ret1 = CheckLimitEtc( item.GetIndex() );

	bool equipError = false;
	if( slotIndex >= INVENTORY_EQUIP_BEGIN && slotIndex <= INVENTORY_EQUIP_END )
	{
		if( item.mSeal == true )
			equipError = true;	
	}

	NiColorA color = NiColorA::WHITE;
	if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS || equipError == true )
	{
		color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
	}
	item.SetColor( color );

	if( slotIndex >= INVENTORY_EQUIP_BEGIN && slotIndex <= INVENTORY_BAG_END )
	{
		/// 시간제 연결
		if( item.mDefine->GetLicenseClass() == 1 || item.mDefine->GetLicenseClass() == 2 )
		{
			cItemBillMap::cIterator b = mItemBillMap.Begin();
			cItemBillMap::cIterator end = mItemBillMap.End();
			for( ; b != end; ++b )
			{
				sItemBillEx* bill = (sItemBillEx*)(b->mSecond);
				if( bill && (unsigned long)bill->inventoryIdx == item.mDBIndex )
				{
					item.mItemBill = bill;
					break;
				}
			}
		}
	}
}

bool cItemManager::NetSetItemBill( sItemBill* bill )
{
	if( bill == 0 )
	{
		assert(0);
		return false;
	}

	// 시간정보 저장
	sItemBillEx* p = new sItemBillEx;
	p->idx = bill->idx;
	p->type = bill->type;
	p->validDate = bill->validDate;
	p->validTime = bill->validTime;
	p->inventoryIdx = bill->inventoryIdx;
	time( &p->begin );	
	p->begin += p->validTime;

	if( mItemBillMap.Insert( p->idx, p ) == false )
	{
		assert(0);
		SAFE_DELETE( p );
		return false;
	}

	// 아이템 탐색 & 연결
	for( unsigned short i = INVENTORY_EQUIP_BEGIN; i <= INVENTORY_BAG_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mDBIndex == (unsigned long)p->inventoryIdx )
		{
			// 검사
			if( !(item.mDefine->GetLicenseClass() == 1 || item.mDefine->GetLicenseClass() == 2) )
				assert(0);

			item.mItemBill = p;
			return true;
		}
	}
	return false;
}

void cItemManager::DeleteItemBill( unsigned long idx )
{
	sItemBillEx* p = (sItemBillEx*)mItemBillMap.GetAt( idx );
	if( p )
	{
		mItemBillMap.Erase( idx );
		SAFE_DELETE( p );
	}
}

void cItemManager::NetTryItem( unsigned int protocol, void* msg )
{
	switch( mTry )
	{
	case ITEM_TRY_MOVE:
		if( protocol == NM_ITEM_MOV_INVENTORY_RES ||
			protocol == NM_NPC_ITEM_MOV2_INVENTORY_RES )
			NetMoveItem( mSlotIndex0, mSlotIndex1 );
		else
			assert( 0 && "invalid protocol" );
		break;


	case ITEM_TRY_MERGE:
		if( protocol == NM_ITEM_MERGE_INVENTORY_RES )
		{
			NetMoveItem( mSlotIndex0, mSlotIndex1 );
		}
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_USE:
		if( protocol == NM_ITEM_USE_INVENTORY_RES )
			NetUseItem( mSlotIndex0 );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_EQUIP:
		if( protocol == NM_ITEM_MOV_INVENTORY_RES )
			NetMoveItem( mSlotIndex0, mSlotIndex1 );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_UNEQUIP:
		if( protocol == NM_ITEM_MOV_INVENTORY_RES )
			NetMoveItem( mSlotIndex0, mSlotIndex1 );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_DUMP:
		if( protocol == NM_ITEM_DEL_INVENTORY_RES )
			NetDumpItem( mSlotIndex0, true );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_SPLIT:
		if( protocol == NM_ITEM_DIVIDE_INVENTORY_RES ||
			protocol == NM_ITEM_MOV_INVENTORY_RES )
			NetSplitItem( (MSG_RES_ITEM_DIVIDE_INVENTORY*)msg );
		else if( protocol == NM_NPC_ITEM_MOV2_INVENTORY_RES )
			NetSplitItem( (MSG_RES_NPC_ITEM_MOV2_INVENTORY*)msg, mSlotIndex0, mSlotIndex1 );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_BUY:
		if( protocol == NM_NPC_ITEM_BUY_RES )
			NetBuyItem( (MSG_RES_NPC_ITEM_BUY*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_SELL:
		if( protocol == NM_NPC_ITEM_SELL_RES )
			NetSellItem( (MSG_RES_NPC_ITEM_SELL*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_GET:
		if( protocol == NM_ITEM_GET_RES )
			NetGetItem( (MSG_RES_ITEM_GET*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_DISJOINT:
		if( protocol == NM_ITEM_DISJOINT_INVENTORY_RES )
			NetDisjointItem( (MSG_RES_ITEM_DISJOINT_INVENTORY*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;

		/// 퀘스트 - 아이템 보상 & 반납
	case ITEM_TRY_QUEST_REWARDTAKE:
		if( protocol == NM_ITEM_REWARD_RES )
			NetRewardItem( (MSG_RES_ITEM_REWARD*)msg );
		else if( protocol == NM_ITEM_TAKE_RES )
			NetTakeItem( (MSG_RES_ITEM_TAKE*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;

	case ITEM_TRY_QUEST_ADDBYITEM:
		if( protocol == NM_ITEM_REWARD_RES )
			NetRewardItem( (MSG_RES_ITEM_REWARD*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;

		/// 퀘스트 - 아이템 반납
	case ITEM_TRY_QUEST_TAKE:
		if( protocol == NM_ITEM_TAKE_RES )
			NetTakeItem( (MSG_RES_ITEM_TAKE*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;
	case ITEM_TRY_CHANGE:
		if( protocol == NM_ITEM_CHANGE_INVENTORY_RES )
			NetChangeItem( (MSG_RES_ITEM_CHANGE_INVENTORY*)msg );
		else
			assert( 0 && "invalid protocol" );
		break;
	case ITEM_TRY_NPCDEALREGIST:
		if( protocol == NM_NPC_ITEM_AGENT_ADD_RES )
			NetItemAgentAddItem( (MSG_RES_NPC_ITEM_AGENT_ADD*) msg );
		else 
			assert( 0 && "invalid protocol" );
		break;
	case ITEM_TRY_NPCDEALDELETE:
		if( protocol == NM_NPC_ITEM_AGENT_DEL_RES )
			NetItemAgentDelItem( (MSG_RES_NPC_ITEM_AGENT_DEL*) msg );
		else 
			assert( 0 && "invalid protocol" );
		break;
	case ITEM_TRY_NPCDEALBUY:
		if( protocol == NM_NPC_ITEM_AGENT_GET_RES )
			NetItemAgentGetItem( (MSG_RES_NPC_ITEM_AGENT_GET*) msg );
		else 
			assert( 0 && "invalid protocol" );
		break;

	default:
		assert( 0 && "invalid item try" );
	}
}

/// 아이템 이동
void cItemManager::NetMoveItem( unsigned short i0, unsigned short i1 )
{
	assert( i0 <= MAX_INVENTORY && "index out of range" );
	assert( i1 <= MAX_INVENTORY && "index out of range" );
	assert( i0 != i1 );

	if( i0 == i1 )
		return;

	/// 이동 가능 여부를 검사
	eItemIn in0 = GetItemIn( i0 );
	eItemIn in1 = GetItemIn( i1 );

	assert( in0 != ITEM_IN_NULL );
	assert( in1 != ITEM_IN_NULL );

	///
	cItem& item0 = mItemArray[i0];
	if( item0.mCount == 0 )
		return;

	cItem& item1 = mItemArray[i1];
	cItemDefine* itemDefine = item0.mDefine;
	if( !itemDefine )
	{
		assert(0);
		return;
	}

	/// 장착 -> 인벤 개인
	if( in0 == ITEM_IN_EQUIP && in1 == ITEM_IN_INVEN_PRIVATE )
	{
		if( item1.mCount && (item1.mFlag & mFlagArray[i0] ) != item1.mFlag )
			return;
		if( MoveItemDown( i0, i1 ) == false )
			return;

		/// 아이템관련 정보 갱신
		CheckItemCount();

		/// 전체 능력치검사
		CheckEquipAbility();

		/// 착용 방어구셋 갱신
		HERO->ResetArmorSets();
	}
	/// 인벤 개인 -> 장착
	else if( in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_EQUIP )
	{
		if( item0.mEquipType == ITEM_EQUIP_TWO_HANDS )
		{
			if( i1 == INVENTORY_HAND_LEFT1 )
				i1 = INVENTORY_HAND_RIGHT1;
			else if( i1 == INVENTORY_HAND_LEFT2 )
				i1 = INVENTORY_HAND_RIGHT2;
		}
		if( item0.mCount && (item0.mFlag & mFlagArray[i1] ) != item0.mFlag )
			return;
		if( MoveItemUp( i0, i1 ) == false )
			return;

		/// 아이템관련 정보 갱신
		CheckItemCount();

		/// 전체 능력치검사
		CheckEquipAbility();

		/// 착용 방어구셋 갱신
		HERO->ResetArmorSets();
	}
	/// 인벤 개인 -> 인벤 개인
	else if( (in0 == ITEM_IN_INVEN_PRIVATE) && in0 == in1 )
	{
		if( item0.mIndex == item1.mIndex && item0.mCash == false && item1.mCash == false )
		{
			/// 같은 종류면 병합
			unsigned short capacity = itemDefine->mCapacity;

			/// 병합이 가능한 갯수면 병합
			if( capacity > 1 && item1.mCount < capacity )
			{
				unsigned short value = item0.mCount;
				unsigned short count = item1.mCount + value;

				if( count > capacity )
				{
					value -= (count - capacity);
					count = capacity;
				}

				assert( value > 0 );
				item0 = item0.mCount - value;
				item1 = count;
			}
		}
		else
		{
			/// 다른 종류면 교환
			cItem tempItem = item0;
			item0 = item1;
			item1 = tempItem;
		}
	}
	/// 창고 개인 -> 창고 개인
	else if( (in0 == ITEM_IN_WAREHOUSE_PRIVATE) && in0 == in1 )
	{
		if( item0.mIndex == item1.mIndex && item0.mCash == false && item1.mCash == false )
		{
			/// 같은 종류면 병합
			unsigned short capacity = itemDefine->mCapacity;

			/// 병합이 가능한 갯수면 병합
			if( capacity > 1 && item1.mCount < capacity )
			{
				unsigned short value = item0.mCount;
				unsigned short count = item1.mCount + value;

				if( count > capacity )
				{
					value -= (count - capacity);
					count = capacity;
				}

				assert( value > 0 );
				item0 = item0.mCount - value;
				item1 = count;
			}
		}
		else
		{
			/// 다른 종류면 교환
			cItem tempItem = item0;
			item0 = item1;
			item1 = tempItem;
		}
	}
	/// 인벤 개인 <-> 창고 개인
	/// 수령함 -> 인벤
	else if( (in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_WAREHOUSE_PRIVATE) ||
		(in0 == ITEM_IN_WAREHOUSE_PRIVATE && in1 == ITEM_IN_INVEN_PRIVATE) ||
		(in0 == ITEM_IN_RECEIPT && in1 == ITEM_IN_INVEN_PRIVATE) )
	{
		if( item0.mIndex == item1.mIndex && item0.mCash == false && item1.mCash == false )
		{
			/// 같은 종류면 병합
			unsigned short capacity = itemDefine->mCapacity;

			/// 병합이 가능한 갯수면 병합
			if( capacity > 1 && item1.mCount < capacity )
			{
				unsigned short value = item0.mCount;
				unsigned short count = item1.mCount + value;

				if( count > capacity )
				{
					value -= (count - capacity);
					count = capacity;
				}

				assert( value > 0 );
				item0 = item0.mCount - value;
				item1 = count;
			}
		}
		else
		{
			/// 다른 종류면 교환
			cItem tempItem = item0;
			item0 = item1;
			item1 = tempItem;
		}

		if( in0 == ITEM_IN_WAREHOUSE_PRIVATE && in1 == ITEM_IN_INVEN_PRIVATE )
		{
			COOLMAN->CopyCoolTime( item1.mIndex );
		}
		else if( in0 == ITEM_IN_RECEIPT && in1 == ITEM_IN_INVEN_PRIVATE )
		{
			COOLMAN->CopyCoolTime( item1.mIndex );
		}	
		else
			item1.EndCoolTime();

		/// 아이템관련 정보 갱신
		UpdateItem();
	}
	/// 카드북 <-> 인벤
	else if( (in0 == ITEM_IN_CARDBOOK_NORMAL && in1 == ITEM_IN_INVEN_PRIVATE) || 
		(in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_CARDBOOK_NORMAL) )
	{
		/// 같은 종류면 병합
		if( item0.mIndex == item1.mIndex )
		{
			/// 병합이 가능한 갯수면 병합
			unsigned short capacity = itemDefine->mCapacity;
			if( capacity > 1 && item1.mCount < capacity )
			{
				unsigned short value = item0.mCount;
				unsigned short count = item1.mCount + value;

				if( count > capacity )
				{
					value -= (count - capacity);
					count = capacity;
				}

				assert( value > 0 );
				item0 = item0.mCount - value;
				item1 = count;
			}
		}
		// 비어있으면 셋팅
		else if( item1.mIndex == 0 )
		{
			item1 = item0;
			item0 = 0;
		}
		else
			assert(0);

		//
		item1.EndCoolTime();

		/// 아이템관련 정보 갱신
		UpdateItem();

		// 타로카드북 갱신
		cCardBookWindow* cardBook = (cCardBookWindow*)GAMEUI->GetCardBookWindow();
		if( cardBook )
		{
			cTarotSheetWindow* sheet = cardBook->GetTarotSheet();
			if( sheet )
			{ 
				if( in0 == ITEM_IN_CARDBOOK_NORMAL )
				{
					int i = i0 - INVENTORY_TAROT_NORMAL_BEGIN;
					if( i >= 0 )
						sheet->ClearIcon( i );	
					else
						assert(0);
				}
				else if( in0 == ITEM_IN_INVEN_PRIVATE )
				{
					int i = i1 - INVENTORY_TAROT_NORMAL_BEGIN;
					if( i >= 0 )
					{
						sheet->UpdateData( i );
						cardBook->Show();
					}
					else
						assert(0);
				}
			}
			else
				assert(0);
		}
		else
			assert(0);
	}
	else
	{
		return;
	}

	/// 아이템 셀을 검사
	CheckItemSlot( i0 );
	CheckItemSlot( i1 );

	ChangeSelectSlot( i0 );
	ChangeSelectSlot( i1 );

	/// 사운드 출력
	PlayIconSettingSound( i1 );
}

/// 소모성 아이템 사용 
void cItemManager::NetUseItem( unsigned short i )
{
	cItem& item = mItemArray[i];
	cItemDefine* itemDefine = item.mDefine;
	if( !itemDefine )
	{
		assert(0);
		return;
	}

	unsigned char type = itemDefine->GetType();
	unsigned char subtype = itemDefine->GetSubType();

	/// 소모성 아이템만 사용 
	if( type == ITEM_ETC1 || type == ITEM_MONSTER || ( type == ITEM_CARD && subtype == ITEM_CARD_COMMON ) || type == ITEM_INVENTORY )
	{
		/// 쿨타임매니저에 사용 등록
		cItemAbility* itemAbil = itemDefine->GetAbililty();
		if( itemAbil && itemAbil->GetCoolTime() > 0 )
		{
			COOLMAN->StartCoolTime( item.mIndex );
		}

		/// 아이템 사용 
		item.SetCount( item.mCount - 1 );

		/// 아이템관련 정보 갱신
		UpdateItem();

		/// 선택 슬롯 갱신
		ChangeSelectSlot( i );
	}
	else
	{
		assert(0);
	}
}

/// 아이템 버리기 
void cItemManager::NetDumpItem( unsigned short i , bool onSound )
{
	if( i > MAX_INVENTORY )
	{
		assert(0);
		return;
	}

	if( mItemArray[i].mLock == true )
		mItemArray[i].SetLock( false );

	/// 아이템 삭제
	mItemArray[i] = 0;

	/// 아이템 삭제 갯수 갱신
	eItemIn in = GetItemIn(i);
	if( in == ITEM_IN_INVEN_PRIVATE )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();
	}

	// 선택슬롯 갱신
	ChangeSelectSlot( i );

	//// 휴지통에 버릴 때 나는 사운드 재생
	if( onSound )
	{
		SOUNDSYS->Play2DSound( 2058 );
	}
}

/// 아이템 분리 ( 인벤 -> 인벤)
void cItemManager::NetSplitItem( MSG_RES_ITEM_DIVIDE_INVENTORY* msgData )
{
	sInventory* inv = msgData->Inventory;
	for( unsigned long i = 0; i < msgData->RowCount; i++, inv++ )
	{
		/// 분리대신 새로생겨난 아이템 세팅.(갯수 추가안함)
		NetSetItem( inv );
	}

	/// 아이템이 있으면 그때 검사
	if( msgData->RowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();		

		/// 아이템 사운드 재생
		PlayIconSettingSound( msgData->Inventory->number );
	}
}

/// 아이템 분리  ( 인벤 <-> 창고 )
void cItemManager::NetSplitItem( MSG_RES_NPC_ITEM_MOV2_INVENTORY* msgData, unsigned short slotIndex0, unsigned short slotIndex1 )
{
	int count = 0;

	/// 분리전의 원래 갯수
	cItem& item = mItemArray[slotIndex0];
	count = item.mCount;

	eItemIn in0 = GetItemIn( slotIndex0 );
	eItemIn in1 = GetItemIn( slotIndex1 );

	/// 쿨타임 세팅여부 ( 창고 -> 인벤 일때만 한다 )
	bool setcool = false;

	/// 창고 ->인벤
	if( in0 == ITEM_IN_WAREHOUSE_PRIVATE && in1 == ITEM_IN_INVEN_PRIVATE )
	{
		setcool = true;
	}

	sInventory* inv = msgData->Inventory;
	for( unsigned long i = 0; i < msgData->RowCount; i++, inv++ )
	{
		/// 분리대신 새로생겨난 아이템 세팅.(갯수 추가안함)
		NetSetItem( inv, setcool );
	}

	/// 아이템이 있으면 그때 검사
	if( msgData->RowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();	

		/// 아이템 사운드 재생
		PlayIconSettingSound( msgData->Inventory->number );
	}
}

/// 아이템 줍기 - 인벤토리만 세팅 ( 기존에 있는 아이템과 자동 합치기 됨 )
void cItemManager::NetGetItem( MSG_RES_ITEM_GET* msgData )
{
	cStringT str;
	unsigned short itemRareLevel = eTOOLTIP_NORMAL;

	sInventory* inv = msgData->Inventory;
	for ( long i = 0; i < msgData->RowCount; i++, inv++ )
	{
		/// 줍기전의 갯수 확인하기
		cItem& item = mItemArray[inv->number];

		/// 추가된 갯수 구하기
		int addcount = inv->count - item.mCount;
		if( addcount > 0 )
		{
			/// 아이템 획득메세지 쏘기 
			cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
			if( itemDefine )
			{
				LPCTSTR countStr = cNumberEditBox::FormatNumber( addcount );

				str.Format( GAMERESOURCEMAN->GetGameText( 240 ), countStr );
				CHATMANAGER->AddItemMsg( inv->ItemIndex, inv->number, 0, (LPCTSTR)str.Cstr() );

				/// 여러개의 아이템을 동시에 습득한 경우 최고 등급 아이템 이펙트만 보여주기.
				if(	itemDefine->GetTipType() > itemRareLevel )
					itemRareLevel = itemDefine->GetTipType();
			}
		}
		else
			assert(0);

		// 인벤토리 셋팅
		NetSetItem( inv );
	}

	/// 갯수 갱신
	if( msgData->RowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();
	}

	/// 아이템 획득 이펙트
	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );

	switch( itemRareLevel )
	{
		//case eTOOLTIP_MALL:
	case eTOOLTIP_ETC:
	case eTOOLTIP_NORMAL:
	case eTOOLTIP_MAGIC:
		{
			HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_itemget_01.nif", &trans, false, true );
			SOUNDSYS->Play2DSound( 2015 );
		}
		break;

	case eTOOLTIP_RARE:
	case eTOOLTIP_UNIQUE:
	case eTOOLTIP_EPIC:	
	case eTOOLTIP_REGEND:
		{
			HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_itemget_02.nif", &trans, false, true );
			SOUNDSYS->Play2DSound( 2016 );
		}
		break;
	}
}

//ItemActiveFront = left2, right2
//ItemActiveBack = left1, right1
void cItemManager::NetSwitchWeapon( eItemActiveWeapon activeWeapon )
{
	SetSwitchWeapon( activeWeapon, true );
}

/// 아이템 사기 (새로운 칸에 삽입)
void cItemManager::NetBuyItem( MSG_RES_NPC_ITEM_BUY* msgData )
{
	cStringT str;
	sInventory* inv = msgData->inventory;

	/// 아이템 Idx, 변화 갯수
	sChangeCount buyItem;
	sChangeCount payItem;

	for ( long i = 0; i < msgData->rowCount; i++, inv++ )
	{
		/// 
		cItem& item = mItemArray[inv->number];

		/// 변화량 체크
		int changeVal = (int)inv->count - item.mCount;
		if( changeVal < 0 )
		{
			/// 지불 아이템
			if( payItem.itemIdx != 0 && payItem.itemIdx != inv->ItemIndex )
				assert(0);

			payItem.itemIdx = inv->ItemIndex;
			payItem.total += changeVal;
		}
		else if( changeVal > 0 )
		{
			/// 구입 아이템
			if( buyItem.itemIdx != 0 && buyItem.itemIdx != inv->ItemIndex )
				assert(0);

			buyItem.itemIdx = inv->ItemIndex;
			buyItem.total += changeVal;
		}

		/// 인벤토리 변경
		if( inv->count == 0 )
		{
			item.SetCount( 0 );
		}
		else
		{
			NetSetItem( inv );
			ChangeSelectSlot( inv->number );
		}
	}

	if( buyItem.total > 0 )
	{
		/// 상점 구입 사운드 재생
		cItemDefine* itemDefine = GetItemDefine( buyItem.itemIdx );
		if( itemDefine )
		{
			LPCTSTR countStr = cNumberEditBox::FormatNumber( buyItem.total );

			str.Format( GAMERESOURCEMAN->GetGameText( 209 ), itemDefine->GetName(), countStr );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NPCSTORE, (LPCTSTR)str.Cstr() );
		}

	}

	if( payItem.total < 0 )
	{
		payItem.total *= -1;

		cItemDefine* itemDefine = GetItemDefine( payItem.itemIdx );
		if( itemDefine )
		{
			LPCTSTR countStr = cNumberEditBox::FormatNumber( payItem.total );

			str.Format( GAMERESOURCEMAN->GetGameText( 217 ), itemDefine->GetName(), countStr );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NPCSTORE, (LPCTSTR)str.Cstr() );
		}
	}


	/// 아이템관련 정보 갱신
	if( msgData->rowCount > 0 )
	{
		UpdateItem();
	}
}

/// 아이템 팔기
void cItemManager::NetSellItem( MSG_RES_NPC_ITEM_SELL* msgData )
{
	cStringT str;
	sInventory* inv = msgData->inventory;

	sChangeCount sellItem;

	for ( long i = 0; i < msgData->rowCount; i++, inv++ )
	{
		/// 팔기전의 갯수 확인하기
		cItem& item = mItemArray[inv->number];

		/// 변화량 체크
		int changeVal = (int)inv->count - item.mCount;
		if( changeVal < 0 )
		{
			/// 지불 아이템
			if( sellItem.itemIdx != 0 && sellItem.itemIdx != inv->ItemIndex )
				assert(0);

			sellItem.itemIdx = inv->ItemIndex;
			sellItem.total += changeVal;
		}

		/// 인벤토리 정보 변경
		if( inv->count == 0 )
			item.SetCount( 0 );
		else
			NetSetItem( inv );
	}

	if( sellItem.total < 0 )
	{
		sellItem.total *= -1;

		cItemDefine* itemDefine = GetItemDefine( sellItem.itemIdx );
		if( itemDefine )
		{
			LPCTSTR countStr = cNumberEditBox::FormatNumber( sellItem.total );

			str.Format( GAMERESOURCEMAN->GetGameText( 218 ), itemDefine->GetName(), countStr );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NPCSTORE, (LPCTSTR)str.Cstr() );
		}
	}

	/// 아이템관련 정보 갱신
	UpdateItem();
	ChangeSelectSlot( mSlotIndex0 );

	/// 상점에서 아이템 판매 시 사운드 재생
	SOUNDSYS->Play2DSound( 2045 );

}

/// 아이템 분해
void cItemManager::NetDisjointItem( MSG_RES_ITEM_DISJOINT_INVENTORY* msgData )
{
	/// 분해된 아이템 삭제
	cNpcDisjointWindow* p = GAMEUI->GetNpcDisjointWindow();
	if( p )
		NetDumpItem( p->GetSlotIndex() );
	else
		assert(0);

	///
	cStringT str;
	sInventory* inv = msgData->Inventory;
	for ( unsigned long i = 0; i < msgData->RowCount; i++, inv++ )
	{
		/// 새로운 갯수 - 기존갯수 = 실제로 추가된 개수
		cItem& item = mItemArray[inv->number];

		/// 추가된 갯수 구하기
		int addcount = inv->count - item.mCount;

		/// 아이템 세팅
		NetSetItem( inv );

		/// 아이템 획득메세지 쏘기 
		cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
		if( itemDefine )
		{
			LPCTSTR countStr = cNumberEditBox::FormatNumber( addcount );

			str.Format( GAMERESOURCEMAN->GetGameText( 253 ), itemDefine->GetName(), countStr );
			CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, (LPCTSTR)str.Cstr() );
		}
	}

	if( msgData->RowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();

		/// 아이템 분해 완료 사운드 재생(아이템 획득 사운드는 없음)
		SOUNDSYS->Play2DSound( 2019 );
	}
}

void cItemManager::NetEnhancedItem( MSG_SYN_ITEM_ENHANCED_END* msgData )
{
	cEnhancedWindow* pEnhancedWin = GAMEUI->GetEnhancedWindow();

	cStringT str;
	sInventory* inv0		= &msgData->Inventory[0];		/// 강화 아이템
	sInventory* inv1		= &msgData->Inventory[1];		/// 강화 카드
	sInventory* inv2		= NULL;

	if( msgData->RowCount > 2 )	
	{	
		inv2 = &msgData->Inventory[2];		/// 캐시 아이템 강화 카드
	}

	bool success = false;

	/// 갱신된 카드갯수 세팅
	cItem* card = GetItem( inv1->number );
	if( card )
		card->SetCount( inv1->count );

	if( inv2 )
	{
		cItem* cashProtectcard = GetItem( inv2->number );
		if( cashProtectcard )
			cashProtectcard->SetCount( inv2->count );
	}

	/// 강화아이템 검사
	cItem& item = mItemArray[inv0->number];
	if( inv0->count > 0 )
	{
		if( item.mEnhanced < inv0->enhanced )
		{
			/// 강화성공
			success = true;
			CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 256 ) );

			// 강화 최종단계시(10강) 강화창 버튼 비활성화
			if( inv0->enhanced == MAX_ITEM_ENHANCED )
			{
				pEnhancedWin->SetStartBtnEnable( false );
			}

			// 강화 7단계 성공 부터 트위터 이벤트 메시지 전송
			if( OPTIONMAN->IsAutoSendTwitterMsg()	== true						&&
				inv0->enhanced						>= TWIT_EVENT_ENHANCE_STEP	 )
			{
				cTwitterWindow* pTwitWin = GAMEUI->GetTwitterWindow();
				if( pTwitWin )
				{
					if( pTwitWin->IsLogin() == true )
					{
						sTwitterEvent tEvent;
						tEvent.mEvent	= eTWITEVENT_ENHANCE;
						tEvent.mExData	= (void*)inv0->number;

						pTwitWin->AddEvent( tEvent );
					}				
				}
				else
				{
					assert(0);
				}
			}
		}
		else if( item.mEnhanced == inv0->enhanced )
		{

			/// 강화실패 - 변동없음
			if( inv2 )
			{	
				cItemDefine* pItemDef = ITEMMAN->GetItemDefine( inv2->ItemIndex );
				if( pItemDef == NULL )
				{
					assert(0);
				}
				if( pItemDef->GetType() == ITEM_MATERIAL )
				{
					if( pItemDef->GetSubType() == ITEM_MATERIAL_ENHANCED_DOWN_30 ||
						pItemDef->GetSubType() == ITEM_MATERIAL_ENHANCED_DOWN_60 )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 12102 ) );
					}
					else
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 12104 ) );
					}

				}
				
			}
			else 
				CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 257 ) );
		}
		else
		{
			/// 강화실패 - 단계 하강
			CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 257 ) );
			CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 258 ) );
							
		}

		/// 세팅
		item.mCardIndex1 = inv0->cardSlot1;
		item.mCardIndex2 = inv0->cardSlot2;
		item.mCardIndex3 = inv0->cardSlot3;
		item.mCardIndex4 = inv0->cardSlot4;
		item.mCardIndex5 = inv0->cardSlot5;
		item.mEnhanced = inv0->enhanced;

		if( pEnhancedWin )
		{
			pEnhancedWin->AddItem( inv0->number, false, true );
		}

	}
	else
	{
		/// 강화실패 - 아이템 파괴
		NetDumpItem( inv0->number );

		if( pEnhancedWin )
		{
			pEnhancedWin->ClearItemSlot();
			pEnhancedWin->ClearCardSlot();
			pEnhancedWin->ClearCahProtectCardSlot();
		}

		CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 257 ) );
		CHATMANAGER->AddSystemMsg( eSYSTEM_DOITEM, GAMERESOURCEMAN->GetGameText( 259 ) );
	}

	if( pEnhancedWin )
		pEnhancedWin->SetResult( success );

	NiTransform trans;
	trans.MakeIdentity();

	if( success == true )
	{
		trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );
		HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_enchant_02.nif", &trans, false, true );
		HERO->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) 817 );
		SOUNDSYS->Play2DSound( 2023 );
	}
	else
	{
		trans.m_Translate = NiPoint3( 0.0f, 0.0f, 20.0f );
		HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_enchant_03.nif", &trans, false, true );
		HERO->ChangeAnimation( (ePLAYER_ANIMATIONTYPE) 818 );
		SOUNDSYS->Play2DSound( 2024 );
	}

	/// 아이템 관련 정보 갱신
	UpdateItem();
	ChangeSelectSlot(0);
}


/// 퀘스트 - 아이템 보상, 신규퀘스트 아이템 제공
void cItemManager::NetRewardItem( MSG_RES_ITEM_REWARD* msgData )
{
	cStringT str;

	// 퀘스트를 발동시키는 아이템도 오므로, 아이템이 소모가 된다
	sInventory* inv = msgData->inventory;
	for ( long i = 0; i < msgData->rowCount; i++, inv++ )
	{
		cItem& item = mItemArray[inv->number];
		
		/// 변화량 체크
		int changeVal = (int)inv->count - item.mCount;
		if( changeVal > 0 )
		{
			/// 획득
			cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
			if( itemDefine )
			{
				LPCTSTR countStr = cNumberEditBox::FormatNumber( changeVal );
				str.Format( GAMERESOURCEMAN->GetGameText( 240 ), countStr );
				CHATMANAGER->AddItemMsg( inv->ItemIndex, inv->number, GAMERESOURCEMAN->GetGameText( 236 ), (LPCTSTR)str.Cstr(), true );
			}
		}

		/// 인벤토리 정보 변경
		if( inv->count == 0 )
			item.SetCount( 0 );
		else
			NetSetItem( inv );

		ChangeSelectSlot( inv->number );
	}

	/// 갯수 갱신
	if( msgData->rowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();		
	}
}

/// 퀘스트 - 아이템 반납
void cItemManager::NetTakeItem( MSG_RES_ITEM_TAKE* msgData )
{
	sInventory* inv = msgData->inventory;
	for( long i = 0; i < msgData->rowCount; ++i, ++inv )
	{
		cItem& item = mItemArray[inv->number];
		item.SetCount( inv->count );

		// 선택슬롯 갱신
		if( inv->count == 0 )
			ChangeSelectSlot( inv->number );
	}
	
	/// 갯수 갱신
	if( msgData->rowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();
	}
}

/// 
void cItemManager::NetTradeItem( MSG_RES_ITEM_EXCHANGE_END* msgData )
{
	/// 거래된 아이템 삭제
	cTradeWindow* trade = GAMEUI->GetTradeWindow();
	if( !trade )
	{
		assert(0);
		return;
	}

	/// 삭제
	unsigned short lastSlot = trade->GetMyLastSlot();
	for( unsigned short i = 0; i < lastSlot; ++i )
	{
		unsigned short slotIndex = trade->GetMySlotIndex( i );
		if( slotIndex <= MAX_INVENTORY )
			NetDumpItem( slotIndex );
	}

	cStringT str;

	sInventory* inv = msgData->Inventory;
	for ( unsigned long i = 0; i < msgData->RowCount; i++, inv++ )
	{
		/// 갯수 및 무게 추가 안함
		NetSetItem( inv );

		/// 아이템 획득메세지 쏘기 
		cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
		if( itemDefine )
		{
			LPCTSTR countStr = cNumberEditBox::FormatNumber( inv->count );

			str.Format( GAMERESOURCEMAN->GetGameText( 240 ), countStr );
			CHATMANAGER->AddItemMsg( inv->ItemIndex, inv->number, 0, (LPCTSTR)str.Cstr() );
		}
	}

	/// 갯수 갱신
	if( msgData->RowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();		
	}
}

void cItemManager::NetChangeItem( MSG_RES_ITEM_CHANGE_INVENTORY* msgData )
{
	cStringT str;
	sInventory* inv = msgData->Inventory;
	for ( unsigned long i = 0; i < msgData->RowCount; i++, inv++ )
	{
		/// 새로운 갯수 - 기존갯수 = 실제로 추가된 개수
		cItem& item = mItemArray[inv->number];

		/// 추가된 갯수 구하기
		int addcount = inv->count - item.mCount;
			
		/// 아이템 세팅
		if( inv->count <= 0 )
		{
			item.SetCount( 0 );
		}
		else
		{
			NetSetItem( inv );
		}

		if( addcount > 0 )
		{
			// 획득한 아이템
			cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
			if( itemDefine )
			{
				LPCTSTR countStr = cNumberEditBox::FormatNumber( addcount );

				str.Format( GAMERESOURCEMAN->GetGameText( 240 ), countStr );
				CHATMANAGER->AddItemMsg( inv->ItemIndex, inv->number, 0, (LPCTSTR)str.Cstr() );
			}
		}

		ChangeSelectSlot( inv->number );
	}

	if( msgData->RowCount > 0 )
	{
		/// 아이템관련 정보 갱신
		UpdateItem();
	}
}

void cItemManager::NetMixSkillItem( MSG_RES_MAKESKILL_ITEM* msgData )
{
	sInventory* inv = &msgData->inventory;
	cItem& item = mItemArray[inv->number];

	if( inv->count == 0 )
	{
		/// 삭제
		item.SetCount( 0 );
	}
	else
	{
		NetSetItem( inv );
	}

	/// 갯수 갱신
	UpdateItem();

	/// 선택 슬롯 갱신
	ChangeSelectSlot( inv->number );
}

void cItemManager::NetMixItem( MSG_RES_ITEM_MIXED* msgData )
{
	cStringT str;
	sInventory* inv = msgData->inventory;
	for ( long i = 0; i < msgData->rowCount; i++, inv++ )
	{
		/// 새로운 갯수 - 기존갯수 = 실제로 추가된 개수
		cItem& item = mItemArray[inv->number];

		/// 변화량 체크
		int changeVal = (int)inv->count - item.mCount;
		if( changeVal > 0 )
		{
			/// 아이템 획득메세지 쏘기 
			cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
			if( itemDefine )
			{
				LPCTSTR countStr = cNumberEditBox::FormatNumber( changeVal );

				str.Format( GAMERESOURCEMAN->GetGameText( 240 ), countStr );
				CHATMANAGER->AddItemMsg( inv->ItemIndex, inv->number, 0, (LPCTSTR)str.Cstr() );
			}
		}

		/// 인벤토리 정보 변경
		if( inv->count == 0 )
			item.SetCount( 0 );
		else
			NetSetItem( inv );

		/// 선택 슬롯 갱신
		ChangeSelectSlot( inv->number );
	}

	/// 갯수 갱신
	UpdateItem();
}

void cItemManager::NetGivePartyItem( MSG_SYN_ITEM_PARTY_GIVE* msgData )
{
	cStringT str;
	sInventory* inv = msgData->inventory;
	for ( long i = 0; i < msgData->rowCount; i++, inv++ )
	{
		/// 새로운 갯수 - 기존갯수 = 실제로 추가된 개수
		cItem& item = mItemArray[inv->number];

		/// 변화량 체크
		int changeVal = (int)inv->count - item.mCount;
		if( changeVal > 0 )
		{
			/// 아이템 획득메세지 쏘기 
			cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
			if( itemDefine )
			{
				LPCTSTR countStr = cNumberEditBox::FormatNumber( changeVal );

				str.Format( GAMERESOURCEMAN->GetGameText( 240 ), countStr );
				CHATMANAGER->AddItemMsg( inv->ItemIndex, inv->number, 0, (LPCTSTR)str.Cstr() );


				//str.Format( GAMERESOURCEMAN->GetGameText( 139 ), mParty[i].name );
				//CHATMANAGER->AddItemMsg( pmsg->ItemIndex[idx], USHRT_MAX, (LPCTSTR)str.Cstr(), GAMERESOURCEMAN->GetGameText( 140 ) );
			}
		}

		/// 인벤토리 정보 변경
		if( inv->count == 0 )
			item.SetCount( 0 );
		else
			NetSetItem( inv );

		/// 선택 슬롯 갱신
		ChangeSelectSlot( inv->number );
	}

	/// 갯수 갱신
	UpdateItem();
}

void cItemManager::NetSkillResetItem( MSG_RES_ITEM_SKILLRESET* msgData )
{
    sInventory* inv = &msgData->inventory;
	cItem& item = mItemArray[inv->number];
	
	if( inv->count == 0 )
	{
		item.SetCount( 0 );
	}
	else
	{
		NetSetItem( inv );
	}

	/// 갯수 갱신
	UpdateItem();

	/// 선택 슬롯 갱신
	ChangeSelectSlot( inv->number );
}

void cItemManager::NetItemAgentAddItem( MSG_RES_NPC_ITEM_AGENT_ADD* msgData )
{
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

	// 관리정보 초기화
	regist->Clear();
	regist->DeleteRegistItem();

	NPC_ITEM_AGENT_OPEN_RESULT* data = msgData->Result;
	for( long i = 0; i < msgData->RowCount; ++i, ++data )
	{
		NPC_ITEM_AGENT_OPEN_RESULT* p = new NPC_ITEM_AGENT_OPEN_RESULT;
		*p = *data;

		if( regist->AddItem( p ) == false )
		{
			assert(0);
			SAFE_DELETE(p);
			continue;
		}
	}

	regist->SetTotalPage( msgData->EndPage );
	regist->ChangePage( msgData->StartPage );

	win->UpdateHaveMoney();

	/// 아이템 삭제
	NetDumpItem( mSlotIndex0 );
}

void cItemManager::NetItemAgentDelItem( MSG_RES_NPC_ITEM_AGENT_DEL* msgData )
{
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

	/// 취소된 아이템이므로 인벤토리에 재 삽입
	NetSetItem( &msgData->Inventory );

	// 아이템 무게 및 갯수, 퀵슬롯 갱신
	UpdateItem();
	
	// 관리정보 초기화
	regist->Clear();

	NPC_ITEM_AGENT_OPEN_RESULT* data = msgData->Result;
	for( long i = 0; i < msgData->RowCount; ++i, ++data )
	{
		NPC_ITEM_AGENT_OPEN_RESULT* p = new NPC_ITEM_AGENT_OPEN_RESULT;
		*p = *data;

		if( regist->AddItem( p ) == false )
		{
			assert(0);
			SAFE_DELETE(p);
			continue;
		}
	}

	regist->SetTotalPage( msgData->EndPage );
	regist->ChangePage( msgData->StartPage );

}

void cItemManager::NetItemAgentGetItem( MSG_RES_NPC_ITEM_AGENT_GET* msgData )
{
	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return;
	}

	cNpcDealSearchSheet* searchSheet = win->GetSearchSheet();
	if( searchSheet )
	{
		searchSheet->ClearData();

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

		searchSheet->SavePage( msgData->StartPage, msgData->EndPage );
		searchSheet->UpdateItemList();
		win->UpdateHaveMoney();
	}

	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
		eMBEVENT_NONE, eSKIN_OK,
		GAMERESOURCEMAN->GetGameText( 1518 ),
		GAMERESOURCEMAN->GetGameText( 301 )
		);

}

// 수령함 아이템 초기화
void cItemManager::ClearCashInven()
{
	for( unsigned short i = INVENTORY_CASH_ITEM_BEGIN; i <= INVENTORY_CASH_ITEM_END; ++i )
	{
		mItemArray[i] = 0;
	}
}

// 캐쉬템 셋팅 - 수령함 아이템
void cItemManager::NetSetCashItem( sCashInventory* inv )
{
	unsigned short slotIndex = inv->number;

	// 인벤토리 넘버 체크
	if( !(slotIndex >= INVENTORY_CASH_ITEM_BEGIN && slotIndex <= INVENTORY_CASH_ITEM_END) )
	{
		assert(0);
		return;
	}

	// 아이템 셋팅
	cItem& item = mItemArray[slotIndex];
	if( item.mCount > 0 )
	{
		if(item.mDBIndex != inv->idx || item.mIndex != inv->ItemIndex )
		{
			assert( 0 && "item already exist" );
			return;
		}
	}

	cItemDefine* itemDefine = GetItemDefine( inv->ItemIndex );
	if( itemDefine == 0 )
	{
		assert( 0 && "failed to find item define" );
		return;
	}

	item.mDBIndex = inv->idx;
	item.mIndex = inv->ItemIndex;
	item.mCount = inv->count;
	item.mDefine = itemDefine;
	item.mSeal = (inv->seal == 1);
	item.mCash = (inv->cash == 1);
	item.mEnhanced = 0;
	item.mEquipType  = itemDefine->mEquipType;
	item.mFlag		 = itemDefine->mFlag;
	item.mCardIndex1 = 0;
	item.mCardIndex2 = 0;
	item.mCardIndex3 = 0;
	item.mCardIndex4 = 0;
	item.mCardIndex5 = 0;

	if( item.mIcon )
		item.BindIcon( item.mIcon );

	/// 아이템의 제한조건 체크
	int ret0 = CheckLimit( item.GetIndex() );
	int ret1 = CheckLimitEtc( item.GetIndex() );

	NiColorA color = NiColorA::WHITE;
	if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
	{
		color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
	}
	item.SetColor( color );

	//if( item.mCash == false )
		//assert(0);
}

bool cItemManager::CheatMakeItem( sInventory* inv )
{
	if( IsLocked() )
		return false;

	int total = 0;
	unsigned long itemIndex = inv->ItemIndex;
	{
		/// 줍기전의 갯수 확인하기
		cItem& item = mItemArray[inv->number];

		/// 새로운 갯수 - 기존갯수 = 실제로 추가된 개수
		total += ( inv->count - item.mCount );

		/// 아이템 세팅
		NetSetItem( inv );
	}

	/// 아이템관련 정보 갱신
	UpdateItem();
	
	/// 아이템 획득메세지 쏘기
	cItemDefine* itemDefine = GetItemDefine( itemIndex );
	if( itemDefine )
	{
		LPCTSTR countStr = cNumberEditBox::FormatNumber( total );

		cStringT str;
		str.Format( GAMERESOURCEMAN->GetGameText( 215 ), itemDefine->GetName(), countStr );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
	}

	return true;
}

void cItemManager::LockChangeToMerge()
{
	/// move 잠금 풀고 merge 잠금 걸기
	if( mTry == ITEM_TRY_MOVE )
	{
		mLocked = false;
		mTry = ITEM_TRY_NULL;
	}
	else
	{
		assert(0);
	}

	///
	mLocked = true;
	mTry = ITEM_TRY_MERGE;
}
