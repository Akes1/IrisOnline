#include "stdafx.h"
#include "TarotManager.h"
#include "ItemManager.h"
#include "GameUIManager.h"
#include "CardImage.h"
#include "TarotWindow.h"
#include "TarotControlWindow.h"
#include "GameResourceManager.h"
#include "ChatManager.h"
#include "Npc.h"
#include "UINpcTelling.h"
#include "Player_Common.h"

#include "UIManager.h"

#include "ObjectManager.h"
#include "Hero.h"

#define TAROT_PARAMS_FILENAME		"./Script/Interface/Path_Tarot.txt"
#define NPCTAROT_SPREAD_FILE		"./Script/Resource/FortuneTarot_Spread.txt"
#define NPCTAROT_COMMENT_FILE		"./Script/Resource/FortuneTarot_Comment.txt"
#define NPCTAROT_CARD_FILE			"./Script/Resource/FortuneTarot_Card.txt"

cTarotManager*	cTarotManager::mSingleton = 0;

cTarotManager::cTarotManager()
{	
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mCurrentPage = ePAGE_NONE;
	mTarotType = eTAROT_NONE;
}

cTarotManager::~cTarotManager()
{
	mSingleton = 0;

	/// 타롯 이미지 파람
	{
		cTarotParamMap::cIterator i = mTarotParamMap.Begin();
		cTarotParamMap::cIterator end = mTarotParamMap.End();

		for( ; i != end; ++i )
		{
			delete (cCardParam*)(i->mSecond);
		}
		mTarotParamMap.Clear();
	}
	/// 스프레드
	{
		cNpcSpreadMap::cIterator i = mNpcSpreadMap.Begin();
		cNpcSpreadMap::cIterator end = mNpcSpreadMap.End();
		for( ; i != end; ++i )
		{
			sNpcSpread* p = (sNpcSpread*)(i->mSecond);
			SAFE_DELETE( p );
		}
		mNpcSpreadMap.Clear();
	}

	/// npc 카드
	{
		cNpcTarotMap::cIterator i = mNpcTarotMap.Begin();
		cNpcTarotMap::cIterator end = mNpcTarotMap.End();
		for( ; i != end; ++i )
		{
			sNpcTarotCard* p = (sNpcTarotCard*)(i->mSecond);
			SAFE_DELETE( p );
		}
		mNpcTarotMap.Clear();
	}

	/// 해석 
	{
		cNpcCommentMap::cIterator i = mNpcCommentMap.Begin();
		cNpcCommentMap::cIterator end = mNpcCommentMap.End();
		for( ; i != end; ++i )
		{
			sNpcTarotComment* p = (sNpcTarotComment*)(i->mSecond);
			SAFE_DELETE( p );
		}
		mNpcCommentMap.Clear();
	}

	/// 선택 타롯 인덱스
	ClearCardinfo();
}

///
void cTarotManager::Clear()
{
	/// 새로운 점이 시작되기전에 데이터 클리어 
	/// 선택 타롯 인덱스
	ClearCardinfo();
}

///
bool cTarotManager::Init()
{
	/// 타롯 이미지 스크립트 로딩
	if( LoadTarotParams( TAROT_PARAMS_FILENAME) == false )
	{
		assert( 0 && "failed to load tarot params file" );
		return false;
	}
	/// 스프레드 정보 로드
	if( LoadNpcSpread( NPCTAROT_SPREAD_FILE ) == false )
	{
		assert( 0 && "failed to load npc spread" );
		return false;
	}

	/// 해석 정보 로드
	if( LoadComment( NPCTAROT_COMMENT_FILE ) == false )
	{
		assert( 0 && "failed to load tarot comment" );
		return false;
	}

	if( LoadTarotDeck( NPCTAROT_CARD_FILE ) == false )
	{
		assert( 0 && "failed to load tarot" );
		return false;
	}
	return true;
}

void cTarotManager::TarotReadyLeader()
{
	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE || state == ePLAYER_STATE_SITDOWN ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return;
	}

	/// 이미 타롯관련창을 보고있으면 리턴
	if( IsUsingTarot() )
		return;

	unsigned int error = TAROTMAN->CheckOpenTarot();
	if( error != ERROR_NONE )
	{
		TAROTMAN->ShowErrorMsg( error );
		return;
	}

	if( HERO->IsRideVehicle() == true )
	{
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		HERO->AlightVehicle();
	}

	/// 따라가기 종료
	if( HERO->IsFollowing() )
		HERO->ReleaseFollowPlayer();

	/// tarot 준비상태 알림
	MSG_REQ_TAROT_READER_READY msg;
	msg.Category = NM_TAROT;
	msg.Protocol = NM_TAROT_READER_READY_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

bool cTarotManager::TarotSelfOpen()
{
	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE || state == ePLAYER_STATE_SITDOWN ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	GAMEUI->HideCardBookWindow();

	/// 이미 타롯관련창을 보고있으면 리턴
	if( IsUsingTarot() )
		return false;

	unsigned int error = TAROTMAN->CheckOpenTarot();
	if( error != ERROR_NONE )
	{
		TAROTMAN->ShowErrorMsg( error );
		return false;
	}

	/// 따라가기 종료
	if( HERO->IsFollowing() )
		HERO->ReleaseFollowPlayer();

	/// selft tarot open 
	MSGROOT msg;
	msg.Category = NM_TAROT;
	msg.Protocol = NM_TAROT_SELF_OPEN_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

bool cTarotManager::TarotTutorialOpen( unsigned long npcIndex )
{
	/// 이미 타롯관련창을 보고있으면 리턴
	if( IsUsingTarot() )
		return false;

	MSG_REQ_TAROT_TUTORIAL_SEEKER_OPEN msg;
	msg.Category = NM_TAROT;
	msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_OPEN_REQ;
	msg.npcIndex = npcIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

unsigned int cTarotManager::CheckOpenTarot()
{
	/// 재료 체크
	sItemCount* p = ITEMMAN->GetItemCount( TAROT_MATERIAL_ITEM );
	unsigned int count = ( p ) ? p->bag : 0;
	if( count == 0 )
		return ERROR_STUFF;

	/// 스프레드 체크
	unsigned int spreadCount = 0;
	for( unsigned short i = 0; i<MAX_SPREAD; i++ )
	{
		cItem* item = ITEMMAN->GetItem( (unsigned short)(INVENTORY_SPREAD_NORMAL_BEGIN + i) );
		if( item )
		{
			unsigned int spreadIdx = item->GetIndex();
			if( spreadIdx == 0 )
				continue;	

			cItemDefine* spreadDefine = ITEMMAN->GetItemDefine( spreadIdx );
			if( !spreadDefine )
			{
				assert(0);
				continue;
			}

			unsigned char type = spreadDefine->GetType();
			if( type != ITEM_SPREAD )
				continue;

			spreadCount++;
		}
	}
	if( spreadCount == 0 )
		return ERROR_SPREAD;

	/// 카드 체크
	for( unsigned short i=0; i<MAX_TAROT_CARDS; i++ )
	{
		/// 타롯슬롯에 있는 아이템 인덱스를 획득
		cItem* item = ITEMMAN->GetItem( (unsigned short)(INVENTORY_TAROT_NORMAL_BEGIN + i) );
		if( item && item->GetDefine() )
		{
			unsigned int itemIdx = item->GetIndex();
			if( itemIdx == 0 )
				return ERROR_CARD;

			unsigned char subType = item->GetDefine()->GetSubType();
			if( subType != ITEM_CARD_TAROT )
				return ERROR_CARD;

			cItemTarot* pTarot = item->GetDefine()->GetTarotDefine();
			if( pTarot == 0 )
				return ERROR_CARD;

			if( pTarot->GetUniqueNumber() != i )
				return ERROR_CARD;
		}
		else
			return ERROR_CARD;
	}

	return ERROR_NONE;
}

///
void cTarotManager::ClearCardinfo()
{
	for( unsigned int i = 0; i < mSelectInfoArr.GetSize(); ++i )
	{
		sResultInfo* info = (sResultInfo*)mSelectInfoArr[i];
		SAFE_DELETE( info );
	}
	mSelectInfoArr.Clear();
}

///
void cTarotManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
	case NM_TAROT_READER_READY_RES:
		{
			/// 타로카드 준비 상태 요청 결과
			MSG_RES_TAROT_READER_READY* pMsg = (MSG_RES_TAROT_READER_READY*)msg;

			switch( pMsg->ErrorCode )
			{
			case ERROR_TAROT_READER_READY_SUCCESS:
				{
					if( mCurrentPage == ePAGE_NONE )
					{
						HERO->SetStopFlag( eSTOP_READYTAROT );
						ChangePage( ePAGE_OPEN );
					}
					else
					{
						assert(0);
					}
				}
				break;
			case ERROR_TAROT_READER_READY_FAIL:
				{
					/// 실패
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 308 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;
			}
		}
		break;
	case NM_TAROT_READER_READY_SYN:
		{	
			/// 다른유저가 타로카드 준비중 
			MSG_SYN_TAROT_READER_READY* pMsg = (MSG_SYN_TAROT_READER_READY*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( pMsg->mCharacterIdx );
			if( p )
			{
				p->SetStopFlag( eSTOP_READYTAROT );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_TAROT_READER_OPEN_RES:
		{
			/// 점집오픈 결과 
			MSG_RES_TAROT_READER_OPEN* msgData = (MSG_RES_TAROT_READER_OPEN*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_TAROT_READER_OPEN_SUCCESS:
				{
					cTarotControlWindow* win = GAMEUI->GetTarotControlWindow();
					if( win)
					{
						win->SetPay( msgData->Price );
						win->SetPoint( msgData->Point );
						win->SetShopName( msgData->strTitle );
						win->SetMoney( 0 );
						win->SetUserNum( 0 );

						sItemCount* p = ITEMMAN->GetItemCount( TAROT_MATERIAL_ITEM );
						unsigned int count = ( p ) ? p->bag : 0;
						win->SetStuffNum( count );
					}
					else
					{
						assert(0);
					}

					HERO->SetStopFlag( eSTOP_OPENTAROT );
					ChangePage( ePAGE_MANAGED );

					/// hero 말풍선 바꿔주기
					HERO->ActiveTarotBubble( (LPTSTR)msgData->strTitle, COLOR_SKYBLUE );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 319 ) );
				}
				break;;
			case ERROR_TAROT_READER_OPEN_CARD:
				{
					/// 카드 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 302 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;
			case ERROR_TAROT_READER_OPEN_SPREAD:
				{
					/// 스프레드 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 303 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;
			case ERROR_TAROT_READER_OPEN_ITEM:
				{
					/// 재료 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 307 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;
			case ERROR_TAROT_READER_OPEN_FAIL:
				{
					/// 실패 
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 308 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;

				// 타로 개설시 설정한 이용료가 한도(10억) 초과시
			case ERROR_TAROT_READER_OPEN_TRADE_MONEY:
				{
					/// 실패 
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11213 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;

				// 타로 개설시 설정한 이용료 + 현재 보유금이 한도(30억) 초과시
			case ERROR_TAROT_READER_OPEN_HAVE_MONEY:
				{
					/// 실패 
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11214 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;


			}
		}
		break;
	case NM_TAROT_READER_OPEN_SYN:
		{
			/// 정보 변경 / 상태 변경 클릭 가능
			MSG_SYN_TAROT_READER_OPEN* msgData = (MSG_SYN_TAROT_READER_OPEN*)msg;

			cPlayer* p = OBJECTMAN->GetPlayer( msgData->mCharacterIdx );
			if( p )
			{
				p->SetStopFlag( eSTOP_OPENTAROT );
				p->ActiveTarotBubble( msgData->mTitle, COLOR_SKYBLUE );
			}
		}
		break;
	case NM_TAROT_READER_CLOSE_RES:
		{
			/// 리더 - 점집 닫기 
			MSG_RES_TAROT_READER_CLOSE* msgData = (MSG_RES_TAROT_READER_CLOSE*)msg;
			cTarotControlWindow* tarotControlWin = GAMEUI->GetTarotControlWindow();
			
			switch( msgData->ErrorCode )
			{
			case ERROR_TAROT_READER_CLOSE_SUCCESS:
				{
					ChangePage( ePAGE_NONE );

					/// 관리상태이면 관리창 닫기
					GAMEUI->HideTarotControlWindow();

					///
					HERO->DeActiveTarotBubble();
					HERO->StandUp();

					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 320 ) );
				}
				break;
			case ERROR_TAROT_READER_CLOSE_FAIL:
				{
					/// 실패 
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 309 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;
			case ERROR_TAROT_READER_CLOSE_ITEM:
				{
					ChangePage( ePAGE_NONE );

					/// 관리상태이면 관리창 닫기
					GAMEUI->HideTarotControlWindow();

					///
					HERO->DeActiveTarotBubble();
					HERO->StandUp();
				}
			case ERROR_TAROT_READER_CLOSE_RESERVED_ON:
				{
					/// 타로 예약 상태 ON
					if( tarotControlWin )
						tarotControlWin->SetReserve( true );
				}
				break;
			case ERROR_TAROT_READER_CLOSE_RESERVED_OFF:
				{
					/// 타로 예약 상태 OFF
					if( tarotControlWin )
						tarotControlWin->SetReserve( false );
				}
				break;
			}

			if( tarotControlWin )
				tarotControlWin->NetUnLock();
		}
		break;
	case NM_TAROT_SEEKER_OPEN_RES:
		{
			/// 초기화
			Clear();

			/// 타로이용중 - 복수이용 불가
			if( mCurrentPage != ePAGE_NONE )
				assert(0);
			
			cTarotWindow* win = GAMEUI->GetTarotWindow();
			if( win == 0 )
			{
				assert(0);
				return;
			}

			/// 타로 상점 정보 받기 
			MSG_RES_TAROT_SEEKER_OPEN* msgData = (MSG_RES_TAROT_SEEKER_OPEN*)msg;

			switch( msgData->ErrorCode )
			{
			case ERROR_TAROT_SEEKER_OPEN_SUCCESS:
				{
					mTarotType = eTAROT_USER;

					win->SetTitle( msgData->Title );
					win->SetPrice( msgData->Price );

					/// 개설자의 카드 인덱스 저장 
					for( int i = 0; i < MAX_TAROT_CARDS; ++i )
					{
						unsigned int cardIndex = msgData->TarotCardIndex[i];
						if( cardIndex )
						{
							/// 윈도우에 카드 세팅 
							win->SetCardInfo( i, cardIndex, (i == 0) );
						}
						else
						{
							assert(0);
						}
					}

					/// 스프레드 인덱스 저장 
					for( int i = 0; i < MAX_SPREAD; ++i )
					{
						unsigned int spreadIndex = msgData->SpreadIndex[i];
						if( spreadIndex )
							win->SetSpreadInfo( i, spreadIndex );
					}

					HERO->SetStopFlag( eSTOP_ENTERTAROT );
					ChangePage( ePAGE_VIEW );
				}
				break;
			case ERROR_TAROT_SEEKER_OPEN_FAIL:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 310 ),
									GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
									0, GAMEUI->GetTarotWindow() );
				break;
			case ERROR_TAROT_SEEKER_OPEN_READER:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 322 ),
									GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
									0, GAMEUI->GetTarotWindow() );
				break;
			case ERROR_TAROT_SEEKER_OPEN_OVER:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 323 ),
									GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
									0, GAMEUI->GetTarotWindow() );
				break;

				// 개설자의 보유금 + 타로 이용자의 요금이 한도(30)억 초과시
			case ERROR_TAROT_SEEKER_OPEN_HAVE_MONEY:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11212 ),
					GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
					0, GAMEUI->GetTarotWindow() );
				break;
			case ERROR_TAROT_SEEKER_OPEN_RESERVED:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 327 ),
					GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
					0, GAMEUI->GetTarotWindow() );
				break;
			}
		}
		break;
	case NM_TAROT_SEEKER_CLOSE_RES:
		{
			/// 점괘 보기 취소
			MSG_RES_TAROT_SEEKER_CLOSE* msgData = (MSG_RES_TAROT_SEEKER_CLOSE*)msg;

			/// tarot state
			if( msgData->ErrorCode == ERROR_TAROT_SEEKER_CLOSE_SUCCESS )
			{
				HERO->SetState( eOBJECT_STATE_IDLE );
				ChangePage( ePAGE_NONE );
			}
			else if( msgData->ErrorCode == ERROR_TAROT_SEEKER_CLOSE_READER )
			{
				/// 점집 이용을 할 것인지 메시지 창이 떠 있으면 닫기.
				if( HERO->GetStopFlag() == eSTOP_ENTERTAROT )
				{
					UIMAN->DestroyMsgBox( eMBEVENT_TAROTSEEKER_START );
				}

				/// 리더가 종료하여 강제 관람 종료
				HERO->SetState( eOBJECT_STATE_IDLE );
				ChangePage( ePAGE_NONE );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 318 ),
					GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
					0, GAMEUI->GetTarotWindow() );
			}
			else if( msgData->ErrorCode == ERROR_TAROT_SEEKER_CLOSE_FAIL )
			{
				/// 실패
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 309 ),
					GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
					0, GAMEUI->GetTarotWindow() );

				cTarotWindow* win = GAMEUI->GetTarotWindow();
				if( win )
				{
					win->NetUnLock(ePAGE_VIEW);
					win->NetUnLock(ePAGE_SELECT);
					win->NetUnLock(ePAGE_RESULT);
				}
			}
		}
		break;
	case NM_TAROT_SEEKER_JOIN_RES:
		{
			ClearCardinfo();

			/// 점괘 보기 요청에 대한 응답 
			MSG_RES_TAROT_SEEKER_JOIN* msgData = (MSG_RES_TAROT_SEEKER_JOIN*)msg;

			if ( msgData->ErrorCode == ERROR_TAROT_SEEKER_JOIN_SUCCESS )
			{
				HERO->SetStopFlag( eSTOP_USETAROT );
				ChangePage( ePAGE_SELECT );

				/// 점괘 보기 요청 성공
				for( int i = 0; i < 10; ++i )
				{
					unsigned int cardIndex = msgData->TarotCardIndex[i];

					sResultInfo* result = new sResultInfo;
					result->TarotCardIndex = cardIndex;
					result->Dir = TAROT_SEEKER_RESULT_STRAIGHT;
					result->Value = 0;
					result->Apply = false;
					mSelectInfoArr.PushBack( result );
				}
			}
			else
			{
				//--- 이 두가지도 처리해야함
				/// 점집이용할수 없는 상태
				/// 다른캐릭터가 이용중
				
				cTarotWindow* win = GAMEUI->GetTarotWindow();
				assert(win);

				/// 이용요금 부족
				switch( msgData->ErrorCode )
				{
				case ERROR_TAROT_SEEKER_JOIN_MONEY:
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 321 ),
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, win );
					break;
				// 개설자의 보유 금액이 한도(30억) 초과로 인해 이용자가 타로 점집을 이용할 수 없을 때
				case ERROR_TAROT_SEEKER_JOIN_HAVE_MONEY :
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11212 ),
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, win );
					break;
				default: /// 그 외 부족
					/// 점괘 보기 요청 실패
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 310 ),
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, win );
					break;
				}				

				
				win->NetUnLock(ePAGE_VIEW);
			}
		}
		break;
	case NM_TAROT_SEEKER_RESULT_RES:
		{
			/// 점괘 선택완료에 대한 응답
			MSG_RES_TAROT_SEEKER_RESULT* msgData = (MSG_RES_TAROT_SEEKER_RESULT*)msg;
			cTarotWindow* win = GAMEUI->GetTarotWindow();
			if( !win )
			{
				assert(0);
				return;
			}

			win->NetUnLock( ePAGE_SELECT );
			win->SetSelectedSpread( msgData->SpreadIndex );

			cItemSpread* spread = win->GetCurrentSpreadInfo();
			if( spread == 0 )
			{
				assert(0);
				return;
			}

			for( int i = 0; i < 10; ++i )
			{
				sResultInfo* result = GetSelectCardInfo( i );
				if( result )
				{
					result->Dir = msgData->Result[i].Dir;
					result->Value = msgData->Result[i].Value;
					result->Apply = false;

					if( ( spread->mParam[i].mDir == eSPREAD_STR && result->Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ||
						( spread->mParam[i].mDir == eSPREAD_INV && result->Dir == TAROT_SEEKER_RESULT_INVERSE ) || 
						( spread->mParam[i].mDir == eSPREAD_ALL ) )
					{
						result->Apply = true;
					}
				}
			}

			ChangePage( ePAGE_RESULT );
		}
		break;
	case NM_TAROT_SEEKER_ACCEPT_RES:
		{
			MSG_RES_TAROT_SEEKER_ACCEPT* pmsg = (MSG_RES_TAROT_SEEKER_ACCEPT*)msg;

			if( pmsg->ErrorCode == ERROR_TAROT_SEEKER_ACCEPT_SUCCESS )
			{
				/// tarot state
				HERO->SetState( eOBJECT_STATE_IDLE );
				ChangePage( ePAGE_NONE );
			}
			else
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 324 ),
									GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
									0, GAMEUI->GetTarotWindow() );
			}
		}
		break;
	case NM_TAROT_READER_CLOSE_SYN:
		{
			/// 다른유저의 타롯상점 닫힘.
			MSG_SYN_TAROT_READER_CLOSE* msgData = (MSG_SYN_TAROT_READER_CLOSE*)msg;
			cPlayer* p = OBJECTMAN->GetPlayer( msgData->mCharacterIdx );
			if( p )
			{
				/// 상태 체크
				if( p->GetState() != eOBJECT_STATE_STOP )
				{
					assert(0);
				}
				if( p->GetStopFlag() != eSTOP_READYTAROT && 
					p->GetStopFlag() != eSTOP_OPENTAROT )
				{
					assert(0);
				}

				p->SetStopFlag( eSTOP_NONE );
				p->DeActiveTarotBubble();
				p->StandUp();
			}
		}
		break;
	case NM_TAROT_READER_JOIN_RES:
		{
			/// 이용자 있음.
			MSG_RES_TAROT_READER_JOIN* msgData = (MSG_RES_TAROT_READER_JOIN*)msg;

			cTarotControlWindow* pCotrol = GAMEUI->GetTarotControlWindow();
			if( pCotrol )
			{
				pCotrol->SetStatus( msgData->seekerName );
				pCotrol->SetPoint( msgData->point );
				pCotrol->SetUserNum( msgData->userCount );

				sItemCount* p = ITEMMAN->GetItemCount( TAROT_MATERIAL_ITEM );
				unsigned int count = ( p ) ? p->bag : 0;
				pCotrol->SetStuffNum( count );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_TAROT_READER_LEAVE_RES:
		{
			///이용자 나감
			MSG_RES_TAROT_READER_LEAVE* msgData = (MSG_RES_TAROT_READER_LEAVE*)msg;

			cTarotControlWindow* pCotrol = GAMEUI->GetTarotControlWindow();
			if( pCotrol )
				pCotrol->ReleaseStatus( msgData->seekerName );
			else
			{
				assert(0);
			}			
		}
		break;
	// tutorial tarot
	case NM_TAROT_TUTORIAL_SEEKER_OPEN_RES:
		{
			/// 초기화
			Clear();

			/// 타로이용중 - 복수이용 불가
			if( mCurrentPage != ePAGE_NONE )
				assert(0);

			cTarotWindow* win = GAMEUI->GetTarotWindow();
			if( win == 0 )
			{
				assert(0);
				return;
			}

			MSG_ERROR* pmsg = (MSG_ERROR*)msg;
			
			switch( pmsg->ErrorCode )
			{
			case ERROR_TAROT_TUTORIAL_SEEKER_OPEN_SUCCESS:
				{
					mTarotType = eTAROT_TUTORIAL;

					cNpc* pNpc = (cNpc*)OBJECTMAN->GetNpc( HERO->GetNpcIdx() );
					if( !pNpc )
					{
						assert(0);
						ChangePage( ePAGE_NONE );
						return;
					}

					cUINpcTelling* data = UIMAN->GetNpcTelling( pNpc->GetClassId() );
					if( !data )
					{
						assert(0);
						ChangePage( ePAGE_NONE );
						return;
					}

					unsigned long* tarotArr = data->GetTarotArr();

					// 카드 세팅
					for( int i = 0; i < MAX_TAROT_CARDS; ++i, ++tarotArr )
					{
						unsigned long cardIndex = (*tarotArr);
						if( cardIndex )
						{
							/// 윈도우에 카드 세팅 
							win->SetCardInfo( i, cardIndex, (i == 0) );
						}
						else
							assert(0);
					}

					// 스프레드 세팅
					unsigned long* spreadIndex = data->GetSpreadArr();
					for( int i = 0; i < MAX_SPREAD; ++i, ++spreadIndex )
					{
						if( (*spreadIndex) )
							win->SetSpreadInfo( i, (*spreadIndex) );
					}

					HERO->SetStopFlag( eSTOP_NPCSPEECH );
					ChangePage( ePAGE_VIEW );
				}
				break;
			case ERROR_TAROT_TUTORIAL_SEEKER_OPEN_FAIL:		// 실패
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 310 ),
								GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
								0, GAMEUI->GetTarotWindow() );
				break;
			case ERROR_TAROT_TUTORIAL_SEEKER_OPEN_OVER:		// 게스트 초과
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 323 ),
								GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
								0, GAMEUI->GetTarotWindow() );
				break;
			}
		}
		break;
	case NM_TAROT_TUTORIAL_SEEKER_CLOSE_RES:
		{
			MSG_ERROR* pmsg = (MSG_ERROR*)msg;

			if( pmsg->ErrorCode == ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_SUCCESS )
			{
				HERO->SetState( eOBJECT_STATE_IDLE );
				HERO->CloseNpcTalk();

				ChangePage( ePAGE_NONE );
			}
			else
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 309 ),
									GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
									0, GAMEUI->GetTarotWindow() );
			}
		}
		break;
	case NM_TAROT_TUTORIAL_SEEKER_JOIN_RES:
		{
			MSG_RES_TAROT_TUTORIAL_SEEKER_JOIN* pmsg = (MSG_RES_TAROT_TUTORIAL_SEEKER_JOIN*)msg;

			ClearCardinfo();

			if( pmsg->ErrorCode == ERROR_TAROT_TUTORIAL_SEEKER_JOIN_SUCCESS )
			{
				// 성공
				ChangePage( ePAGE_SELECT );

				/// 점괘 보기 요청 성공
				for( int i = 0; i < 10; ++i )
				{
					unsigned int cardIndex = pmsg->TarotCardIndex[i];

					sResultInfo* result = new sResultInfo;
					result->TarotCardIndex = cardIndex;
					result->Dir = TAROT_SEEKER_RESULT_STRAIGHT;
					result->Value = 0;
					result->Apply = false;
					mSelectInfoArr.PushBack( result );
				}
			}
			else
			{
				// 실패
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 310 ),
									GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
									0, GAMEUI->GetTarotWindow() );
			}

			//
			cTarotWindow* win = GAMEUI->GetTarotWindow();
			if( win )
				win->NetUnLock(ePAGE_VIEW);
		}
		break;
	case NM_TAROT_TUTORIAL_SEEKER_RESULT_RES:
		{
			/// 점괘 선택완료에 대한 응답
			MSG_RES_TAROT_TUTORIAL_SEEKER_RESULT* pmsg = (MSG_RES_TAROT_TUTORIAL_SEEKER_RESULT*)msg;

			cTarotWindow* win = GAMEUI->GetTarotWindow();
			if( !win )
			{
				assert(0);
				return;
			}

			win->NetUnLock( ePAGE_SELECT );
			win->SetSelectedSpread( pmsg->SpreadIndex );

			cItemSpread* spread = win->GetCurrentSpreadInfo();
			if( spread == 0 )
			{
				assert(0);
				return;
			}

			for( int i = 0; i < 10; ++i )
			{
				sResultInfo* result = GetSelectCardInfo( i );
				if( result )
				{
					result->Dir = pmsg->Result[i].Dir;
					result->Value = pmsg->Result[i].Value;
					result->Apply = false;

					if( ( spread->mParam[i].mDir == eSPREAD_STR && result->Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ||
						( spread->mParam[i].mDir == eSPREAD_INV && result->Dir == TAROT_SEEKER_RESULT_INVERSE ) || 
						( spread->mParam[i].mDir == eSPREAD_ALL ) )
					{
						result->Apply = true;
					}
				}
			}

			ChangePage( ePAGE_RESULT );
		}
		break;
	case NM_TAROT_TUTORIAL_SEEKER_ACCEPT_RES:
		{
			MSG_RES_TAROT_TUTORIAL_SEEKER_ACCEPT* pmsg = (MSG_RES_TAROT_TUTORIAL_SEEKER_ACCEPT*)msg;

			if( pmsg->ErrorCode == ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_SUCCESS )
			{
				/// tarot state
				HERO->SetState( eOBJECT_STATE_IDLE );
				HERO->CloseNpcTalk();

				ChangePage( ePAGE_NONE );
			}
			else
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 324 ),
									GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
									0, GAMEUI->GetTarotWindow() );
			}
		}
		break;
	case NM_TAROT_SELF_OPEN_RES:
		{
			MSG_ERROR* pmsg = (MSG_ERROR*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_TAROT_SELF_OPEN_SUCCESS:
				{
					mTarotType = eTAROT_SELF;

					// 
					ChangePage( ePAGE_SELFVIEW );

					HERO->SetStopFlag( eSTOP_SELFTAROT );
				}
				break;
			case ERROR_TAROT_SELF_OPEN_FAIL:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 310 ),
					GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
					0, GAMEUI->GetTarotWindow() );
				break;
			}
		}
		break;
	case NM_TAROT_SELF_CLOSE_RES:
		{
			MSG_ERROR* pmsg = (MSG_ERROR*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_TAROT_SELF_CLOSE_SUCCESS:
				{
					HERO->SetState( eOBJECT_STATE_IDLE );
					
					//
					ChangePage( ePAGE_NONE );
				}
				break;
			case ERROR_TAROT_SELF_CLOSE_FAIL:
				{
					/// 실패 
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 309 ),	// 에러 메세지
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, GAMEUI->GetTarotWindow() );
				}
				break;
			}

			cTarotWindow* win = GAMEUI->GetTarotWindow();
			if( win )
				win->NetUnLock( ePAGE_SELFVIEW );
		}
		break;
	case NM_TAROT_SELF_JOIN_RES:
		{
			MSG_RES_TAROT_SELF_JOIN* pmsg = (MSG_RES_TAROT_SELF_JOIN*)msg;

			cTarotWindow* win = GAMEUI->GetTarotWindow();

			switch( pmsg->ErrorCode )
			{
			case ERROR_TAROT_SELF_JOIN_SUCCESS:
				{
					ClearCardinfo();

					ChangePage( ePAGE_SELECT );

					/// 점괘 보기 요청 성공
					for( int i = 0; i < 10; ++i )
					{
						unsigned int cardIndex = pmsg->TarotCardIndex[i];

						sResultInfo* result = new sResultInfo;
						result->TarotCardIndex = cardIndex;
						result->Dir = TAROT_SEEKER_RESULT_STRAIGHT;
						result->Value = 0;
						result->Apply = false;
						mSelectInfoArr.PushBack( result );
					}
				}
				break;
			case ERROR_TAROT_SELF_JOIN_FAIL:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 310 ),
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, win );
				}
				break;
			case ERROR_TAROT_SELF_JOIN_METARIAL:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 307 ),
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, win );
				}
				break;
			case ERROR_TAROT_SELF_JOIN_SPREAD:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 303 ),
						GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
						0, win );
				}
				break;
			}
			
			if( win )
				win->NetUnLock( ePAGE_SELFVIEW );
		}
		break;
	case NM_TAROT_SELF_RESULT_RES:
		{
			/// 점괘 선택완료에 대한 응답
			MSG_RES_TAROT_SELF_RESULT* msgData = (MSG_RES_TAROT_SELF_RESULT*)msg;
			cTarotWindow* win = GAMEUI->GetTarotWindow();
			if( !win )
			{
				assert(0);
				return;
			}

			win->NetUnLock( ePAGE_SELECT );
			win->SetSelectedSpread( msgData->SpreadIndex );

			cItemSpread* spread = win->GetCurrentSpreadInfo();
			if( spread == 0 )
			{
				assert(0);
				return;
			}

			for( int i = 0; i < 10; ++i )
			{
				sResultInfo* result = GetSelectCardInfo( i );
				if( result )
				{
					result->Dir = msgData->Result[i].Dir;
					result->Value = msgData->Result[i].Value;
					result->Apply = false;

					if( ( spread->mParam[i].mDir == eSPREAD_STR && result->Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ||
						( spread->mParam[i].mDir == eSPREAD_INV && result->Dir == TAROT_SEEKER_RESULT_INVERSE ) || 
						( spread->mParam[i].mDir == eSPREAD_ALL ) )
					{
						result->Apply = true;
					}
				}
			}

			ChangePage( ePAGE_RESULT );
		}
		break;
	case NM_TAROT_SELF_ACCEPT_RES:
		{
			MSG_ERROR* pmsg = (MSG_ERROR*)msg;

			if( pmsg->ErrorCode == ERROR_TAROT_SELF_ACCEPT_SUCCESS )
			{
				/// tarot state
				HERO->SetState( eOBJECT_STATE_IDLE );
				ChangePage( ePAGE_NONE );
			}
			else
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 324 ),
					GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
					0, GAMEUI->GetTarotWindow() );
			}
		}
		break;
	default:
		assert( 0 && "error network message" );
		break;
	}
}

void cTarotManager::ChangePage( unsigned char page )
{
	if( mCurrentPage == page )
		return;

	switch( page )
	{
	case ePAGE_NONE:
	case ePAGE_OPEN:
	case ePAGE_VIEW:
	case ePAGE_SELECT:
	case ePAGE_RESULT:
	case ePAGE_NPCVIEW:
	case ePAGE_NPCSELECT:
	case ePAGE_NPCRESULT:
	case ePAGE_SELFVIEW:
		{
			GAMEUI->ShowTarotWindow( page );
		}
		break;
	case ePAGE_MANAGED:
		{
			/// 타로 관리 윈도우 열기
			GAMEUI->ShowTarotControlWindow();
		}
		break;
	}

	// 초기화
	if( page == ePAGE_NONE )
	{
		mTarotType = eTAROT_NONE;
	}

	mCurrentPage = (eTarotPage)page;
}

void cTarotManager::ShowErrorMsg( int error )
{
	switch( error )
	{
	case ERROR_CARD:
		{
			/// 카드가 부족함.
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 302 ),
				GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
				0, GAMEUI->GetTarotWindow() );
		}
		break;
	case ERROR_SPREAD:
		{
			/// 스프레드가 부족함
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 303 ),
				GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
				0, GAMEUI->GetTarotWindow() );
		}
		break;
	case ERROR_PRICE:
		{
			/// 가격 세팅
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 304 ),
				GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
				0, GAMEUI->GetTarotWindow() );
		}
		break;
	case ERROR_TITLE:
		{
			/// 점집명 세팅 에러
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 305 ),
				GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
				0, GAMEUI->GetTarotWindow() );
		}
		break;
	case ERROR_STUFF:
		{
			/// 재료 부족 에러
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 307 ),
				GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
				0, GAMEUI->GetTarotWindow() );
		}
		break;

	case ERROR_MAX_MONEY:
		{
			/// 등록한 이용 요금 + 인벤 보유 금액이 최대한도(30억)을 넘게 된다면!
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 325 ),
				GAMERESOURCEMAN->GetGameText( 301 ),	// 에러
				0, GAMEUI->GetTarotWindow() );
		}
		break;
	}
}
