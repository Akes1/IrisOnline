#include "stdafx.h"
#include "PartyManager.h"
#include "Protocol.h"
#include "ChatManager.h"
#include "PartyStateWindow.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "PartyWindow.h"
#include "Player.h"
#include "SkillObject.h"
#include "InputSystem.h"
#include "PVPManager.h"
#include "SmallMiniMapWindow.h"
#include "MiniMapWindow.h"
#include "ItemManager.h"
#include "GameResourceManager.h"
#include "DiceWindow.h"

#include "SkillExecuter.h"

cPartyManager* cPartyManager::mpSingleton = 0;

int DiceUserSort( const void *arg1, const void *arg2 )
{
	sDiceResult* data1 = (sDiceResult*)arg1;
	sDiceResult* data2 = (sDiceResult*)arg2;

	if( data1 && data2 )
	{
		if( data2->mRandom > data1->mRandom )
		{
			return 1;
		}
		else if( data2->mRandom < data1->mRandom )
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	assert(0);
	return 0;
}

cPartyManager::cPartyManager()
: mDivideType( PARTY_DIVIDE_NONE )
, mPartyIndex(0)
, mLeaderIndex(0)
, mCount(0)
, mLastAccumTime(0)
, mHPMPAccumTime(0)
, mSelectArrIndex(-1)
, mIsDice( false )
{
	mpSingleton = this;
	::memset( &mParty, 0, sizeof(mParty) );
}

cPartyManager::~cPartyManager()
{
	mpSingleton = 0;
}

void cPartyManager::Open()
{
	Clear();
}

void cPartyManager::Close()
{
	Clear();
}

void cPartyManager::Clear()
{
	mDivideType = PARTY_DIVIDE_NONE;
	mPartyIndex = 0;
	mLeaderIndex = 0;
	mCount = 0;
	mLastAccumTime = 0;
	mHPMPAccumTime = 0;
	mSelectArrIndex = -1;
	mIsDice = false;

	mTempTargetName.Clear();

	for( int i = 0; i < MAX_PARTY; ++i )
	{
		cPlayer* player = OBJECTMAN->GetPlayer( mParty[i].userIndex );
		if( player )
		{
			player->PartyOut_Init();
		}

		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
		{
			state->Clear();
			state->Hide();
		}

		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
		if( statemini )
		{
			statemini->Clear();
			statemini->Hide();
		}
	}

	// 초기화
	::memset( &mParty, 0, sizeof(mParty) );

	cPartyWindow* win = GAMEUI->GetPartyWindow();
	if( win )
	{
		win->Clear();
		win->Hide();
	}

	// 주사위굴리기 윈도우 닫기
	cDiceWindow* dice = GAMEUI->GetDiceWindow();
	if( dice )
	{
		dice->Hide();
	}

	cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
	cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
	if( minimap && pWide )
	{
		minimap->ClearParty();
		pWide->ClearParty();
	}

	if( HERO ) 
	{
		HERO->SetPartyIndex( 0 );
		HERO->SetPartyRoomIndex( 0 );
	}
}

void cPartyManager::Process( unsigned long accumTime )
{
	if( mCount == 0 )
		return;

	if( accumTime - mHPMPAccumTime > PARTY_DATAUPDATE )
	{
		for( int i = 0; i < mCount; ++i )
		{
			cPlayer* user = OBJECTMAN->GetPlayer( mParty[i].userIndex );
			if( user )
			{
				UpdatePlayer( i, mParty[i].userIndex, user->GetLevel(), user->GetMaxHP(), user->GetMaxMP(), 
									user->GetHP(), user->GetMP(), user->GetXPos(), user->GetYPos() );
			}
		}

		mHPMPAccumTime = accumTime;
	}

	if( accumTime - mLastAccumTime > PARTY_POSUPDATE )
	{
		UpdateMinimap();

		mLastAccumTime = accumTime;
	}
}

void cPartyManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;
		
	switch( tempMsg->Protocol )
	{
	case NM_PARTY_LIST_SYN:		/// 파티 전체 리스트
		{
			MSG_SYN_PARTY_LIST* pmsg = (MSG_SYN_PARTY_LIST*)msg;

			mPartyIndex = pmsg->mIndex;
			mLeaderIndex = pmsg->mLeaderIndex;
			mDivideType = (PARTY_DIVIDE_TYPE)pmsg->mDivideType;

			HERO->SetPartyIndex( mPartyIndex );
			
			unsigned short roomIndex = 0;
			sPartyUserData* p = pmsg->mTable;
			
			// hero 정보 찾기
			for( int i = 0; i < pmsg->RowCount; ++i, ++p )
			{
				if( HERO->GetObjectID() == p->userIndex )
				{
					roomIndex = p->roomNum;
					break;
				}
			}

			HERO->SetPartyRoomIndex( roomIndex );

			// user 삽입
			int cnt = 0;
			p = pmsg->mTable;
			for( int i = 0; i < pmsg->RowCount; ++i, ++p )
			{
				if( HERO->GetObjectID() != p->userIndex )
				{
					// hero 정보는 넣지 않는다
					if( AddPlayer( cnt, p ) == false )
					{
						assert(0);
						continue;
					}

					cnt++;
				}
			}

			cPartyWindow* win = GAMEUI->GetPartyWindow();
			if( win )
			{
				if( HERO->IsOpenNpcTalk() == false && GAMEUI->IsDuelMode() == false )
					win->Show();

				bool set = ( HERO->GetObjectID() == mLeaderIndex );
				win->SetLeader( set );

				win->SetDivideType( mDivideType );
			}
		}
		break;
	case NM_PARTY_ADDONE_SYN:
		{
			/// 파티원 추가
			MSG_SYN_PARTY_ADDONE* pmsg = (MSG_SYN_PARTY_ADDONE*)msg;

			if( AddPlayer( mCount, &pmsg->mTable ) == false )
			{
				assert(0);
				return;
			}

			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 105 ), pmsg->mTable.name );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		}
		break;
	case NM_PARTY_ADD_RES:
		{
			/// 파티 신청 결과
			MSG_RES_PARTY_ADD* pmsg = (MSG_RES_PARTY_ADD*)msg;
			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTY_ADD_SUCCESS:
				{
					/// 초대한 유저 이름 보여주기
					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( 100 ), (LPCTSTR)mTempTargetName.Cstr() );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
				}
				break;
			case ERROR_PARTY_ADD_CREATE_SUCCESS:	/// 파티생성됨
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 103 ) );
				break;
			case ERROR_PARTY_ADD_STATUS:			/// 파티 신청 불가 상태
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 135 ) );
				break;
			case ERROR_PARTY_ADD_USEROUT:			/// 초대한 유저가 게임을 종료함 
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 106 ) );
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 106 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_PARTY_ADD_COMPLETE:			/// 해당유저가 파티 가입중
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 107 ) );
				break;
			case ERROR_PARTY_ADD_CONTINUE:			/// 여러명 초대 불가
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 138 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_PARTY_ADD_USER:				/// 초대한캐릭터 없음.
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 101 ) );
				break;
			case ERROR_PARTY_ADD_COUNT:			/// 파티인원 풀
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 109 ) );
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 109 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_PARTY_ADD_REFUSE:		/// 파티 거절
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 104 ) );
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 104 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_PARTY_ADD_NOTLEADER:		/// 권한없음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 110 ) );
				break;
			case ERROR_PARTY_ADD_MAP:			/// 맵이동이라 초대할수 없음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 106 ) );
				break;
			case ERROR_PARTY_ADD_CREATEFAIL:	/// 파티 생성 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 111 ) );
				break;
			case ERROR_PARTY_ADD_OPTIONREFUSE:	/// 파티 거부 상태
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2701 ) );
				break;		
			case ERROR_PARTY_ADD_FAIL:			/// 파티 신청 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2701 ) );
				break;
			case ERROR_PARTY_ADD_PARTYUNION:
				// 파티연합중이라 실패
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 146 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				break;
			case ERROR_PARTY_ADD_REFUSETIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 147 ) );
				break;
			}

			mTempTargetName.Clear();

			/// 파티관련 UI 락 풀기
			GAMEUI->NetUnLockParty( eLOCK_PARTYADD );
		}
		break;
	case NM_PARTY_ADD_SYN:
		{
			/// 파티로부터 가입 신청 옴
			MSG_SYN_PARTY_ADD* pmsg = (MSG_SYN_PARTY_ADD*)msg;

			/// 신청
			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 131 ), pmsg->Name );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

			str.Format( GAMERESOURCEMAN->GetGameText( 132 ), pmsg->Name );

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_PARTY_ADD_REQ, eSKIN_YESNO,
				(LPCTSTR)str.Cstr(),				
				GAMERESOURCEMAN->GetGameText( 99 ),	
				0, 0, PARTY_WAIT_TIME, false, false );

		}
		break;
	case NM_PARTY_REP_RES:		/// 파티 신청 수락에 대한 결과 
		{
			/// 파티 신청 수락에 대한 결과
			UIMAN->SendEventToMsgbox( eMBEVENT_PARTY_ADD_REQ, false );

			MSG_RES_PARTY_REP* pmsg = (MSG_RES_PARTY_REP*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTY_REP_REFUSE:
				{
					/// 파티거절
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 133 ) );
				}
				break;
			case ERROR_PARTY_REP_FULL:
				{
					/// 파티인원 초과
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 112 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case ERROR_PARTY_REP_FAIL:
				{
					/// 초대한 파티 존재 하지 않음
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 114 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case ERROR_PARTY_REP_PARTYUNION:
				{
					// 파티연합중이라 실패
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 146 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case ERROR_PARTY_REP_REFUSETIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 147 ) );
				break;
			}
		}
		break;
	case NM_PARTY_OUT_RES:
		{
			MSG_RES_PARTY_OUT* pmsg = (MSG_RES_PARTY_OUT*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTY_OUT_SUCCESS:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 116 ) );

					///
					Clear();

					HERO->SetPartyIndex( 0 );

					GAMEUI->NetUnLockParty( eLOCK_PARTYOUT );
				}
				break;
			case ERROR_PARTY_OUT_FAIL:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 145 ) );
				}
				break;
			}
		}
		break;
	case NM_PARTY_OUT_SYN:		
		{
			// 파티 탈퇴
			MSG_SYN_PARTY_OUT* pmsg = (MSG_SYN_PARTY_OUT*)msg;
			
			int i = FindUser( pmsg->UserIndex );
			if( i >= 0 )
			{
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 115 ), mParty[i].name );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

				//
				DeletePlayer( i );
			}
			else
				assert(0);
		}
		break;
	case NM_PARTY_DELETE_SYN:	
		{
			// 파티 해산
			Clear();

			HERO->SetPartyIndex( 0 );

			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 141 ) );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 117 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case NM_PARTY_CHANGELEADER_RES:
		{
			MSG_RES_PARTY_CHANGELEADER* pmsg = (MSG_RES_PARTY_CHANGELEADER*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTY_CHANGELEADER_SUCESS:
				break;
			case ERROR_PARTY_CHANGELEADER_LEADER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 119 ) );
				break;
			case ERROR_PARTY_CHANGELEADER_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 120 ) );
				break;
			}

			/// 파티 관련 UI 락 풀기
			GAMEUI->NetUnLockParty( eLOCK_PARTYCHANGELEADER );
		}
		break;
	case NM_PARTY_CHANGELEADER_SYN:
		{
			/// 파티장 변경됨
			MSG_SYN_PARTY_CHANGELEADER* pmsg = (MSG_SYN_PARTY_CHANGELEADER*)msg;
			
			mLeaderIndex = pmsg->UserIndex;

			LPCTSTR name = 0;
			if( HERO->GetObjectID() == pmsg->UserIndex )
			{
				name = HERO->GetName();
			}
			else
			{
				int i = FindUser( pmsg->UserIndex );
				if( i >= 0 )
					name = mParty[i].name;
				else
					assert(0);
			}		

			//
			cPartyWindow* win = GAMEUI->GetPartyWindow();
			if( win )
			{
				bool set = (HERO->GetObjectID() == pmsg->UserIndex);
				win->SetLeader( set );
			}

			//
			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 118 ), name );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		}
		break;
	case NM_PARTY_CHANGEDIVIDE_RES:
		{
			/// 파티분배방식 변경 요청 결과
			MSG_RES_PARTY_CHAGNEDIVIDE* pmsg = (MSG_RES_PARTY_CHAGNEDIVIDE*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTY_CHANGEDIVIDE_SUCESS:
				break;
			case ERROR_PARTY_CHANGEDIVIDE_LEADER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
				break;
			case ERROR_PARTY_CHANGEDIVIDE_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 123 ) );
				break;
			}

			/// 파티 관련 UI 
			GAMEUI->NetUnLockParty( eLOCK_PARTYCHANGEDIVIDE );
		}
		break;
	case NM_PARTY_CHANGEDIVIDE_SYN:
		{
			/// 파티분배방식 변경 알림
			MSG_SYN_PARTY_CHANGEDIVDE* pmsg = (MSG_SYN_PARTY_CHANGEDIVDE*)msg;
			mDivideType = (PARTY_DIVIDE_TYPE)pmsg->type;

			unsigned int textId = 0;
			switch( mDivideType )
			{
			case PARTY_DIVIDE_TURN:		textId = 265;	break;
			case PARTY_DIVIDE_RANDOM:	textId = 266; 	break;
			case PARTY_DIVIDE_DAMAGE:	textId = 267; 	break;
			case PARTY_DIVIDE_FREE:		textId = 268; 	break;
			case PARTY_DIVIDE_LEADER:	textId = 269;	break;
			default:					assert(0);		return;
			}

			if( textId > 0 )
			{
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 124 ), UIMAN->GetUIText( textId ) );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
			}

			/// UI 세팅
			cPartyWindow* pPartyWindow = GAMEUI->GetPartyWindow();
			if( pPartyWindow )
			{
				pPartyWindow->SetDivideType( mDivideType );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_PARTY_THROW_RES:
		{
			/// 파티원 추방 결과
			MSG_RES_PARTY_THROW* pmsg = (MSG_RES_PARTY_THROW*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTY_THROW_SUCESS:
				break;
			case ERROR_PARTY_THROW_LEADER:
				/// 권한 없음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
				break;
			case ERROR_PARTY_THROW_FAIL:
				/// 추방 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 125 ) );
				break;
			}

			/// 파티 관련 UI 
			GAMEUI->NetUnLockParty( eLOCK_PARTYTHROW );
		}
		break;
	case NM_PARTY_THROW_SYN:
		{
			/// 파티원 추방 알림
			MSG_SYN_PARTY_THROW* pmsg = (MSG_SYN_PARTY_THROW*)msg;
			int i = FindUser( pmsg->UserIndex );
			if( i >= 0 )
			{
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 126 ), mParty[i].name );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

				DeletePlayer( i );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_PARTY_CUT_SYN:
		{
			/// HERO가 파티로부터 추방당함
			Clear();

			HERO->SetPartyIndex( 0 );

			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 127 ) );

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 127 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case NM_PARTY_MAPCHANGE_SYN:
		{
			MSG_SYN_PARTY_MAPCHANGE* pmsg = (MSG_SYN_PARTY_MAPCHANGE*)msg;

			int i = FindUser( pmsg->UserIndex );
			if( i >= 0 )
			{
				UpdateMap( i, pmsg->MapNum, pmsg->RoomNum, pmsg->ChannelNum );

				cStringT str;
				unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);

				CHATMANAGER->AddChatMsgEx( (LPCTSTR)GAMERESOURCEMAN->GetGameText( 11101 ), COLOR_WHITE, infoIndex, false, true );
				CHATMANAGER->AddChatMsgEx( (LPCTSTR)mParty[i].name, COLOR_PARTY_EX, 0, true, true );

				str.Format( GAMERESOURCEMAN->GetGameText( 11105 ), GAMERESOURCEMAN->GetMapName( pmsg->MapNum ) );
				CHATMANAGER->AddChatMsgEx( (LPCTSTR)str.Cstr(), COLOR_WHITE, 0, true, true );				
			}
			else
				assert(0);
		}
		break;
	case NM_PARTY_JOBCHANGE_SYN:
		{
			/// 전직 정보
			MSG_SYN_PARTY_CHANGEJOB* pmsg = (MSG_SYN_PARTY_CHANGEJOB*)msg;

			int i = FindUser( pmsg->userIndex );
			if( i >= 0 )
			{
				UpdateJob( i, (ePLAYER_JOB)pmsg->job );
			}
			else
				assert(0);
		}
		break;
	case NM_PARTY_DATA_SYN:
		{
			// 일정시간마다 데이터 갱신
			MSG_SYN_PARTY_DATA* pmsg = (MSG_SYN_PARTY_DATA*)msg;

			int i = FindUser( pmsg->userIndex );
			if( i >= 0 )
			{
				// 기본정보 갱신
				UpdatePlayer( i, pmsg->userIndex, pmsg->level, pmsg->maxHP, pmsg->maxMP, pmsg->hp, pmsg->mp, pmsg->posX, pmsg->posY );

				// 버프 갱신
				if( UpdateBuff( i, pmsg->mTable, pmsg->RowCount ) == false )
				{
					assert(0);
					return;
				}

				UpdateMouseHover();
			}
			else
				assert(0);
		}
		break;
	case NM_PARTY_DATA_LEVEL_SYN:
		{
			/// 동일 맵에 존재하지 않을때 레벨 정보만 받는다
			MSG_SYN_PARTYLEVEL_DATA* pmsg = (MSG_SYN_PARTYLEVEL_DATA*)msg;

			int i = FindUser( pmsg->userIndex );
			if( i >= 0 )
			{
				// 기본정보 갱신
				UpdatePlayer( i, pmsg->userIndex, pmsg->level, mParty[i].maxhp, mParty[i].maxmp, mParty[i].hp, mParty[i].mp, mParty[i].posX, mParty[i].posY );
			}
			else
				assert(0);
		}
		break;
	case NM_PARTY_ITEMGET_SYN:
		{
			/// 파티원들의 아이템 획득
			MSG_SYN_PARTY_ITEMGET* pmsg = (MSG_SYN_PARTY_ITEMGET*)msg;

			cPlayer *pPartyPlayer;
			pPartyPlayer = OBJECTMAN->GetPlayer( pmsg->UserIndex );

			int i = FindUser( pmsg->UserIndex );
			if( i >= 0 )
			{
				cStringT str;
				for( short idx = 0; idx < pmsg->RowCount; ++idx )
				{
					str.Format( GAMERESOURCEMAN->GetGameText( 139 ), mParty[i].name );
					CHATMANAGER->AddItemMsg( pmsg->ItemIndex[idx], USHRT_MAX, (LPCTSTR)str.Cstr(), GAMERESOURCEMAN->GetGameText( 140 ) );

					/// 파티원 아이템 획득 이펙트 보여주기
					if( pPartyPlayer )
					{
						cItemDefine* itemDefine = ITEMMAN->GetItemDefine( pmsg->ItemIndex[idx] );
						if( itemDefine )
						{
							unsigned short itemRareLevel = itemDefine->GetTipType();

							/// 아이템 획득 이펙트. 레어도에 따라 다름.
							NiTransform trans;
							trans.MakeIdentity();
							trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );

							if( itemRareLevel < eTOOLTIP_RARE )
								pPartyPlayer->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_itemget_01.nif", &trans, false, true );
							else
								pPartyPlayer->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_itemget_02.nif", &trans, false, true );
						}
					}
				}
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_PARTY_DICE_START_RES:
		{
			MSG_ERROR* pmsg = (MSG_ERROR*)msg;

			cPartyWindow* win = GAMEUI->GetPartyWindow();
			if( win )
				win->NetUnLock();

			switch( pmsg->ErrorCode )
			{
			case ERROR_DICE_START_SUCCESS:
				{
					unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
					CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetGameText( 650 ), COLOR_SYSTEM, infoIndex, false, true );

					if( win )
						win->SetEnableDice( false );
				}
				break;
			case ERROR_DICE_START_FAIL:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 657 ) );
				}
				break;
			case ERROR_DICE_START_PARTY_LEADER:		/// 실패 - 파티장
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
				}
				break;
			case ERROR_DICE_START_PARTY_STATE:		/// 실패 - 파티 상태
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 114 ) );
				}
				break;
			}
		}
		break;
	case NM_PARTY_DICE_START_SYN:
		{
			// 주사위굴리기중
			mIsDice = true;

			// 주사위 굴리기 시작
			cDiceWindow* dice = GAMEUI->GetDiceWindow();
			if( dice )
			{
				dice->StartDice( PARTY_DICETIME );
			}
			else
				assert(0);
		}
		break;
	case NM_PARTY_DICE_ROLL_RES:		 /// 주사위 굴리기/포기 선택
		{
			MSG_RES_PARTY_DICE_ROLL* pmsg = (MSG_RES_PARTY_DICE_ROLL*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_DICE_ROLL_SUCCESS:
				{
					if( pmsg->mRandom == 0 )
					{
						// 포기
						unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
						CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetGameText( 651 ), COLOR_SYSTEM, infoIndex, false, true );
					}
					else
					{
						// 굴리기
						unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
						CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetGameText( 660 ), COLOR_SYSTEM, infoIndex, false, true );

						cStringT str;
						str.Format( _T(" %d"), pmsg->mRandom );
						CHATMANAGER->AddChatMsgEx( (LPCTSTR)str.Cstr(), COLOR_DICENUM, 0, true, true );
						CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetGameText( 661 ), COLOR_SYSTEM, 0, true, true );
					}
				}
				break;
			case ERROR_DICE_ROLL_NOT_START:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 653 ) );
				break;
			case ERROR_DICE_ROLL_ALREADY:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 654 ) );
				break;
			case ERROR_DICE_ROLL_PARTY_STATE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 114 ) );
				break;
			case ERROR_DICE_ROLL_MEMBER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 658 ) );
				break;
			}

			cDiceWindow* dice = GAMEUI->GetDiceWindow();
			if( dice )
				dice->NetUnLock();
		}
		break;
	case NM_PARTY_DICE_REMAIN_TIME_SYN:
		{
			//맵이동후 시간 재 세팅
			MSG_SYN_PARTY_DICE_REMAIN_TIME* pmsg = (MSG_SYN_PARTY_DICE_REMAIN_TIME*)msg;

			if( pmsg->mTime > 0 )
			{
				// 주사위굴리기중
				mIsDice = true;

				// 주사위 굴리기 시작
				cDiceWindow* dice = GAMEUI->GetDiceWindow();
				if( dice )
				{
					dice->StartDice( pmsg->mTime );
				}
			}
		}
		break;
	case NM_PARTY_DICE_END_SYN:			/// 주사위 굴리기 종료
		{
			MSG_SYN_PARTY_DICE_END* pmsg = (MSG_SYN_PARTY_DICE_END*)msg;

			/// 결과 출력
			if( pmsg->mTotalCnt > pmsg->mNotRollCnt )
			{
				/// 한명이상 굴린 경우
				cStringT str;
				//"주사위 굴리기 : 총 %d 명(포기 %d 명)
				str.Format( GAMERESOURCEMAN->GetGameText( 655 ), pmsg->mTotalCnt, pmsg->mNotRollCnt );

				unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
				CHATMANAGER->AddChatMsgEx( (LPCTSTR)str.Cstr(), COLOR_SYSTEM, infoIndex, false, true );

				sDiceResult temp[MAX_PARTYUSER];
				::memset( &temp, 0, sizeof(temp) );

				/// 검증코드
				if( pmsg->mRowCount > MAX_PARTYUSER )
				{
					assert(0);
					return;
				}

				int count = pmsg->mTotalCnt - pmsg->mNotRollCnt;
				if( count > MAX_PARTYUSER )
				{
					assert(0);
					count = MAX_PARTYUSER;
				}

				if( count > pmsg->mRowCount )
				{
					assert(0);
					count = pmsg->mRowCount;
				}
				///

				::memcpy( &temp, pmsg->mResult, count*sizeof(sDiceResult) );

				/// 정렬
				::qsort( (void*)temp, count, sizeof(sDiceResult), DiceUserSort );

				// 동일 점수는 동일 등수로 출력하기 위함
				sDiceResult* data = temp;
				int rank = 0;
				int tempRandom = 0;
				for( int i = 0; i < count; ++i, ++data )
				{
					if( tempRandom != data->mRandom )
					{
						tempRandom = data->mRandom;
						rank = i+1;
					}

					// 문자열 조합
					// "[%s]님이 %d등 입니다. ( %d )
					unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
					str.Format( GAMERESOURCEMAN->GetGameText( 656 ), data->mCharName, rank );
					CHATMANAGER->AddChatMsgEx( (LPCTSTR)str.Cstr(), COLOR_SYSTEM, infoIndex, false, true );

					str.Format( _T("%d"), data->mRandom );
					CHATMANAGER->AddChatMsgEx( (LPCTSTR)str.Cstr(), COLOR_DICENUM, 0, true, true );
					CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetGameText( 659 ), COLOR_SYSTEM, 0, true, true );
				}
			}
			else if( pmsg->mTotalCnt == pmsg->mNotRollCnt )
			{
				/// 모두 포기인 경우
				cStringT str;
				//"주사위 굴리기 : 총 %d 명(포기 %d 명)
				str.Format( GAMERESOURCEMAN->GetGameText( 655 ), pmsg->mTotalCnt, pmsg->mNotRollCnt );

				unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);
				CHATMANAGER->AddChatMsgEx( (LPCTSTR)str.Cstr(), COLOR_SYSTEM, infoIndex, false, true );
				CHATMANAGER->AddChatMsgEx( GAMERESOURCEMAN->GetGameText(662), COLOR_SYSTEM, infoIndex, false, true );
			}
			else
			{
				/// error
				assert(0);
			}

			// 굴리기 윈도우 닫기
			cDiceWindow* dice = GAMEUI->GetDiceWindow();
			if( dice )
				dice->Hide();

			// 파티윈도우 주사위 활성화
			cPartyWindow* win = GAMEUI->GetPartyWindow();
			if( win )
				win->SetEnableDice( true );

			// 주사위굴리기 완료
			mIsDice = false;
		}
		break;
	default:
		assert(0);
		break;
	}
}

bool cPartyManager::AddPlayer( int i, sPartyUserData* data )
{
	if( mCount >= MAX_PARTY )
	{
		assert(0);
		return false;
	}

	/// data
	mParty[i] = *data;

	/// ui setting
	cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
	cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
	if( minimap && pWide )
	{
		minimap->AddParty( &mParty[i] );
		pWide->AddParty( &mParty[i] );
	}

	cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( mCount );
	cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( mCount );
	if( state && statemini )
	{
		state->SetInfo( data->userIndex, data->name, data->level, (ePLAYER_JOB)data->job, 
						data->maxhp, data->hp, data->maxmp, data->mp, data->mapNum, data->roomNum, data->channelNum, i );

		statemini->SetInfo( data->userIndex, data->name, data->maxhp, data->hp, 
							data->mapNum, data->mp, data->mapNum, data->roomNum, data->channelNum, i );

		//
		ShowStateUI( i );

		// 버프 세팅
		cPlayer* user = OBJECTMAN->GetPlayer( mParty[i].userIndex );
		if( user )
		{
			UpdateBuff( i, user );
		}

		UpdateMouseHover();
	}
	else
	{
		assert(0);
		return false;
	}

	mCount++;

	cPlayer* player = OBJECTMAN->GetPlayer( mParty[i].userIndex );
	if( player )
	{
		player->PartyIn_Init();
	}
	return true;
}

bool cPartyManager::DeletePlayer( int i )
{
	unsigned long userIndex = mParty[i].userIndex;

	// data
	sPartyUserData* data = &mParty[i];
	::memset( data, 0, sizeof(sPartyUserData) );

	// 땡기기
	sPartyUserData* data1 = mParty + i;
	for( int idx = i; idx < MAX_PARTY - 1; ++idx )
	{
		sPartyUserData* data2 = data1 + 1;
		*data1 = *data2;

		data1++;
	}

	sPartyUserData* last = &mParty[MAX_PARTY-1];
	::memset( last, 0, sizeof(sPartyUserData) );

	if( mCount > 0 )
		mCount--;
	else
		assert(0);

	cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
	cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
	if( minimap && pWide )
	{
		minimap->DeleteParty( userIndex );
		pWide->DeleteParty( userIndex );
	}

	cPlayer* player = OBJECTMAN->GetPlayer( userIndex );
	if( player )
	{
		player->PartyOut_Init();
	}


	// ui setting
	// 전체 리스트 재 세팅
	sPartyUserData* p = mParty;
	for( int i = 0; i < MAX_PARTY; ++i, ++p )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
		if( state && statemini )
		{
			state->Clear();
			statemini->Clear();

			if( p->userIndex > 0 )
			{
				state->SetInfo( p->userIndex, p->name, p->level, (ePLAYER_JOB)p->job, 
								p->maxhp, p->hp, p->maxmp, p->mp, p->mapNum, p->roomNum, p->channelNum, i );

				statemini->SetInfo( p->userIndex, p->name, p->maxhp, p->hp, 
									p->maxmp, p->mp, p->mapNum, p->roomNum, p->channelNum, i );

				//
				ShowStateUI( i );

				// 버프 세팅
				cPlayer* user = OBJECTMAN->GetPlayer( mParty[i].userIndex );
				if( user )
				{
					UpdateBuff( i, user );
				}

				UpdateMouseHover();
			}
			else
			{
				HideStateUI( i );
			}
		}
	}
	return true;
}

void cPartyManager::UpdatePlayer( unsigned int i, unsigned long userIndex, unsigned char level, 
								  unsigned int maxhp, unsigned int maxmp, 
								  unsigned int hp, unsigned int mp, float posX, float posY )
{
	if( i >= 0 && i < MAX_PARTY )
	{
		if( mParty[i].userIndex == userIndex )
		{
			// 정보 갱신
			mParty[i].level = level;
			mParty[i].maxhp = maxhp;
			mParty[i].hp	= hp;
			mParty[i].maxmp = maxmp;
			mParty[i].mp    = mp;
			mParty[i].posX  = posX;
			mParty[i].posY  = posY;

			// ui
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
			if( state && state->GetUserIndex() == userIndex )
			{
				state->UpdateInfo( level, maxhp, hp, maxmp, mp );
			}
			else
				assert(0);

			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
			if( statemini && statemini->GetUserIndex() == userIndex )
			{
				statemini->UpdateInfo( maxhp, hp, maxmp, mp );
			}
			else
				assert(0);
		}
		else
			assert(0);
	}
	else
		assert(0);
}

bool cPartyManager::UpdateBuff( unsigned int i, sInfluenceClient* data, long count )
{
	if( i >= 0 && i < MAX_PARTY )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
		{
			state->ClearBuff();
			for( long i = 0; i < count; ++i, ++data )
			{
				state->AddBuff( data->mUniqueIdx, data->mInfluenceClassIdx );
			}
		}
	}
	else
	{
		assert(0);
		return false;
	}

	return true;
}

void cPartyManager::ClearAllBuff( unsigned long objectIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 )
	{	
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
			state->ClearBuff();
	}
	else
		assert(0);
}

void cPartyManager::AddBuff( unsigned long objectIdx, unsigned long uniqueIdx, unsigned long influenceIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 )
	{	
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
		{
			state->AddBuff( uniqueIdx, influenceIdx );
		}

		UpdateMouseHover();
	}
}

void cPartyManager::DeleteBuff( unsigned long objectIdx, unsigned long uniqueIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
		{
			state->DelBuff( uniqueIdx );
		}

		UpdateMouseHover();
	}
}

void cPartyManager::DeleteDeBuff( unsigned long objectIdx, unsigned long uniqueIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
		{
			state->DelDeBuff( uniqueIdx );
		}

		UpdateMouseHover();
	}
}

bool cPartyManager::UpdateBuff( unsigned int i, cPlayer* player )
{
	typedef tPointerHashMap<unsigned long, void*> cInfluenceMap;

	if( player != NULL )
	{
		if( i >= 0 && i < MAX_PARTY )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
			if( state )
			{
				// 세팅	
				state->ClearBuff();

				cInfluenceMap* buffMap = player->GetBufMap();
				cInfluenceMap* debuffMap = player->GetDeBufMap();
				if( buffMap )
				{
					/// 버프 정보 세팅
					cInfluenceMap::cIterator b = buffMap->Begin();
					cInfluenceMap::cIterator end = buffMap->End();
					for( ; b != end; ++b )
					{
						unsigned long uniqueIndex = (unsigned long)(b->mFirst);
						cInfluenceObject* p = (cInfluenceObject*)(b->mSecond);
						if( p )
							state->AddBuff( uniqueIndex, p->GetInfluenceIdx() );
					}
				}

				if( debuffMap )
				{
					/// 디버프 정보 세팅
					cInfluenceMap::cIterator b = debuffMap->Begin();
					cInfluenceMap::cIterator end = debuffMap->End();
					for( ; b != end; ++b )
					{
						unsigned long uniqueIndex = (unsigned long)(b->mFirst);
						cInfluenceObject* p = (cInfluenceObject*)(b->mSecond);
						if( p )
							state->AddBuff( uniqueIndex, p->GetInfluenceIdx() );
					}
				}	
			}
		}
		else
		{
			assert(0);
			return false;
		}
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

void cPartyManager::UpdateMap( unsigned int i, unsigned short mapnum, unsigned short roomnum, DWORD channelnum )
{
	if( i >= 0 && i < MAX_PARTY )
	{
		mParty[i].mapNum = mapnum;
		mParty[i].roomNum = roomnum;
		mParty[i].channelNum = channelnum;

		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
		if( state && statemini )
		{
			state->UpdateMap( mParty[i].mapNum, mParty[i].roomNum, mParty[i].channelNum );
			statemini->UpdateMap( mParty[i].mapNum, mParty[i].roomNum, mParty[i].channelNum );
		}

		cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
		cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
		if( minimap && pWide )
		{
			minimap->UpdatePartyMap( mParty[i].userIndex, mParty[i].mapNum, mParty[i].roomNum, mParty[i].channelNum );
			pWide->UpdatePartyMap( mParty[i].userIndex, mParty[i].mapNum, mParty[i].roomNum, mParty[i].channelNum );
		}
	}
	else
		assert(0);
}

void cPartyManager::UpdateJob( unsigned int i, ePLAYER_JOB job )
{
	if( i >= 0 && i < MAX_PARTY )
	{
		mParty[i].job = (unsigned short)job;

		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
		{
			state->UpdateJob( job );
		}
	}
	else
		assert(0);
}

/// 근접 유저 정보 세팅
void cPartyManager::UpdateLevel( unsigned long userIndex, unsigned char level )
{
	int i = FindUser( userIndex );
	if( i >= 0 && i < MAX_PARTY )
	{
		mParty[i].level = level;

		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
			state->SetLevel( level );
	}
	else
		assert(0);
}

void cPartyManager::SetTargetByUI( unsigned long userIndex )
{
	if( HERO->GetPartyIndex() == 0 )
		return;

	if( HERO->GetObjectID() == userIndex )
		return;

	int i = FindUser( userIndex );
	SetTargetByArrIndex( i );
}

void cPartyManager::SetTargetByArrIndex( unsigned int i )
{
	if( i >= 0 && i < MAX_PARTY )
	{
		if( SKILLEXECUTER->IsSkillCharge() == true )
		{
			cBaseObject* curTarget = HERO->GetTargetObject();
			if( curTarget == 0 || curTarget->GetState() == eOBJECT_STATE_DIE )
			{
				/// 신규 타겟 등록후 발동
				HERO->SetTargetObject( eOBJECTTYPE_PLAYER, mParty[i].userIndex );
			}
			else
			{
				/// 타겟 변경 없이 발동
				cBaseObject* target = OBJECTMAN->GetObject( eOBJECTTYPE_PLAYER, mParty[i].userIndex );

				sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
				SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, target );
				return;
			}
		}
		else
		{
			HERO->SetTargetObject( eOBJECTTYPE_PLAYER, mParty[i].userIndex );
		}

		// 파티 윈도우세팅
		for( unsigned int w = 0; w < 5; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				state->SetSelect( false );
				statemini->SetSelect( false );
			}
		}

		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
		if( state && statemini )
		{
			state->SetSelect( true );
			statemini->SetSelect( true );
		}

		//
		mSelectArrIndex = i;
	}
	else
		assert(0);
}

void cPartyManager::SetTargetByHero( unsigned long userIndex )
{
	if( HERO->GetPartyIndex() == 0 )
		return;

	if( HERO->GetObjectID() == userIndex )
		return;

	int i = FindUser( userIndex );
	if( i >= 0 && i < MAX_PARTY )
	{
		// 파티 윈도우세팅
		for( unsigned int w = 0; w < 5; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				state->SetSelect( false );
				statemini->SetSelect( false );
			}
		}

		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
		if( state && statemini )
		{
			state->SetSelect( true );
			statemini->SetSelect( true );
		}

		//
		mSelectArrIndex = i;
	}
}

void cPartyManager::ClearTarget()
{
	if( HERO->GetPartyIndex() == 0 )
		return;

	if( mSelectArrIndex >= 0 && mSelectArrIndex < 5 )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( mSelectArrIndex );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( mSelectArrIndex );
		if( state && statemini )
		{
			state->SetSelect( false );
			statemini->SetSelect( false );
		}

		mSelectArrIndex = -1;
	}
}

void cPartyManager::UpdateMouseHover()
{
	for( int i = 0; i < mCount; ++i )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
			state->UpdateMouseHover( cUIPos( MOUSE->GetMouseX(), MOUSE->GetMouseY() ));
	}
}

void cPartyManager::UpdateMinimap()
{
	cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
	cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
	if( minimap && pWide )
	{
		for( int i = 0; i < mCount; ++i )
		{
			minimap->UpdatePartyPos( mParty[i].userIndex, mParty[i].posX, mParty[i].posY );
			pWide->UpdatePartyPos( mParty[i].userIndex, mParty[i].posX, mParty[i].posY );
		}
	}
}

LPCTSTR cPartyManager::GetSelectUserName()
{
	if( mSelectArrIndex > -1 )
	{
		return mParty[mSelectArrIndex].name;
	}
	return 0;
}

int cPartyManager::FindUser( unsigned long playerIndex )
{
	if( playerIndex == 0 )
		return -1;

	for( int i = 0; i < MAX_PARTY; ++i )
	{
		if( mParty[i].userIndex == playerIndex )
			return i;
	}
	return -1;
}

bool cPartyManager::IsHeroLeader()
{
	return (mLeaderIndex == HERO->GetObjectID());
}

bool cPartyManager::IsHeroParty( unsigned long userIndex )
{
	if( mCount == 0 )
		return false;

	return ( FindUser( userIndex ) >= 0  );
}

void cPartyManager::ShowUI()
{
	if( HERO->GetPartyIndex() == 0 )
		return;

	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin )
		partyWin->Hide();

	for( int i = 0; i < mCount; ++i )
	{
		ShowStateUI( i );
	}
}

void cPartyManager::HideUI()
{
	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin )
		partyWin->Hide();

	for( int i = 0; i < MAX_PARTY; ++i )
	{
		HideStateUI( i );
	}
}

// display 모드에 따른 윈도우 보기 설정
void cPartyManager::ShowStateUI( int i )
{
	// npc 대화중, 혹은 결투 모드이면 리턴
	if( HERO->IsOpenNpcTalk() || GAMEUI->IsDuelMode() )
		return;

	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin )
	{
		bool show = false;
		switch( partyWin->GetDisplayType() )
		{
		case ePARTY_STATE_DISPLAY_NORMAL:
			{
				cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
				if( state && state->GetUserIndex() > 0 )
				{
					state->Show();
					show = true;
				}
			}
			break;
		case ePARTY_STATE_DISPLAY_MINI:
			{
				cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
				if( statemini && statemini->GetUserIndex() > 0 )
				{
					statemini->Show();
					show = true;
				}
			}
			break;
		case ePARTY_STATE_DISPLAY_HIDE:
			show = true;
			break;
		}

		if( show == true )
			partyWin->Show();
	}
}

void cPartyManager::HideStateUI( int i )
{
	cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
	if( state )
		state->Hide();

	cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
	if( statemini )
		statemini->Hide();
}

void cPartyManager::ChangeDisplayType( unsigned char type )
{
	for( int i = 0; i < MAX_PARTY; ++i )
	{
		HideStateUI( i );
	}

	// hide 모드가 아니면 다시 보여주기
	if( type != ePARTY_STATE_DISPLAY_HIDE )
	{
		for( int i = 0; i < mCount; ++i )
		{
			ShowStateUI( i );
		}
	}
}

void cPartyManager::RequestPartyAdd( LPCTSTR targetName )
{
	/// 파티 가입중일때는 자신이 파티장이어야 함
	if( HERO->GetPartyIndex() > 0 )
	{
		if( mLeaderIndex != HERO->GetObjectID() )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 110 ) );
			return;
		}
	}

	/// 자기자신 초대불가
	if( ::_tcscmp( targetName, HERO->GetName() ) == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
		return;
	}

	/// 파티연합중에는 파티를 맺을수 없음
	if( HERO->GetPartyUnionIndex() > 0 )
	{	
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 146 ) );
		return;
	}

	mTempTargetName = targetName;

	///
	MSG_REQ_PARTY_ADD msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_PARTY;
	msg.Protocol = NM_PARTY_ADD_REQ;
	Sstrncpy( msg.Name, MAX_NAME_BUFFER_SIZE, targetName, MAX_NAME_SIZE );
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 파티 관련 UI 모두 락
	GAMEUI->NetLockParty( eLOCK_PARTYADD );
}

void cPartyManager::RequestChangeLeader()
{
	/// 파티검사
	if( HERO->GetPartyIndex() == 0 )
		return;

	/// 파티장 검사
	if( mLeaderIndex != HERO->GetObjectID() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
		return;
	}

	if( mSelectArrIndex < 0 || mSelectArrIndex >= MAX_PARTY )
	{
		assert(0);
		return;
	}

	unsigned long userIndex = mParty[mSelectArrIndex].userIndex;
	if( userIndex == 0 )
	{
		assert(0);
		return;
	}

	///
	if( userIndex != HERO->GetObjectID() )
	{
		MSG_REQ_PARTY_CHANGELEADER msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_PARTY;
		msg.Protocol = NM_PARTY_CHANGELEADER_REQ;
		msg.UserIndex = userIndex;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 파티 관련 UI 모두 락
		GAMEUI->NetLockParty( eLOCK_PARTYCHANGELEADER );
	}
}

void cPartyManager::RequestChangeLeader( unsigned long playerIdx )
{
	/// 파티검사
	if( HERO->GetPartyIndex() == 0 )
		return;

	/// 파티장 검사
	if( mLeaderIndex != HERO->GetObjectID() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
		return;
	}

	int i = FindUser( playerIdx );
	if( i >= 0 )
	{
		MSG_REQ_PARTY_CHANGELEADER msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_PARTY;
		msg.Protocol = NM_PARTY_CHANGELEADER_REQ;
		msg.UserIndex = mParty[i].userIndex;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 파티 관련 UI 모두 락
		GAMEUI->NetLockParty( eLOCK_PARTYCHANGELEADER );
	}
	else
		assert(0);
}

void cPartyManager::RequestPartyOut()
{
	/// 파티가 아니면 탍퇴 불가
	if( HERO->GetPartyIndex() == 0 )
		return;

	MSG_REQ_PARTY_OUT msg;
	::memset( &msg, 0, sizeof( msg ) );
	msg.Category = NM_PARTY;
	msg.Protocol = NM_PARTY_OUT_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 파티 관련 UI 모두 락
	GAMEUI->NetLockParty( eLOCK_PARTYOUT );
}

void cPartyManager::RequestThrow()
{
	/// 파티검사
	if( HERO->GetPartyIndex() == 0 )
		return;

	/// 파티장 검사
	if( mLeaderIndex != HERO->GetObjectID() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
		return;
	}

	if( mSelectArrIndex == -1 )
	{
		assert(0);
		return;
	}

	unsigned long userIndex = mParty[mSelectArrIndex].userIndex;
	if( userIndex == 0 )
	{
		assert(0);
		return;
	}

	/// 자기자신은 추방할수 없음
	if( userIndex == HERO->GetObjectID() )
		return;

	MSG_REQ_PARTY_THROW msg;
	::memset( &msg, 0, sizeof( msg ) );
	msg.Category = NM_PARTY;
	msg.Protocol = NM_PARTY_THROW_REQ;
	msg.UserIndex = userIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 파티 관련 UI 모두 락
	GAMEUI->NetLockParty( eLOCK_PARTYTHROW );
}

void cPartyManager::RequestThrow( unsigned long playerIdx )
{
	/// 파티검사
	if( HERO->GetPartyIndex() == 0 )
		return;

	/// 파티장 검사
	if( mLeaderIndex != HERO->GetObjectID() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
		return;
	}

	int i = FindUser( playerIdx );
	if( i >= 0 )
	{
		MSG_REQ_PARTY_THROW msg;
		::memset( &msg, 0, sizeof( msg ) );
		msg.Category = NM_PARTY;
		msg.Protocol = NM_PARTY_THROW_REQ;
		msg.UserIndex = playerIdx;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 파티 관련 UI 모두 락
		GAMEUI->NetLockParty( eLOCK_PARTYTHROW );
	}
	else
	{
		assert(0);
	}
}

void cPartyManager::RequestChangeDivide( PARTY_DIVIDE_TYPE type )
{
	/// 파티검사
	if( HERO->GetPartyIndex() == 0 )
		return;

	/// 파티장 검사
	if( mLeaderIndex != HERO->GetObjectID() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
		return;
	}

	if( type == mDivideType )
		return;

	if( type == PARTY_DIVIDE_NONE )
	{
		assert(0);
		return;
	}

	MSG_REQ_PARTY_CHANGEDIVIDE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_PARTY;
	msg.Protocol = NM_PARTY_CHANGEDIVIDE_REQ;
	msg.type = (char)type;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 파티 관련 UI 모두 락
	GAMEUI->NetLockParty( eLOCK_PARTYCHANGEDIVIDE );
}

sPartyUserData* cPartyManager::GetPartyInfo( unsigned int index )
{
	if( index >= MAX_PARTY )
	{
		assert( 0 && "bad index");
		return 0;
	}
	else
	{
		return &mParty[index];
	}
}

sPartyUserData* cPartyManager::GetPartyInfoByUserIndex( unsigned long userIndex )
{
	int i = FindUser( userIndex );
	if( i >= 0 )
	{
		return &mParty[i];
	}
	return 0;
}
