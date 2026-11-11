#include "stdafx.h"
#include "gamesrv.h"
#include "ThemeManager.h"
#include "Party.h"
#include "PartyManager.h"
#include "ObjectManager.h"
#include "CommonNetworkMsgStruct.h"
#include "StageScript.h"
#include "GridManager.h"
#include "TutorialObject.h"
#include "TutorialScript.h"
#include "SkillManager.h"

cThemeManager* cThemeManager::mpThemeManager = NULL;

cThemeManager::cThemeManager()
{
	if(mpThemeManager)
	{
		MessageBox(NULL,"cSkillManager : 이 클래스는 객체가 유일한객체(singleton)이어야 합니다.\
			이전 객체를 사용하던지 이전 객체를 지우고 생성하십시오.","경고",MB_OK);
	}
	else
	{
		mpThemeManager = this;
	}
}


cThemeManager::~cThemeManager()
{
	Release();
}


bool cThemeManager::Init( SERVER_TYPE type )
{
	if( mThemeTimeScript.Init() == false )
		return false;

	/// memorypooh.h 이용하는걸 고려해보자!!
	if( type == _E_ST_ID_THEME_ || type == _E_ST_ID_TUTORIAL_ )
		mThemeObjectPool.Reserve(255,10);		
	else
		mThemeObjectPool.Reserve(1,1);

	mType = type;

	mNextEventTime = 0;
	mNextResetTime = 0;
	mThemeOpenSendTime = 0;

	mStartPosIdx = CHARACTER_CREATE_POS_IDX;
	switch( NETWORK2->GetInDunMapNum() )
	{
	case MAP_CIRCUS:	mStartPosIdx = CIRCUS_START_POS_IDX;	break;
	case MAP_RIOHA:		mStartPosIdx = RIOHA_START_POS_IDX;		break;
	case MAP_DECOMUSS:	mStartPosIdx = DECOMUSS_START_POS_IDX;	break;
	case MAP_TUTORIAL:	mStartPosIdx = TUTORIAL_POS_IDX;		break;
	}

	/// 시작 설정
	if( NETWORK2->GetInDunMapNum() == MAP_TUTORIAL )
	{
		mProcess = eTHEMEPROCESS_RUN;
		NETWORK2->PostThemeStatus( _E_THEME_RUNNING_ );
		mNextEventTime = ULONG_MAX;
	}
	else
	{
		mProcess = eTHEMEPROCESS_READY;
		NETWORK2->PostThemeStatus( _E_THEME_READY_ );
	}

	return true;
}


void cThemeManager::Release()
{
	NETWORK2->PostThemeStatus( _E_THEME_CLOSED_ );

	cPHashMap::cIterator b = mThemeMap.Begin();
	cPHashMap::cIterator e = mThemeMap.End();

	for( ; b != e ; ++b )
	{
		cThemeObject* pObject = (cThemeObject*)(*b).mSecond;
		SAFE_DELETE( pObject );	
	}
	mThemeMap.Clear();

	mThemeTimeScript.Release();
}


void cThemeManager::Process( unsigned long elapsedTime, unsigned long accumTime )
{
	/// 서커스 인던 개별 객체 프로세스
	cPHashMap::cIterator b = mThemeMap.Begin();
	cPHashMap::cIterator e = mThemeMap.End();
	for( ; b != e ; ++b )
	{
		cThemeObject* pObject = (cThemeObject*)(*b).mSecond;
		if( pObject == NULL )
			continue;

		pObject->ProcessTheme( elapsedTime, accumTime );
	}

	for( unsigned long i = 0 ; i < mDelAry.GetSize() ; ++i )
	{
		unsigned long themeIdx = mDelAry[i];
		cThemeObject* ptheme = (cThemeObject*)mThemeMap.GetAt( themeIdx );
		if( ptheme != NULL )
		{
			SAFE_DELETE( ptheme );
		}

		mThemeMap.Erase( themeIdx );
	}
	mDelAry.Clear();

	/// 상태별 프로세스
	switch( mProcess )
	{
	case eTHEMEPROCESS_RUN:
		{
			/// 서버가 켜져있는것을 주기적으로 알려줌
			if( mThemeOpenSendTime < accumTime )
			{
				mThemeOpenSendTime = accumTime + THEME_OPEN_SEND_TIME;
				NETWORK2->PostThemeStatus( _E_THEME_RUNNING_ );
			}

			if( mNextEventTime > accumTime )
				return;

			/// 다음 이벤트 발동시간 설정
			mNextEventTime = mNextEventTime + THEME_READY_TIME;
			mProcess = eTHEMEPROCESS_READY;
			NETWORK2->PostThemeStatus( _E_THEME_READY_ );
			return;
		}
		break;
	case eTHEMEPROCESS_READY:
		{
			if( mNextEventTime > accumTime )
				return;

			///db에 요청
			mProcess = eTHEMEPROCESS_WAITDB;
			HANDLE        handle = NULL;
			THEME_CREATE* create = (THEME_CREATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_CREATE );

			// HACK - 시간과 룸 정의 해야됨.
			create->themeIdx = NETWORK2->GetInDunMapNum();
			create->maxRoom  = INDUN_MAP_MAX;
			sThemeTime* pTime = THEMETIMESCRIPT->GetThemeTime( NETWORK2->GetInDunMapNum() );
			if( pTime != NULL )
			{
				create->fixTime  = pTime->mResetClockHour;
				create->varTime  = pTime->mResetIntervalHour;
			}
			else
			{
				create->fixTime  = 12;
				create->varTime  = 24;
			}
			NETWORK2->SendSQL( handle, sizeof(THEME_CREATE) );
			return;
		}
		break;
	case eTHEMEPROCESS_5MIN:
		{
			if( mReadyPlusTime > accumTime )
				return;

			mReadyPlusTime = 0;

			mProcess = eTHEMEPROCESS_RUN;

			mThemeOpenSendTime = accumTime + THEME_OPEN_SEND_TIME;
			NETWORK2->PostThemeStatus( _E_THEME_RUNNING_ );
		}
		break;
	case eTHEMEPROCESS_WAITDB:
		return;
	}
}


void cThemeManager::ThemeRequest( unsigned long playerIdx, unsigned short themeMapNum )
{
	HANDLE handle  = NULL;

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;
	
	if( pPlayer->IsRequestRejection() == true || pPlayer->GetGameIn() == false || pPlayer->GetThemePartyDB() == true )
	{
		NETWORK2->SendMsgError( pPlayer->GetConnectionIdx(), NM_THEME, NM_THEME_READY_RES, ERROR_THEME_READY_STATE );
	}
	else
	{
		pPlayer->SetThemePartyDB( true );

		cParty* pParty = NULL;
		if( pPlayer->GetPartyIndex() != 0 )
			pParty = PARTYMAN->GetParty( pPlayer->GetPartyIndex() );

		if( pParty != NULL )
		{
			/// db에서 파티원들의 귀속정보를 로드한다.
			THEME_PARTYINFO_SELECT* pPartySelect = (THEME_PARTYINFO_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_PARTY_SELECT );

			pPartySelect->mThemeIdx = themeMapNum;
			pPartySelect->mPartyCnt = (unsigned char)pParty->GetCount();
			for( unsigned int i = 0 ; i < pParty->GetCount() ; ++i )
				pPartySelect->mCharacterIdx[i] = pParty->GetUserArr()[i];
			
			NETWORK2->SendSQL( pPlayer->GetConnectionIdx(), handle, pPartySelect->GetMsgLength() );
		}
		else
		{
			THEME_PARTYINFO_SELECT* pPartySelect = (THEME_PARTYINFO_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_PARTY_SELECT );

			pPartySelect->mThemeIdx = themeMapNum;
			pPartySelect->mPartyCnt = 1;
			pPartySelect->mCharacterIdx[0] = playerIdx;

			NETWORK2->SendSQL( pPlayer->GetConnectionIdx(), handle, pPartySelect->GetMsgLength() );
		}
	}
}


void cThemeManager::ThemeReadyAccept( unsigned long playerIdx, unsigned short themeMapNum, unsigned char themeMode )
{
	HANDLE handle  = NULL;

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;
	
	try
	{
		sMapInfo* pMapInfo = STAGESCRIPT->GetMapFolderIdx( themeMapNum, themeMode );
		if( pMapInfo == NULL )
			throw ERROR_THEME_READY_SCRIPT;

		sThemeData* pData = NETWORK2->GetThemeData( themeMapNum );
		if( pData == NULL )
			throw ERROR_THEME_READY_SERVER_READY;
					
		if( pData->status == _E_THEME_READY_ )
			throw ERROR_THEME_READY_SERVER_READY;
		else if( pData->status == _E_THEME_CLOSED_ )
			throw ERROR_THEME_READY_SERVER_NOTRUN;

		/// 레벨 제한 체크
		if( pPlayer->GetLevel() < pMapInfo->minLevel || pMapInfo->maxLevel < pPlayer->GetLevel() )
			throw ERROR_THEME_READY_PLAYER_LEVEL;

		/// 장비 소유 체크
		if( pMapInfo->itemIdx != 0 && pPlayer->IsItem( pMapInfo->itemIdx ) == false )
			throw ERROR_THEME_READY_ITEM;

		///// 파티유저가 이미 종속된 방이 있는 경우
		//if( pPlayer->GetPartyIndex() != 0 && pPlayer->IsThemeUser( MAKETHEMECID( themeMapNum, themeMode ) ) == true )
		//	throw ERROR_THEME_READY_DEPENDENCY;

		pPlayer->StartRequestRejection( eREQREJCT_THEME );

		if( pPlayer->IsThemeUser( MAKETHEMECID( themeMapNum, themeMode) ) == true )
		{
			/// 종속된경우 종속입장
			THEME_USER_UPDATE* themeUserUpdate = (THEME_USER_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_UPDATE );

			themeUserUpdate->themeIdx        = themeMapNum;
			themeUserUpdate->themeMode       = themeMode;
			themeUserUpdate->characterIdx    = playerIdx;
			themeUserUpdate->characterMaxCnt = 0;

			sMapInfo* info = STAGESCRIPT->GetMapFolderIdx( themeUserUpdate->themeIdx, themeUserUpdate->themeMode );
			themeUserUpdate->characterMaxCnt = ( info ) ? info->maxPlayer : 0;

			//themeUserUpdate->characterMaxCnt = CIRCUS_MEMBER_COUNT_MAX;

			NETWORK2->SendSQL( pPlayer->GetConnectionIdx(), handle, sizeof(THEME_USER_UPDATE) );
		}
		else		/// 종속이 안된경우
		{
			cParty* pParty = NULL;
			if( pPlayer->GetPartyIndex() != 0 )
				pParty = PARTYMAN->GetParty( pPlayer->GetPartyIndex() );

			if( pParty == NULL )
			{
				/// 파티가 없는 경우 신규입장
				THEME_ROOM_CHECK*  themeRoomCheck = (THEME_ROOM_CHECK*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_ROOM_CHECK );
				themeRoomCheck->characterIdx = playerIdx;
				themeRoomCheck->themeIdx   = themeMapNum;
				themeRoomCheck->themeMode  = themeMode;

				NETWORK2->SendSQL( handle, sizeof(THEME_ROOM_CHECK) );
			}
			else
			{
				/// db에서 파티원들의 귀속정보를 로드한다.
				THEME_PARTYROOM_SELECT* pRoomSelect = (THEME_PARTYROOM_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_PARTYROOM_SELECT );

				pRoomSelect->mThemeIdx = themeMapNum;
				pRoomSelect->mMode = themeMode;
				pRoomSelect->mRoomIdx = 0;
				pRoomSelect->mPartyCnt = (unsigned char)pParty->GetCount();
				for( unsigned int i = 0 ; i < pParty->GetCount() ; ++i )
					pRoomSelect->mPartyUserIdx[i] = pParty->GetUserArr()[i];

				NETWORK2->SendSQL( pPlayer->GetConnectionIdx(), handle, sizeof(THEME_PARTYROOM_SELECT) );
			}
		}
	}
	catch ( unsigned char errCode )
	{
		/// 수락창에서 거절한 경우를 제외하곤 모든 에러를 보여준다.
		if( errCode != ERROR_THEME_READY_REJECTION )
		{
			NETWORK2->SendMsgError( pPlayer->GetConnectionIdx(), NM_THEME, NM_THEME_READY_RES, errCode );
		}

		return;		
	}
}


void cThemeManager::ThemeDBErr( unsigned long playerIdx, unsigned char errorCode )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;

	/// 상태 해제
	NETWORK2->SendMsgError( pPlayer->GetConnectionIdx(), NM_THEME, NM_THEME_READY_RES, errorCode );

	pPlayer->SetThemePartyDB( false );
	pPlayer->EndRequestRejection( eREQREJCT_THEME );
}


cThemeObject* cThemeManager::ThemeJoin( cPlayer* pPlayer, unsigned short themeIdx, unsigned char levelMode, float *pX, float *pY )
{
	if( pX == NULL )
		return NULL;

	if( pY == NULL )
		return NULL;

	STAGESCRIPT->CalcTargetMapPos( mStartPosIdx, pX, pY );

	if( pPlayer == NULL )
	{
		NETWORK2->PostServerEvent("cThemeManager::ThemeJoin pPlayer == NULL");
		return NULL;
	}

	if( mType != _E_ST_ID_THEME_ )
	{
		NETWORK2->PostServerEvent("cThemeManager::ThemeJoin mType[%d,%d] != _E_ST_ID_THEME_", pPlayer->GetObjectID(), mType );
		return NULL;
	}
	
	cThemeObject* ptheme = (cThemeObject*)mThemeMap.GetAt( themeIdx );
	if( ptheme == NULL )
	{
		unsigned short mapDataNum = NETWORK2->GetInDunMapNum();
		switch( mapDataNum )
		{
		case MAP_CIRCUS:
			ptheme = new cThemeObject;
			break;
		case MAP_RIOHA:
			ptheme = new cThemeObject;
			break;
		case MAP_DECOMUSS:
			ptheme = new cThemeObject;
			break;
		default:
			return NULL;
		}

		if( ptheme->InitTheme( themeIdx, levelMode ) == false )
		{
			NETWORK2->PostServerEvent("cThemeManager::ThemeJoin ptheme->Init( %d,%d ) == false",
				pPlayer->GetObjectID(), themeIdx );
			SAFE_DELETE( ptheme );
			return NULL;
		}

		if( mThemeMap.Insert( themeIdx, ptheme ) == false )
		{
			NETWORK2->PostServerEvent("cThemeManager::ThemeJoin mThemeMap.Insert( %d,%d ) == false",
				pPlayer->GetObjectID(), themeIdx );
			SAFE_DELETE( ptheme );
			return NULL;
		}
	}
	else if( ptheme->GetMode() != levelMode)
	{
		NETWORK2->PostServerEvent("cThemeManager::ThemeJoin ptheme->GetMode() != levelMode" );
	}

	if( ptheme->ThemeAddPlayer( pPlayer->GetObjectID() ) == false )
	{
		NETWORK2->PostServerEvent("cThemeManager::ThemeJoin ptheme->ThemeAddPlayer( %d,%d ) == false",
			pPlayer->GetObjectID(), themeIdx );
		SAFE_DELETE( ptheme );
		return NULL;
	}
	
	return ptheme;
}

cThemeObject* cThemeManager::TutorialJoin( cPlayer* pPlayer, unsigned char levelMode, unsigned long tutorialMode, float *pX, float *pY, bool gmJoin )
{
	if( pX == NULL )
		return NULL;

	if( pY == NULL )
		return NULL;

	if( pPlayer == NULL )
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin pPlayer == NULL");
		return NULL;
	}

	if( mType != _E_ST_ID_TUTORIAL_ )
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin mType[%d,%d] != _E_ST_ID_TUTORIAL_", pPlayer->GetObjectID(), mType );
		return NULL;
	}

	if( tutorialMode == -1 )
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin modeIndex error(%d)", pPlayer->GetObjectID() );
		return NULL;
	}

	cTutorialMode* mode = TUTORIALSCRIPT->GetTutorialMode( tutorialMode );
	if( mode == NULL )
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin mode data error(%d,%d)", pPlayer->GetObjectID(), tutorialMode );
		return NULL;
	}

	NiPoint2 pos;
	if( STAGESCRIPT->CalcTargetMapPos( mode->mStartPosIndex, &pos.x, &pos.y ) == true )
	{
		*pX = pos.x;
		*pY = pos.y;
	}
	else
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin CalcTargetMapPos(%d,%d,%d)", pPlayer->GetObjectID(), tutorialMode, mode->mStartPosIndex );
		return NULL;
	}

	cPHashMap::cIterator b = mThemeMap.Begin();
	cPHashMap::cIterator e = mThemeMap.End();

	cThemeObject* ptheme = NULL;
	for( ; b != e ; ++b )
	{
		ptheme = (cThemeObject*)(*b).mSecond;
		if( ptheme == NULL )
			continue;

		if( ptheme->IsMemberMax() == true )
			continue;

		if( ptheme->ThemeAddPlayer( pPlayer->GetObjectID() ) == false )
		{
			NETWORK2->PostServerEvent("cThemeManager::TutorialJoin ptheme->ThemeAddPlayer( %d ) == false",
				pPlayer->GetObjectID() );
			continue;
		}

		if( gmJoin == false )
			SKILLMANAGER->AddTutorialHaveSkill( pPlayer );

		return ptheme;
	}

	if( gmJoin == false )
		SKILLMANAGER->AddTutorialHaveSkill( pPlayer );

	unsigned short themeIdx = (unsigned short)mIdxGen.GeneratIdx();

	if( themeIdx >= INDUN_MAP_MAX )
		return NULL;

	unsigned short mapDataNum = NETWORK2->GetInDunMapNum();
	switch( mapDataNum )
	{
	case MAP_TUTORIAL:
		ptheme = new cTutorialObject;
		break;
	default:
		return NULL;
	}

	if( ptheme->InitTheme( themeIdx, levelMode ) == false )
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin ptheme->Init( %d,%d ) == false",
			pPlayer->GetObjectID(), themeIdx );
		return NULL;
	}

	if( mThemeMap.Insert( themeIdx, ptheme ) == false )
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin mThemeMap.Insert( %d,%d ) == false",
			pPlayer->GetObjectID(), themeIdx );
		return NULL;
	}

	if( ptheme->ThemeAddPlayer( pPlayer->GetObjectID() ) == false )
	{
		NETWORK2->PostServerEvent("cThemeManager::TutorialJoin ptheme->ThemeAddPlayer( %d,%d ) == false",
			pPlayer->GetObjectID(), themeIdx );
		return NULL;
	}

	return ptheme;
}

cThemeObject* cThemeManager::GetThemeObject( unsigned long themeIdx )
{
	return (cThemeObject*)mThemeMap.GetAt( themeIdx );
}


void cThemeManager::ThemeRestart()
{
	///db에 요청
	mProcess = eTHEMEPROCESS_WAITDB;
	HANDLE        handle = NULL;
	THEME_CREATE* create = (THEME_CREATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_THEME_CREATE );

	// HACK - 시간과 룸 정의 해야됨.
	create->themeIdx = NETWORK2->GetInDunMapNum();
	create->maxRoom  = INDUN_MAP_MAX;
	sThemeTime* pTime = THEMETIMESCRIPT->GetThemeTime( NETWORK2->GetInDunMapNum() );
	if( pTime != NULL )
	{
		create->fixTime  = pTime->mResetClockHour;
		create->varTime  = pTime->mResetIntervalHour;
	}
	else
	{
		create->fixTime  = 12;
		create->varTime  = 24;
	}
	NETWORK2->SendSQL( handle, sizeof(THEME_CREATE) );

	/// 방전체 초기화
	cPHashMap::cIterator b = mThemeMap.Begin();
	cPHashMap::cIterator e = mThemeMap.End();
	for( ; b != e ; ++b )
	{
		cThemeObject* pObject = (cThemeObject*)(*b).mSecond;
		if( pObject == NULL )
			continue;

		pObject->ThemeAllOut();
		pObject->RelaseObject();
		SAFE_DELETE( pObject );
	}
	mThemeMap.Clear();

	OBJECTMANAGER->ThemeDataClear();
}


void cThemeManager::SetThemeRestartTime( unsigned long resetLeftTime )
{
	mNextResetTime = NETWORK2->GetAccumTime() + ( resetLeftTime );

	/// 다음 초기화 시간이 레디시간보다 짧은 경우
	if( resetLeftTime < THEME_READY_TIME )
	{
		/// 유저 진입 금지
		NETWORK2->PostThemeStatus( _E_THEME_READY_ );
		/// 다음초기화 이벤트 발생 시간 설정
		mProcess = eTHEMEPROCESS_READY;
		if( resetLeftTime < 10 * SECOND )
			mNextEventTime = NETWORK2->GetAccumTime() + ( 10 * SECOND );	/// 부하 방지를 위해서 10초갭을 준다.
		else
			mNextEventTime = NETWORK2->GetAccumTime() + ( resetLeftTime );
		return;
	}
	
	/// 다음 유저 진입 금지 시간설정
	if( NETWORK2->GetAccumTime() + resetLeftTime < THEME_READY_TIME )
		mNextEventTime = 0;
	else
		mNextEventTime = NETWORK2->GetAccumTime() + resetLeftTime - THEME_READY_TIME;

	/// 테마 초기화 다음 시간에 따른 상태 변경
	unsigned long checkTime = 24*HOUR;
	sThemeTime* pTime = THEMETIMESCRIPT->GetThemeTime( NETWORK2->GetInDunMapNum() );
	if( pTime != NULL )
		checkTime = pTime->mResetIntervalHour *HOUR;

	if( checkTime < resetLeftTime )
	{
		/// 체크 타임보다 남은시간이 크면
		mProcess = eTHEMEPROCESS_RUN;

		mThemeOpenSendTime = NETWORK2->GetAccumTime() + THEME_OPEN_SEND_TIME;
		NETWORK2->PostThemeStatus( _E_THEME_RUNNING_ );
		return;
	}
	else
	{
		///
		if( checkTime - resetLeftTime > THEME_READY_TIME )
		{
			/// 초기화 후 5분이상 지났다면
			mProcess = eTHEMEPROCESS_RUN;

			mThemeOpenSendTime = NETWORK2->GetAccumTime() + THEME_OPEN_SEND_TIME;
			NETWORK2->PostThemeStatus( _E_THEME_RUNNING_ );
		}
		else
		{
			/// 테마 초기화 후 아직 5분이 지나지 않았다면
			mProcess = eTHEMEPROCESS_5MIN;
			mReadyPlusTime = NETWORK2->GetAccumTime() + (THEME_READY_TIME - (checkTime - resetLeftTime));
		}
	}

}


bool cThemeManager::MonsterCntTest( unsigned short mapNum, unsigned long monsterIdx, unsigned long classIdx, unsigned long regenIdx )
{
	cThemeObject* pObject = (cThemeObject*)mThemeMap.GetAt( mapNum );
	if( pObject == NULL )
		return false;

	return pObject->MonsterCntTestObject( mapNum, monsterIdx, classIdx, regenIdx );	
}
