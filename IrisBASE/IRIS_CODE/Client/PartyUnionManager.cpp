#include "stdafx.h"
#include "PartyUnionManager.h"
#include "Protocol.h"
#include "ChatManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "PartyUnionWindow.h"
#include "GameUIManager.h"
#include "PartyStateWindow.h"
#include "SkillObject.h"
#include "PartyWindow.h"
#include "InputSystem.h"
#include "SmallMiniMapWindow.h"
#include "MiniMapWindow.h"
#include "GameResourceManager.h"

#include "SkillExecuter.h"

cPartyUnionManager* cPartyUnionManager::mpSingleton = 0;

cPartyUnionManager::cPartyUnionManager()
: mIndex(0)
, mLeaderIndex(0)
, mpLinkedIcon(0)
, mTempSlot(UINT_MAX)
, mGroup( eUNION_MAX )
, mHPAccumTime(0)
, mPosAccumTime(0)
, mPvpLeaderIndex(0)
, mFlagPlayerIndex(0)
{
	mpSingleton = this;
	::memset( &mParty, 0, sizeof(mParty) );
}

cPartyUnionManager::~cPartyUnionManager()
{
	mpSingleton = 0;
}

void cPartyUnionManager::Close()
{
	Clear();
}

void cPartyUnionManager::Clear()
{
	mIndex = 0;
	mLeaderIndex = 0;
	mTempSlot = UINT_MAX;

	for( unsigned int i = 0; i < MAX_PARTYUNION; ++i )
	{
		DeletePlayer( i );
	}

	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin )
		partyWin->Hide();

	for( int i = 0; i < MAX_PARTY; ++i )
	{
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

	//
	mGroup = eUNION_MAX;

	mHPAccumTime = 0;
	mPosAccumTime = 0;

	mPvpLeaderIndex = 0;
	mFlagPlayerIndex = 0;

	if( HERO )
		HERO->SetPartyUnionIndex( 0 );
}

void cPartyUnionManager::Process( unsigned long accumTime )
{
	if( mIndex == 0 )
		return;

	/// 일정 갱신시간
	if( accumTime - mHPAccumTime > PARTYUNION_DATAUPDATE )
	{
		/// HP 갱신
		for( int i = 0; i < MAX_PARTYUNION; ++i )
		{
			if( mParty[i].userIndex == 0 )
				continue;

			cPlayer* user = OBJECTMAN->GetPlayer( mParty[i].userIndex );
			if( user )
			{
				// 정보 갱신 및 연합 ui세팅
				UpdatePlayer( i, mParty[i].userIndex, user->GetLevel(), user->GetMaxHP(), user->GetMaxMP(), 
								 user->GetHP(), user->GetMP(), user->GetXPos(), user->GetYPos() );

				// 정보 갱신 및 파티 ui세팅
				if( IsHeroGroup( i )  == true )
				{
					if( UpdateGroup( i ) == false )
						assert(0);
				}
			}
		}

		mHPAccumTime = accumTime;
	}

	// 미니맵 세팅
	if( accumTime - mPosAccumTime > PARTYUNION_POSUPDATE )
	{
		UpdateMinimap();

		mPosAccumTime = accumTime;
	}
}

void cPartyUnionManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	cStringT str;
	switch( tempMsg->Protocol )
	{
	case NM_PARTYUNION_LIST_SYN:
		{
			// 연합 리스트
			MSG_SYN_PARTYUNION_LIST* pmsg = (MSG_SYN_PARTYUNION_LIST*)msg;
			
			// 
			Clear();

			// 정보 세팅
			mIndex			= pmsg->mIndex;
			mLeaderIndex	= pmsg->mLeaderIndex;

			// 유저 정보 세팅
			sUnionUserData* p = pmsg->mTable;
			for( unsigned char i = 0; i < pmsg->RowCount; ++i, ++p )
			{
				if( AddPlayer( p->idx, p ) == false )
				{
					assert(0);
					continue;
				}
			}

			UpdateLeader();

			// Hero 가 속한 그룹 정보 세팅
			int i = FindUser( HERO->GetObjectID() );
			ChangeHeroGroup( i );
			
			//
			SetGroupList();

			// hero에 파티연합정보 세팅
			HERO->SetPartyUnionIndex( mIndex );

			if( HERO->IsOpenNpcTalk() == false && GAMEUI->IsDuelMode() == false )
			{
				ShowLargeUnionUI( true );
			}
		}
		break;
	case NM_PARTYUNION_ADDONE_SYN:
		{
			// 1명 추가
			MSG_SYN_PARTYUNION_ADDONE* pmsg = (MSG_SYN_PARTYUNION_ADDONE*)msg;

			if( AddPlayer( pmsg->mTable.idx, &pmsg->mTable ) == false )
			{
				assert(0);
				return;
			}

			// 해당 유저가 HERO 그룹이면
			if( IsHeroGroup( pmsg->mTable.idx ) == true )
			{
				SetGroupList();
			}

			// 
			str.Format( GAMERESOURCEMAN->GetGameText(1400), pmsg->mTable.name );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		}
		break;
	case NM_PARTYUNION_OUT_RES:
		{
			// 연합 탈퇴
			MSG_RES_PARTYUNION_OUT* pmsg = (MSG_RES_PARTYUNION_OUT*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTYUNION_OUT_SUCESS:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1401) );

					// 파티연합정보 삭제
					Clear();

					// 
					HERO->SetPartyUnionIndex( 0 );

					cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
					if( win )
						win->Hide();

					cPartyUnionSmallWindow* smallWin = GAMEUI->GetPartyUnionSmallWindow();
					if( smallWin )
						smallWin->Hide();
				}
				break;
			case ERROR_PARTYUNION_OUT_FAIL:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1402) );
				}
				break;
			}
		}
		break;
	case NM_PARTYUNION_OUT_SYN:
		{
			// 다른 유저 연합 탈퇴
			MSG_SYN_PARTYUNION_OUT* pmsg = (MSG_SYN_PARTYUNION_OUT*)msg;

			int i = FindUser( pmsg->mPlayerIndex );
			if( i >= 0 )
			{
				str.Format( GAMERESOURCEMAN->GetGameText(1403), mParty[i].name );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

				//
				DeletePlayer( i );

				// 해당 유저가 어느그룹인지 판단후 맞으면
				if( IsHeroGroup( i ) == true )
				{
					SetGroupList();
				}
			}
			else
				assert(0);
		}
		break;
	case NM_PARTYUNION_CHANGELEADER_RES:
		{
			// 연합장 위임
			MSG_RES_PARTYUNION_CHANGELEADER* pmsg = (MSG_RES_PARTYUNION_CHANGELEADER*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTYUNION_CHANGELEADER_SUCESS:
				break;
			case ERROR_PARTYUNION_CHANGELEADER_LEADER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1404) );
				break;
			case ERROR_PARTYUNION_CHANGELEADER_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1405) );
				break;
			}

			cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
			if( win )
				win->NetUnLock();
		}
		break;
	case NM_PARTYUNION_CHANGELEADER_SYN:
		{
			MSG_SYN_PARTYUNION_CHANGELEADER* pmsg = (MSG_SYN_PARTYUNION_CHANGELEADER*)msg;

			int i = FindUser( pmsg->mPlayerIndex );
			if( i >= 0 )
			{
				str.Format( GAMERESOURCEMAN->GetGameText(1406), mParty[i].name );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

				//
				mLeaderIndex = pmsg->mPlayerIndex;

				//	
				UpdateLeader();
			}
			else
				assert(0);
		}
		break;
	case NM_PARTYUNION_MOVEGROUP_RES:
		{
			// 그룹 이동
			MSG_RES_PARTYUNION_MOVEGROUP* pmsg = (MSG_RES_PARTYUNION_MOVEGROUP*)msg;
			
			switch( pmsg->ErrorCode )
			{
			case ERROR_PARTYUNION_MOVEGROUP_SUCESS:
				break;
			case ERROR_PARTYUNION_MOVEGROUP_LEADER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1407) );
				break;
			case ERROR_PARTYUNION_MOVEGROUP_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1408) );
				break;
			}

			cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
			if( win )
				win->NetUnLock();
		}
		break;
	case NM_PARTYUNION_MOVEGROUP_SYN:
		{
			MSG_SYN_PARTYUNION_MOVEGROUP* pmsg = (MSG_SYN_PARTYUNION_MOVEGROUP*)msg;

			if( ChangePlayer( pmsg->mIndex0, pmsg->mIndex1 ) == true )
			{
				UpdateLeader();

				// hero 가 이동되었으면 그룹정보 갱신
				if( mParty[pmsg->mIndex0].userIndex == HERO->GetObjectID() )
				{
					ChangeHeroGroup( pmsg->mIndex0 );
				}
				else if( mParty[pmsg->mIndex1].userIndex == HERO->GetObjectID() )
				{
					ChangeHeroGroup( pmsg->mIndex1 );
				}

				// hero 그룹이면 갱신
				if( IsHeroGroup( pmsg->mIndex0 ) == true || IsHeroGroup( pmsg->mIndex1 ) == true )
				{
					SetGroupList();
				}
			}
		}
		break;
	case NM_PARTYUNION_UPDATE_SYN:
		{
			// 정보 갱신
			MSG_SYN_PARTYUNION_UPDATE* pmsg = (MSG_SYN_PARTYUNION_UPDATE*)msg;

			sUnionUpdate* data = pmsg->table;
			for( unsigned char i = 0; i < pmsg->rowCount; ++i, ++data )
			{
				UpdatePlayer( data->idx, data->userIndex, data->maxhp, data->hp, data->posX, data->posY );
			}
		}
		break;
	case NM_PARTYUNION_GROUPUPDATE_SYN:
		{
			// 동일 그룹 정보 갱신
			MSG_SYN_PARTYUNION_GROUPUPDATE* pmsg = (MSG_SYN_PARTYUNION_GROUPUPDATE*)msg;

			// 정보 갱신
			UpdatePlayer( pmsg->idx, pmsg->userIndex, pmsg->level, pmsg->maxhp, pmsg->maxmp, pmsg->hp, pmsg->mp, pmsg->posX, pmsg->posY );

			// 파티 UI 갱신
			if( IsHeroGroup( pmsg->idx )  == true )
			{
				if( UpdateGroup( pmsg->idx ) == false )
				{
					assert(0);
					return;
				}

				// 버프 갱신
				if( UpdateGroupBuff( pmsg->idx, pmsg->table, pmsg->rowCount ) == false )
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
	default:
		assert(0);
		break;
	}
}

void cPartyUnionManager::LinkIcon( cUnionIcon* icon )
{
	if( icon )
	{
		if( mpLinkedIcon == icon )
			return;

		if( mpLinkedIcon )
		{
			mpLinkedIcon->SetColor( NiColorA::WHITE );
			assert(0);
		}

		NiColorA check;
		mpLinkedIcon = icon;
		mpLinkedIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
	}
	else
		assert(0);
}

void cPartyUnionManager::UnLinkIcon()
{
	if( mpLinkedIcon )
	{
		mpLinkedIcon->SetColor( NiColorA::WHITE );
		mpLinkedIcon = 0;
	}
}

bool cPartyUnionManager::AddPlayer( unsigned int i, sUnionUserData* data )
{
	// 맵 넘버 갱신 패킷 만들기 전까지는 우선 hero 꺼로 세팅하자 09.06.19
	data->mapNum = HERO->GetMapNum();

	/// data
	mParty[i] = *data;

	/// ui setting
	cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
	cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
	if( minimap && pWide )
	{
		minimap->AddUnion( data );
		pWide->AddUnion( data );
	}

	cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
	if( win )
	{
		win->SetInfo( i, data->name, data->maxhp, data->hp, data->job, data->level );
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

bool cPartyUnionManager::DeletePlayer( unsigned int i )
{
	/// data setting
	sUnionUserData* data = &mParty[i];

	//
	cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
	cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
	if( minimap && pWide )
	{
		minimap->DeleteUnion( data->userIndex );
		pWide->DeleteUnion( data->userIndex );
	}

	::memset( data, 0, sizeof(sUnionUserData) );
	
	/// ui setting
	cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
	if( win )
	{
		win->ClearInfo( i );
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

bool cPartyUnionManager::ChangePlayer( unsigned int i0, unsigned int i1 )
{
	if( i0 < MAX_PARTYUNION && i1 < MAX_PARTYUNION )
	{
		// data
		sUnionUserData p0 = mParty[i0];
		sUnionUserData p1 = mParty[i1];
		sUnionUserData temp;
		::memset( &temp, 0, sizeof(temp) );

		if( p0.userIndex == 0 )
			assert(0);

		temp = p0;
		p0 = p1;
		p1 = temp;

		mParty[i0] = p0;
		mParty[i1] = p1;

		//
		cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
		cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
		if( minimap && pWide )
		{
			// 삭제 & 삽입
			if( mParty[i0].userIndex > 0 )
			{
				minimap->DeleteUnion( mParty[i0].userIndex );
				pWide->DeleteUnion( mParty[i0].userIndex );

				minimap->AddUnion( &mParty[i0] );
				pWide->AddUnion( &mParty[i0] );

				/// 수장 표시 갱신
				if( mParty[i0].userIndex == mPvpLeaderIndex )
				{
					if( minimap )
						minimap->SetPvpLeader( mPvpLeaderIndex );
					if( pWide )
						pWide->SetPvpLeader( mPvpLeaderIndex );
				}
			}

			if( mParty[i1].userIndex > 0 )
			{
				minimap->DeleteUnion( mParty[i1].userIndex );
				pWide->DeleteUnion( mParty[i1].userIndex );

				minimap->AddUnion( &mParty[i1] );
				pWide->AddUnion( &mParty[i1] );

				/// 수장 표시 갱신
				if( mParty[i1].userIndex == mPvpLeaderIndex )
				{
					if( minimap )
						minimap->SetPvpLeader( mPvpLeaderIndex );
					if( pWide )
						pWide->SetPvpLeader( mPvpLeaderIndex );
				}
			}
		}

		// ui
		cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
		if( win )
		{
			win->ClearInfo( i0 );
			win->ClearInfo( i1 );

			if( mParty[i0].userIndex > 0 )
			{
				win->SetInfo( i0, mParty[i0].name, mParty[i0].maxhp, mParty[i0].hp, mParty[i0].job, mParty[i0].level );
				/// 수장, 깃발 가진 유저 표시
				if( mParty[i0].userIndex == mPvpLeaderIndex )
					win->SetPvpLeader( i0 );
				else if( mParty[i0].userIndex == mFlagPlayerIndex )
					win->SetFlagPlayer( i0 );
			}

			if( mParty[i1].userIndex > 0 )
			{
				win->SetInfo( i1, mParty[i1].name, mParty[i1].maxhp, mParty[i1].hp, mParty[i1].job, mParty[i1].level );
				/// 수장, 깃발 가진 유저 표시
				if( mParty[i1].userIndex == mPvpLeaderIndex )
					win->SetPvpLeader( i1 );
				else if( mParty[i1].userIndex == mFlagPlayerIndex )
					win->SetFlagPlayer( i1 );

			}

			win->SetSelect( mTempSlot );
			mTempSlot = UINT_MAX;
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

void cPartyUnionManager::UpdatePlayer( unsigned int i, unsigned long userIndex, unsigned char level, unsigned int maxhp, unsigned int maxmp, 
									   unsigned int hp, unsigned int mp, float posX, float posY )
{
	if( i >= 0 && i < MAX_PARTYUNION )
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

			// ui 세팅
			cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
			if( win )
			{
				win->UpdateInfo( i, mParty[i].maxhp, mParty[i].hp );
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

void cPartyUnionManager::UpdatePlayer( unsigned int i, unsigned long userIndex, unsigned int maxhp, unsigned int hp, float posX, float posY )
{
	if( i >= 0 && i < MAX_PARTYUNION )
	{
		if( mParty[i].userIndex == userIndex )
		{
			// 정보 갱신
			mParty[i].maxhp = maxhp;
			mParty[i].hp	= hp;
			mParty[i].posX  = posX;
			mParty[i].posY  = posY;

			// ui 세팅
			cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
			if( win )
			{
				win->UpdateInfo( i, mParty[i].maxhp, mParty[i].hp );
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


void cPartyUnionManager::UpdateLeader()
{
	cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
	if( win )
	{
		int i = FindUser( mLeaderIndex );
		if( i >= 0 )
		{
			win->SetLeader( i );
		}

		bool enable = ( HERO->GetObjectID() == mLeaderIndex );
		win->EnableLeader( enable );
	}
	else
		assert(0);
}

/// party ui에 파티 연합 정보 갱신
bool cPartyUnionManager::SetGroupList()
{
	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin )
		partyWin->Hide();

	// 초기화
	for( int i = 0; i < MAX_PARTYUNION; ++i )
	{
		cPlayer* player = OBJECTMAN->GetPlayer( mParty[i].userIndex );
		if( player )
			player->PartyOut_Init();
	}

	// 초기화
	for( int i = 0; i < MAX_PARTY; ++i )
	{
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

	int cnt = 0;
	for( int i = 0; i < MAX_PARTYUSER; ++i )
	{
		// 데이터 세팅
		int idx = i + ( MAX_PARTYUSER * mGroup );
		sUnionUserData* p = &mParty[idx];

		// hero 정보는 세팅하지 않는다
		if( p->userIndex == HERO->GetObjectID() )
			continue;

		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( cnt );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( cnt );
		if( state && statemini )
		{
			// 초기화
			state->Clear();
			statemini->Clear();

			// 정보 세팅
			if( p->userIndex > 0 )
			{
				state->SetInfo( p->userIndex, p->name, p->level, (ePLAYER_JOB)p->job, p->maxhp, p->hp, p->maxmp, p->mp, p->mapNum, 0, p->channelNum, cnt );
				statemini->SetInfo( p->userIndex, p->name, p->maxhp, p->hp, p->maxmp, p->mp, p->mapNum, 0, p->channelNum, cnt );

				ShowStateUI( cnt );

				cPlayer* player = OBJECTMAN->GetPlayer( p->userIndex );
				if( player )
					player->PartyIn_Init();

				cnt++;
			}
			else
			{
				HideStateUI( cnt );
			}
		}

		// 버프 세팅
		cPlayer* user = OBJECTMAN->GetPlayer( mParty[i].userIndex );
		if( user )
		{
			UpdateGroupBuff( i, user );

			UpdateMouseHover();
		}
	}
	return true;
}

// 한사람 갱신 - 파티창 갱신
bool cPartyUnionManager::UpdateGroup( unsigned int i )
{
	sUnionUserData* p = &mParty[i];

	//
	if( p->userIndex == HERO->GetObjectID() )
		return true;

	for( int idx = 0; idx < MAX_PARTY; ++idx )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( idx );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( idx );
		if( state && statemini )
		{
			if( state->GetUserIndex() == p->userIndex && statemini->GetUserIndex() == p->userIndex )
			{
				state->UpdateInfo( p->level, p->maxhp, p->hp, p->maxmp, p->mp );
				statemini->UpdateInfo( p->maxhp, p->hp, p->maxmp, p->mp );
				return true;
			}
		}
	}
	return false;
}

bool cPartyUnionManager::UpdateGroupBuff( unsigned int i, sInfluenceClient* data, long count )
{
	sUnionUserData* p = &mParty[i];

	if( p->userIndex == HERO->GetObjectID() )
		return true;

	for( int idx = 0; idx < MAX_PARTY; ++idx )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( idx );
		if( state && state->GetUserIndex() == p->userIndex )
		{
			// 세팅	
			state->ClearBuff();
			for( long i = 0; i < count; ++i, ++data )
			{
				state->AddBuff( data->mUniqueIdx, data->mInfluenceClassIdx );
			}
			return true;
		}
	}
	return false;
}

void cPartyUnionManager::ClearAllBuff( unsigned long objectIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 && i < MAX_PARTYUNION )
	{
		for( int idx = 0; idx < MAX_PARTY; ++idx )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( idx );
			if( state && state->GetUserIndex() == objectIdx )
			{
				state->ClearBuff();
				return;
			}
		}
	}
	else
		assert(0);
}

void cPartyUnionManager::AddBuff( unsigned long objectIdx, unsigned long uniqueIdx, unsigned long influenceIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 && i < MAX_PARTYUNION )
	{
		for( int idx = 0; idx < MAX_PARTY; ++idx )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( idx );
			if( state && state->GetUserIndex() == objectIdx )
			{
				state->AddBuff( uniqueIdx, influenceIdx );
				UpdateMouseHover();
				return;
			}
		}
	}
}

void cPartyUnionManager::DeleteBuff( unsigned long objectIdx, unsigned long uniqueIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 && i < MAX_PARTYUNION )
	{
		for( int idx = 0; idx < MAX_PARTY; ++idx )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( idx );
			if( state && state->GetUserIndex() == objectIdx )
			{
				state->DelBuff( uniqueIdx );
				UpdateMouseHover();
				return;
			}
		}
	}
}

void cPartyUnionManager::DeleteDeBuff( unsigned long objectIdx, unsigned long uniqueIdx )
{
	int i = FindUser( objectIdx );
	if( i >= 0 && i < MAX_PARTYUNION )
	{
		for( int idx = 0; idx < MAX_PARTY; ++idx )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( idx );
			if( state && state->GetUserIndex() == objectIdx )
			{
				state->DelDeBuff( uniqueIdx );
				UpdateMouseHover();
				return;
			}
		}
	}
}

bool cPartyUnionManager::UpdateGroupBuff( unsigned int i, cPlayer* player )
{
	typedef tPointerHashMap<unsigned long, void*> cInfluenceMap;

	if( player != NULL )
	{
		for( int idx = 0; idx < MAX_PARTY; ++idx )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( idx );
			if( state && state->GetUserIndex() == player->GetObjectID() )
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
				return true;
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

void cPartyUnionManager::UpdateMouseHover()
{
	for( int i = 0; i < MAX_PARTY; ++i )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
		if( state )
			state->UpdateMouseHover( cUIPos( MOUSE->GetMouseX(), MOUSE->GetMouseY() ));
	}
}

void cPartyUnionManager::UpdateMinimap()
{
	cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
	cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();
	if( minimap && pWide )
	{
		sUnionUserData* p = mParty;
		for( int i = 0; i < MAX_PARTYUNION; ++i, ++p )
		{
			if( p->userIndex > 0 )
			{
				minimap->UpdateUnion( p->userIndex, p->posX, p->posY );
				pWide->UpdateUnion( p->userIndex, p->posX, p->posY );
			}
		}
	}
}

void cPartyUnionManager::ShowUI()
{
	if( HERO->GetPartyUnionIndex() == 0 )
		return;

	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin )
		partyWin->Hide();

	for( int i = 0; i < MAX_PARTY; ++i )
	{
		ShowStateUI( i );
	}

	ShowLargeUnionUI( true );
}

void cPartyUnionManager::HideUI()
{
	for( int i = 0; i < MAX_PARTY; ++i )
	{
		HideStateUI( i );
	}

	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin )
		partyWin->Hide();

	cPartyUnionWindow* unionWin = GAMEUI->GetPartyUnionWindow();
	if( unionWin )
		unionWin->Hide();

	cPartyUnionSmallWindow* smallWin = GAMEUI->GetPartyUnionSmallWindow();
	if( smallWin )
		smallWin->Hide();
}

// display 모드에 따른 윈도우 보기 설정
void cPartyUnionManager::ShowStateUI( int i )
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
					bool isPvpLeader = (state->GetUserIndex() == mPvpLeaderIndex ) ;
					state->SetPvpLeaderVisible( isPvpLeader );
					
					bool isFlagPlayer = (state->GetUserIndex() == mFlagPlayerIndex ) ;
					state->SetFlagPlayerVisible( isFlagPlayer );

					show = true;
					state->Show();
				}
			}
			break;
		case ePARTY_STATE_DISPLAY_MINI:
			{
				cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
				if( statemini && statemini->GetUserIndex() > 0 )
				{
					bool isPvpLeader = (statemini->GetUserIndex() == mPvpLeaderIndex) ;
					statemini->SetPvpLeaderVisible( isPvpLeader );

					bool isFlagPlayer = (statemini->GetUserIndex() == mFlagPlayerIndex ) ;
					statemini->SetFlagPlayerVisible( isFlagPlayer );

					show = true;
					statemini->Show();
				}
			}
			break;
		case ePARTY_STATE_DISPLAY_HIDE:
			{
				show = true;
			}
			break;
		}

		if( show == true )
		{
			partyWin->SetUnion();
			partyWin->Show();
		}
	}
}

void cPartyUnionManager::HideStateUI( int i )
{
	cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( i );
	if( state )
		state->Hide();

	cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( i );
	if( statemini )
		statemini->Hide();
}
//
void cPartyUnionManager::ChangeDisplayType( unsigned char type )
{
	for( int i = 0; i < MAX_PARTY; ++i )
	{
		HideStateUI( i );
	}

	// hide 모드가 아니면 다시 보여주기
	if( type != ePARTY_STATE_DISPLAY_HIDE )
	{
		for( int i = 0; i < MAX_PARTY; ++i )
		{
			ShowStateUI( i );
		}
	}
}

// 창 같이 열리지 않도록 조정
void cPartyUnionManager::ShowLargeUnionUI( bool show )
{
	if( show == true )
	{
		cPartyUnionWindow* unionWin = GAMEUI->GetPartyUnionWindow();
		if( unionWin )
			unionWin->Show();

		cPartyUnionSmallWindow* smallWin = GAMEUI->GetPartyUnionSmallWindow();
		if( smallWin )
			smallWin->Hide();
	}
	else
	{
		cPartyUnionWindow* unionWin = GAMEUI->GetPartyUnionWindow();
		if( unionWin )
			unionWin->Hide();

		cPartyUnionSmallWindow* smallWin = GAMEUI->GetPartyUnionSmallWindow();
		if( smallWin )
			smallWin->Show();
	}
}

// request
bool cPartyUnionManager::TryToMovePlayer( unsigned char i0, unsigned char i1 )
{
	cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return true;

	if( i0 >= MAX_PARTYUNION || i1 >= MAX_PARTYUNION )
	{
		assert(0);
		return false;
	}

	if( mParty[i0].userIndex == 0 )
	{
		assert(0);
		return false;
	}

	if( HERO->GetObjectID() != mLeaderIndex )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1407) );
		return false;
	}

	MSG_REQ_PARTYUNION_MOVEGROUP msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_PARTYUNION;
	msg.Protocol = NM_PARTYUNION_MOVEGROUP_REQ;
	msg.mIndex0 = i0;
	msg.mIndex1 = i1;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	mTempSlot = i1;

	//
	win->NetLock();
	return true;
}

bool cPartyUnionManager::TryToChangeLeader( unsigned long playerIndex )
{
	cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return true;

	if( playerIndex == 0 )
		return true;

	if( HERO->GetObjectID() != mLeaderIndex )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(1404) );
		return false;
	}

	MSG_REQ_PARTYUNION_CHANGELEADER msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_PARTYUNION;
	msg.Protocol = NM_PARTYUNION_CHANGELEADER_REQ;
	msg.mPlayerIndex = playerIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	win->NetLock();

	return true;
}


void cPartyUnionManager::ChangeHeroGroup( int i )
{
	if( i >= 0 && i < MAX_PARTYUSER )
	{
		mGroup = eUNION_GROUP1;
	}
	else if( i >= MAX_PARTYUSER && i < MAX_PARTYUSER * eUNION_GROUP3 )
	{
		mGroup = eUNION_GROUP2;
	}
	else if( i >= MAX_PARTYUSER * eUNION_GROUP3 && i < MAX_PARTYUSER * eUNION_MAX )
	{
		mGroup = eUNION_GROUP3;
	}
	else
		assert(0);
}

int cPartyUnionManager::GetGroup( unsigned long userIndex )
{
	int i = FindUser( userIndex );
	if( i >= 0 )
	{
		if( i < MAX_PARTYUSER )
		{
			return eUNION_GROUP1;
		}
		else if( i >= MAX_PARTYUSER && i < MAX_PARTYUSER * eUNION_GROUP3 )
		{
			return eUNION_GROUP2;
		}
		else if( i >= MAX_PARTYUSER * eUNION_GROUP3 && i < MAX_PARTYUSER * eUNION_MAX )
		{
			return eUNION_GROUP3;
		}
	}
	return -1;
}

// i 번째 유저가 hero 그룹인지 판단
bool cPartyUnionManager::IsHeroGroup( int i )
{
	switch( mGroup )
	{
	case eUNION_GROUP1:
		if( i >= 0 && i < MAX_PARTYUSER )
			return true;
		break;
	case eUNION_GROUP2:
		if( i >= MAX_PARTYUSER && i < MAX_PARTYUSER * eUNION_GROUP3 )
			return true;
		break;
	case eUNION_GROUP3:
		if( i >= MAX_PARTYUSER * eUNION_GROUP3 && i < MAX_PARTYUSER * eUNION_MAX )
			return true;
		break;
	default:
		assert(0);
		break;
	}
	return false;
}

int cPartyUnionManager::FindUser( unsigned long playerIndex )
{
	if( playerIndex == 0 )
		return -1;

	for( int i = 0; i < MAX_PARTYUNION; ++i )
	{
		if( mParty[i].userIndex == playerIndex )
			return i;
	}
	return -1;
}

bool cPartyUnionManager::IsHeroLeader()
{
	return (HERO->GetObjectID() == mLeaderIndex);
}

bool cPartyUnionManager::IsHeroParty( unsigned long userIndex )
{
	int i = FindUser( userIndex );
	if( i > -1 )
	{
		return IsHeroGroup( i );
	}
	return false;
}

void cPartyUnionManager::SetTargetByUI( unsigned long userIndex )
{
	if( HERO->GetPartyUnionIndex() == 0 )
		return;

	int i = FindUser( userIndex );
	if( i >= 0 && i < MAX_PARTYUNION )
	{
		if( SKILLEXECUTER->IsSkillCharge() == true )
		{
			cBaseObject* curTarget = HERO->GetTargetObject();
			if( curTarget == 0 || curTarget->GetState() == eOBJECT_STATE_DIE )
			{
				/// 신규 타겟 등록후 발동
				HERO->SetTargetObject( eOBJECTTYPE_PLAYER, userIndex );
			}
			else
			{
				/// 타겟 변경 없이 발동
				cBaseObject* target = OBJECTMAN->GetObject( eOBJECTTYPE_PLAYER, userIndex );

				sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();

				SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, target );
				return;
			}
		}
		else
		{
			HERO->SetTargetObject( eOBJECTTYPE_PLAYER, userIndex );
		}

		// 파티연합 윈도우 세팅
		cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
		if( win )
			win->SetSelect( i );

		// 파티 윈도우세팅
		for( unsigned int w = 0; w < MAX_PARTY; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				bool select = ( state->GetUserIndex() == userIndex );
				state->SetSelect( select );

				bool selectmini = ( statemini->GetUserIndex() == userIndex );
				statemini->SetSelect( selectmini );
			}
		}

//		HERO->SetTargetObject( eOBJECTTYPE_PLAYER, userIndex );
	}
	else
		assert(0);
}

void cPartyUnionManager::SetTargetByHero( unsigned long userIndex )
{
	if( HERO->GetPartyUnionIndex() == 0 )
		return;

	if( HERO->GetObjectID() == userIndex )
		return;

	int i = FindUser( userIndex );
	if( i >= 0 && i < MAX_PARTYUNION )
	{
		// 파티연합 윈도우 세팅
		cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
		if( win )
			win->SetSelect( i );

		// 파티 윈도우세팅
		for( unsigned int w = 0; w < MAX_PARTY; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				bool select = ( state->GetUserIndex() == userIndex );
				state->SetSelect( select );

				bool selectmini = ( statemini->GetUserIndex() == userIndex );
				statemini->SetSelect( selectmini );
			}
		}
	}
}

void cPartyUnionManager::ClearTarget()
{
	if( HERO->GetPartyUnionIndex() == 0 )
		return;

	cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
	if( win )
		win->SetSelect( UINT_MAX );

}

/// 전장에서 수장 표시.
void cPartyUnionManager::SetPvpLeader( unsigned long characterIndex )
{

	int arrIndex = FindUser( characterIndex );
	if( arrIndex >= 0 && arrIndex < MAX_PARTYUNION )
	{
		mPvpLeaderIndex = characterIndex;

		cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
		if( win )
			win->SetPvpLeader( arrIndex );

		// 파티 윈도우세팅
		for( unsigned int w = 0; w < MAX_PARTY; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				bool select = ( state->GetUserIndex() == characterIndex );
				state->SetPvpLeaderVisible( select );

				bool selectmini = ( statemini->GetUserIndex() == characterIndex );
				statemini->SetPvpLeaderVisible( selectmini );
			}
		}
	}
	else
	{
		assert(0 && "Bad character index");

		cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
		if( win )
			win->SetPvpLeader( UINT_MAX );

		// 파티 윈도우세팅
		for( unsigned int w = 0; w < MAX_PARTY; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				state->SetPvpLeaderVisible( false );
				statemini->SetPvpLeaderVisible( false );
			}
		}
	}
}

/// 플래그 매치에서 깃발 든 유저 표시
void cPartyUnionManager::SetFlagPlayer( unsigned long characterIndex )
{

	int arrIndex = FindUser( characterIndex );
	if( arrIndex >= 0 && arrIndex < MAX_PARTYUNION )
	{
		mFlagPlayerIndex = characterIndex;

		cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
		if( win )
			win->SetFlagPlayer( arrIndex );

		// 파티 윈도우세팅
		for( unsigned int w = 0; w < MAX_PARTY; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				bool select = ( state->GetUserIndex() == characterIndex );
				state->SetFlagPlayerVisible( select );

				bool selectmini = ( statemini->GetUserIndex() == characterIndex );
				statemini->SetFlagPlayerVisible( selectmini );
			}
		}
	}
	else
	{
		mFlagPlayerIndex = 0;

		cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
		if( win )
			win->SetFlagPlayer( UINT_MAX );

		// 파티 윈도우세팅
		for( unsigned int w = 0; w < MAX_PARTY; ++w )
		{
			cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
			cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
			if( state && statemini )
			{
				state->SetFlagPlayerVisible( false );
				statemini->SetFlagPlayerVisible( false );
			}
		}
	}
}

void cPartyUnionManager::ClearFlagPlayer()
{
	mFlagPlayerIndex = 0;

	cPartyUnionWindow* win = GAMEUI->GetPartyUnionWindow();
	if( win )
		win->SetFlagPlayer( UINT_MAX );

	// 파티 윈도우세팅
	for( unsigned int w = 0; w < MAX_PARTY; ++w )
	{
		cPartyStateWindow* state = GAMEUI->GetPartyStateWindow( w );
		cPartyStateMiniWindow* statemini = GAMEUI->GetPartyStateMiniWindow( w );
		if( state && statemini )
		{
			state->SetFlagPlayerVisible( false );
			statemini->SetFlagPlayerVisible( false );
		}
	}
}

LPCTSTR cPartyUnionManager::GetPlayerName( unsigned long characterIndex )
{
	if( characterIndex == 0 )
		return NULL;

	for( int i = 0; i < MAX_PARTYUNION; ++i )
	{
		if( mParty[i].userIndex == characterIndex )
			return mParty[i].name;
	}

	return NULL;
}
