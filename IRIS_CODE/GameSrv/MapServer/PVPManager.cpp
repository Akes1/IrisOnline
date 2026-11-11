#include "stdafx.h"
#include "PVPManager.h"

#include "gamesrv.h"
#include "ObjectManager.h"
#include "PvPScript.h"
#include "StageScript.h"
#include "AppTimer.h"
#include "GridManager.h"


cPVPManager* cPVPManager::mpPVPManager = NULL;


cPVPManager::cPVPManager()
: mpDMInfo(0)
{
	if(mpPVPManager)
	{
		MessageBox(NULL,"cSkillManager : 이 클래스는 객체가 유일한객체(singleton)이어야 합니다.\
						이전 객체를 사용하던지 이전 객체를 지우고 생성하십시오.","경고",MB_OK);
	}
	else
	{
		mpPVPManager = this;
	}

	mpPvPScript = 0;

	mDMMaxCnt = 0;

	mPvPObjectRoot.pool = NULL;
	mPvPObjectRoot.root = NULL;

	mMapNumber = MAP_NONE;

	mTeamGenPos[0] = 0;
	mTeamGenPos[1] = 0;
}


cPVPManager::~cPVPManager()
{
	//Release();
}

bool cPVPManager::Init( SERVER_TYPE type, unsigned short mapNumber )
{
	for( unsigned char i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
	{
		mSectionInfo[i].mSecProcess = ePVPSECTION_WAIT;
		mSectionInfo[i].mMinLevel = i * 10 + 1;
		mSectionInfo[i].mMaxLevel = i * 10 + 10;
		mSectionInfo[i].mNoticeSendTime = 0;
		mSectionInfo[i].mNoticeCount = PVP_DM_NOTICE_TIME / MINUTE;
	}

	mSectionInfo[PVP_DM_LEVEL_SECTION_MAX-1].mMaxLevel = 100;

	/// 081114 PKH pvp 스크립트
	mpPvPScript = new cPvPScript;
	if ( !mpPvPScript->Init( ) )
	{
		return false;
	}

	if( type != _E_ST_ID_PVP_ )
	{
		mProcess = ePVPPROCESS_WAIT;
		return true;
	}

	///
	mpDMInfo = PVPSCRIPT->GetDmInfo( mapNumber );
	if( mpDMInfo == 0 )
		return false;

	/// 다음 시작 시간을 설정할 스크립트 참조정보를 얻어옴
	cPAry* pTimeAry = PVPSCRIPT->GetStartTimeAry();
	if( pTimeAry == NULL )
		return false;

	/// 시작 시간이 없으면 서버를 킬이유도 없다.
	if( pTimeAry->GetSize() == 0 )
		return false;

	unsigned long serverPlusTime = 0;
	//if( NETWORK2->GetServerNum() > 0 )
	//	serverPlusTime = ( NETWORK2->GetServerNum() - 1 ) * 10 * MINUTE;

	/// 현재 시계를 얻어옴
	TIMESTAMP_STRUCT now = TIMER->GetRealTime();
	for( unsigned long i = 0 ; i < pTimeAry->GetSize() ; ++i )
	{
		/// 스크립트 항목 로드
		sPvPStartTime* pStartTime = (sPvPStartTime*)(*pTimeAry)[i];
		if( pStartTime == NULL )
			return false;

		/// 초기화 설정 - 목록 마지막가지 선정이 안된경우 내일을 시작으로 설정
		if( i == 0 )
		{
			/// 날짜를 하루 다음날로 놓고 시간을 스크립트 처음것으로 설정
			TIMESTAMP_STRUCT tomorrow = TIMER->GetRealPlusTime( 1 * DAY );
			tomorrow.hour = pStartTime->mHour;
			tomorrow.minute = pStartTime->mMinute;
			tomorrow.second = 0;

			struct tm tmTomorrow = TIMER->ConvertTStoTM( tomorrow );
			time_t result;

			/// 서버군별 시작시간+@ 시간 더하기
			tmTomorrow.tm_sec = tmTomorrow.tm_sec + ( serverPlusTime / 1000 );

			if( (result = mktime( &tmTomorrow )) != (time_t)-1 )
			{
				//datetime = *localtime( &result );
				errno_t err = localtime_s( &tmTomorrow , &result );
				if( err != 0 )
					NETWORK2->PostServerEvent("cPvPManager::Init errno_t err[%d]", err );
			}

			tomorrow.year	= (SQLSMALLINT)tmTomorrow.tm_year+1900;
			tomorrow.month  = (SQLSMALLINT)tmTomorrow.tm_mon+1;
			tomorrow.day    = (SQLSMALLINT)tmTomorrow.tm_mday;
			tomorrow.hour   = (SQLSMALLINT)tmTomorrow.tm_hour;
			tomorrow.minute = (SQLSMALLINT)tmTomorrow.tm_min;
			tomorrow.second = (SQLSMALLINT)tmTomorrow.tm_sec;
			tomorrow.fraction = 0;
			
			mNextStartTime = tomorrow;

			mStartTimeScriptPos = i;
		}

		TIMESTAMP_STRUCT scriptTime = now;
		scriptTime.hour = pStartTime->mHour;
		scriptTime.minute = pStartTime->mMinute;
		scriptTime.second = 0;

		struct tm tmScript = TIMER->ConvertTStoTM( scriptTime );
		time_t result;
		
		/// 서버군별 시작시간+@ 시간 더하기
		tmScript.tm_sec = tmScript.tm_sec + ( serverPlusTime / 1000 );

		if( (result = mktime( &tmScript )) != (time_t)-1 )
		{
			//datetime = *localtime( &result );
			errno_t err = localtime_s( &tmScript , &result );
			if( err != 0 )
				NETWORK2->PostServerEvent("cPvPManager::Init errno_t err[%d]", err );
		}

		scriptTime.year	  = (SQLSMALLINT)tmScript.tm_year+1900;
		scriptTime.month  = (SQLSMALLINT)tmScript.tm_mon+1;
		scriptTime.day    = (SQLSMALLINT)tmScript.tm_mday;
		scriptTime.hour   = (SQLSMALLINT)tmScript.tm_hour;
		scriptTime.minute = (SQLSMALLINT)tmScript.tm_min;
		scriptTime.second = (SQLSMALLINT)tmScript.tm_sec;
		scriptTime.fraction = 0;				

		/// 스크립트 시간+서버별 증감시간 이랑 현재 시간이랑 비교해서 스크립트 시간이 지나지 않았으면 다음 시작으로 설정
		if( TIMER->DiffRealTime( scriptTime ) > 0 ) 
		{
			mNextStartTime = scriptTime;

			mStartTimeScriptPos = i;
			break;
		}

		/*
		/// 현재 시계와 스크립트 시계를 비교해서 오늘 켜질수 있는것중 가장 가까운것을 찾는다.
		/// ※ 찾지 못하는 경우 위의 초기화 설정부에서 하루 다음날 처음 스크립트 위치의 시간으로 진행된다
		if( pStartTime->mHour > now.hour || ( pStartTime->mHour == now.hour && pStartTime->mMinute > now.minute ) )
		{
			mNextStartTime = now;
			mNextStartTime.hour = pStartTime->mHour;
			mNextStartTime.minute = pStartTime->mMinute;
			mNextStartTime.second = 0;

			mStartTimeScriptPos = i;
			break;
		}
		*/
	}

	/////
	//tPointerArray<void*>* pArray = STAGESCRIPT->GetMapChangeDestArr( mapNumber );
	//if( pArray == 0 || pArray->GetSize() < 2 )
	//{
	//	assert(0);
	//	return false;
	//}

	//sStageChangePos* pAPos = (sStageChangePos*)(*pArray)[0];
	//sStageChangePos* pBPos = (sStageChangePos*)(*pArray)[1];
	//if( pAPos == 0 || pBPos == 0 )
	//{
	//	assert(0);
	//	return false;
	//}
	//mTeamGenPos[0] = pAPos->mPosIdx;
	//mTeamGenPos[1] = pBPos->mPosIdx;

	if( mapNumber == MAP_DEATHMATCH )
	{
		sStageChangePos* pAPos = STAGESCRIPT->GetPosScriptInfo( 210101 );
		sStageChangePos* pBPos = STAGESCRIPT->GetPosScriptInfo( 210102 );
		if( pAPos == 0 || pBPos == 0 )
		{
			assert(0);
			return false;
		}
		mTeamGenPos[0] = pAPos->mPosIdx;
		mTeamGenPos[1] = pBPos->mPosIdx;
	}
	else
	{
		sStageChangePos* pAPos = STAGESCRIPT->GetPosScriptInfo( 210201 );
		sStageChangePos* pBPos = STAGESCRIPT->GetPosScriptInfo( 210202 );
		if( pAPos == 0 || pBPos == 0 )
		{
			assert(0);
			return false;
		}
		mTeamGenPos[0] = pAPos->mPosIdx;
		mTeamGenPos[1] = pBPos->mPosIdx;
	}

	/// 레벨 구간별 인원선정용 구조체 생성
	unsigned int blockCnt = mpDMInfo->mLevelBlockInfo.GetSize();
	for( unsigned int i=0;i<blockCnt; ++i )
	{
		sDMBlockInfo* pInfo = mpDMInfo->mLevelBlockInfo[i];
		if( pInfo == 0 )
		{
			assert(0);
			return false;
		}

		/// 구간정보 생성
		sLevelForceInfo* pLvForceInfo = new sLevelForceInfo;
		pLvForceInfo->mpBlockInfo = pInfo;

		mLevelForceAry.PushBack( pLvForceInfo );

		/// 레벨별 참조 정보 생성
		for( unsigned char j = pInfo->mLevelMin ; j <= pInfo->mLevelMax ; ++j )
		{
			if( mLevelForceMap.Insert( j, pLvForceInfo ) == false )
			{
				assert(0);
				return false;
			}
		}
	}

	mMapNumber = mapNumber;
	mProcess = ePVPPROCESS_WAIT;

	/// 서버다운으로 남아있을지 모르는 데이터를 초기화 해준다.
	HANDLE handle = NULL;
	PVP_USER_CLEAR* pPvPClear = (PVP_USER_CLEAR*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_PVP_USER_CLEAR );
	if( pPvPClear != NULL )
	{
		pPvPClear->pvpIdx = mapNumber;
		pPvPClear->minLevel = 0;
		pPvPClear->maxLevel = CHAR_MAX;
		NETWORK2->SendSQL( handle, sizeof(PVP_USER_CLEAR) );
	}

	return true;
}


void cPVPManager::Release()
{
	for( unsigned long i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
	{
		if( mSectionInfo[i].mSecProcess != ePVPPROCESS_WAIT )
			NETWORK2->PostPVPStatus( _E_PVP_CLOSING_, NETWORK2->GetInDunMapNum(), i );
	}

	PVPOBJECTPOOL->Shutdown();
	PVPPLAYERLISTPOOL->Shutdown();

	mLevelForceMap.Clear();

	for( unsigned long i = 0 ; i < mLevelForceAry.GetSize() ; ++i )
	{
		sLevelForceInfo* pLevelForceInfo = (sLevelForceInfo*)mLevelForceAry[i];
		SAFE_DELETE( pLevelForceInfo );
	}
	mLevelForceAry.Clear();

//	if( mPvPType == ePVPSERVERTYPE_NONE )
	SAFE_RELEASE_DELETE( mpPvPScript );
}


void cPVPManager::Process( unsigned long elapsedTime, unsigned long accumTime )
{
	switch( mProcess )
	{
	case ePVPPROCESS_WAIT:
		{

			if( TIMER->DiffRealTime( mNextStartTime ) > 0 )
				return;

			//////////////////////////////////////////////////////////////////////////
			/// 스크립트를 읽어오지 못하면 더 진행을 시키지 않는다.
			cPAry* pTimeAry = PVPSCRIPT->GetStartTimeAry();
			if( pTimeAry == NULL )
			{
				assert(NULL);
				return;
			}

			/// 시작 시간이 없으면 더 진행을 시키지 않는다.
			if( pTimeAry->GetSize() == 0 )
			{
				assert(NULL);
				return;
			}

			unsigned long serverPlusTime = 0;
			//if( NETWORK2->GetServerNum() > 0 )				
			//	serverPlusTime = ( NETWORK2->GetServerNum() - 1 ) * 10 * MINUTE;

			/// 마지막 시간가지 사용했으면 날짜를 하루 넘기고 처음 시작시간으로 설정한다.
			if( mStartTimeScriptPos + 1 >= pTimeAry->GetSize() )
			{
				/// 스크립트 항목 로드
				sPvPStartTime* pStartTime = (sPvPStartTime*)(*pTimeAry)[0];
				if( pStartTime == NULL )
				{
					assert(NULL);
					return;
				}

				/// 날짜를 하루 다음날로 놓고 시간을 스크립트 처음것으로 설정
				TIMESTAMP_STRUCT tomorrow = TIMER->GetRealPlusTime( 1 * DAY );
				tomorrow.hour = pStartTime->mHour;
				tomorrow.minute = pStartTime->mMinute;
				tomorrow.second = 0;

				struct tm tmTomorrow = TIMER->ConvertTStoTM( tomorrow );
				time_t result;

				/// 서버군별 시작시간+@ 시간 더하기
				tmTomorrow.tm_sec = tmTomorrow.tm_sec + ( serverPlusTime / 1000 );

				if( (result = mktime( &tmTomorrow )) != (time_t)-1 )
				{
					//datetime = *localtime( &result );
					errno_t err = localtime_s( &tmTomorrow , &result );
					if( err != 0 )
						NETWORK2->PostServerEvent("cPvPManager::Init errno_t err[%d]", err );
				}

				tomorrow.year	= (SQLSMALLINT)tmTomorrow.tm_year+1900;
				tomorrow.month  = (SQLSMALLINT)tmTomorrow.tm_mon+1;
				tomorrow.day    = (SQLSMALLINT)tmTomorrow.tm_mday;
				tomorrow.hour   = (SQLSMALLINT)tmTomorrow.tm_hour;
				tomorrow.minute = (SQLSMALLINT)tmTomorrow.tm_min;
				tomorrow.second = (SQLSMALLINT)tmTomorrow.tm_sec;
				tomorrow.fraction = 0;

				mNextStartTime = tomorrow;

				mStartTimeScriptPos = 0;

			}
			else
			{
				/// 스크립트 항목 로드
				sPvPStartTime* pStartTime = (sPvPStartTime*)(*pTimeAry)[mStartTimeScriptPos + 1];
				if( pStartTime == NULL )
				{
					assert(NULL);
					return;
				}

				/// 다음번에 있는 시간 값을 가지고와 셋팅한다.
				TIMESTAMP_STRUCT nextTime = mNextStartTime;
				nextTime.hour = pStartTime->mHour;
				nextTime.minute = pStartTime->mMinute;
				nextTime.second = 0;

				struct tm tmNextTime = TIMER->ConvertTStoTM( nextTime );
				time_t result;

				/// 서버군별 시작시간+@ 시간 더하기
				tmNextTime.tm_sec = tmNextTime.tm_sec + ( serverPlusTime / 1000 );

				if( (result = mktime( &tmNextTime )) != (time_t)-1 )
				{
					//datetime = *localtime( &result );
					errno_t err = localtime_s( &tmNextTime , &result );
					if( err != 0 )
						NETWORK2->PostServerEvent("cPvPManager::Init errno_t err[%d]", err );
				}

				nextTime.year	= (SQLSMALLINT)tmNextTime.tm_year+1900;
				nextTime.month  = (SQLSMALLINT)tmNextTime.tm_mon+1;
				nextTime.day    = (SQLSMALLINT)tmNextTime.tm_mday;
				nextTime.hour   = (SQLSMALLINT)tmNextTime.tm_hour;
				nextTime.minute = (SQLSMALLINT)tmNextTime.tm_min;
				nextTime.second = (SQLSMALLINT)tmNextTime.tm_sec;
				nextTime.fraction = 0;

				mNextStartTime = nextTime;

				++mStartTimeScriptPos;
			}

			/// 공지발송 끝나는 시간
			mNextEventTime = accumTime + PVP_DM_NOTICE_TIME;
			mDMMaxCnt = 0;	
			mProcess = ePVPPROCESS_RUN;
			for( unsigned long i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
			{
				/// 구간별 전장 시작 정보 설정
				mSectionInfo[i].mSecProcess = ePVPSECTION_WAIT;
				mSectionInfo[i].mNextEventTime = accumTime + i * PVP_DM_LEVEL_START_TERM;
			}
		}
	case ePVPPROCESS_RUN:
		{
			for( unsigned long i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
			{
				/// 구간별 모집
				switch( mSectionInfo[i].mSecProcess )
				{
				case ePVPSECTION_WAIT:
					{
						if( mSectionInfo[i].mNextEventTime > accumTime )
							continue;

						/// 공지 발송 시작
						NETWORK2->PostPvPNotice( PVP_DM_NOTICE_CNT, NETWORK2->GetInDunMapNum(), i );
						mSectionInfo[i].mSecProcess = ePVPPROCESS_NOTICE;
						mSectionInfo[i].mNextEventTime = accumTime + PVP_DM_NOTICE_TIME;
						mSectionInfo[i].mNoticeSendTime = 0;
						mSectionInfo[i].mNoticeCount = PVP_DM_NOTICE_CNT;
					}
					break;
				case ePVPPROCESS_NOTICE:
					{

						if( mSectionInfo[i].mNextEventTime <= accumTime )
						{
							/// 공지끝 - 참여자 신청받음
							NETWORK2->PostPVPStatus( _E_PVP_READY_, NETWORK2->GetInDunMapNum(), i );
							mSectionInfo[i].mSecProcess = ePVPPROCESS_READY;
							mSectionInfo[i].mNextEventTime = accumTime + PVP_DM_JOIN_TIME;
						}
						else
						{
							/// pvp 공지 발송
							mSectionInfo[i].mNoticeSendTime += elapsedTime;
							if( mSectionInfo[i].mNoticeSendTime > MINUTE )
							{
								mSectionInfo[i].mNoticeCount--;
								mSectionInfo[i].mNoticeSendTime = mSectionInfo[i].mNoticeSendTime-MINUTE;
								NETWORK2->PostPvPNotice( mSectionInfo[i].mNoticeCount, NETWORK2->GetInDunMapNum(), i );
							}
						}
					}
					break;
				case ePVPPROCESS_READY:
					{
						if( mSectionInfo[i].mNextEventTime > accumTime )
							continue;

						/// 참여자 신청 마감
						NETWORK2->PostPVPStatus( _E_PVP_PLAYERLISTWAIT_, NETWORK2->GetInDunMapNum(), i );
 
						HANDLE            handle         = NULL;
						PVP_USER_SELECT* pPvPSelect = (PVP_USER_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_PVP_USER_SELECT );
						if( pPvPSelect != NULL )
						{
							pPvPSelect->pvpIdx = NETWORK2->GetInDunMapNum();
							pPvPSelect->aryPos = i;
							pPvPSelect->minLevel = mSectionInfo[i].mMinLevel;
							pPvPSelect->maxLevel = mSectionInfo[i].mMaxLevel;
							NETWORK2->SendSQL( handle, sizeof(PVP_USER_SELECT) );
						}

						/// 방별로 입장부터 끝날때까지 프로세스를 맡긴다
						mSectionInfo[i].mSecProcess = ePVPPROCESS_RUNOBJECT;
						mSectionInfo[i].mNextEventTime = 0;
					}
					break;
				}
			}

			/// 방별 전장 프로세싱
			ObjectProcess( elapsedTime, accumTime );

			/// 시간이 다되도록 인던이 사라지지 않으면 정리해준다.
			if( TIMER->DiffRealTime( mNextStartTime ) <= 0 )
			{
				mProcess = ePVPPROCESS_WAIT;
				//for( unsigned long i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
				//{
				//	/// 프로세스 상태 바꿈
				//	mSectionInfo[i].mSecProcess = ePVPSECTION_WAIT;
				//	NETWORK2->PostPVPStatus( _E_PVP_CLOSED_, NETWORK2->GetInDunMapNum(), i );
				//}

				cBaseDeathMatch* pPvPObject = (cBaseDeathMatch*)mPvPObjectRoot.pool;
				while( pPvPObject )
				{
					/// 사라지지 않은 방정보 로그남김
					NETWORK2->PostServerEvent("cPVPManager::Process TIMER->DiffRealTime pPvPObject[%d,%d,%d][%d,%d]", 
						pPvPObject->GetType(), pPvPObject->GetDMIdx(), pPvPObject->GetProcess(), 
						pPvPObject->GetProcessEndTime(), NETWORK2->GetAccumTime());

					pPvPObject->EndDM();
					cBaseDeathMatch* pDelObject = pPvPObject;


					pPvPObject = (cBaseDeathMatch*)pPvPObject->next;

					/// 오브젝트 삭제
					PVPOBJECTPOOL->ReleasePvPObject( &mPvPObjectRoot, pDelObject->GetDMIdx() );
				}

				PVPOBJECTPOOL->Shutdown();
				mPvPObjectRoot.pool = NULL;
				mPvPObjectRoot.root = NULL;
				mPvPObjectRoot.count = 0;

				for( unsigned long i = 0 ; i < mLevelForceAry.GetSize() ; ++i )
				{
					sLevelForceInfo* pLvForceInfo = (sLevelForceInfo*)mLevelForceAry[i];
					if( pLvForceInfo != NULL )
						pLvForceInfo->Init();
				}
			}
		}
	}

/*
	switch( mProcess )
	{
	case ePVPPROCESS_WAIT:
		{
			if( TIMER->DiffRealTime( mNextStartTime ) > 0 )
				return;

			mAutoOutCheckTime = 0;

			//////////////////////////////////////////////////////////////////////////
			/// 스크립트를 읽어오지 못하면 더 진행을 시키지 않는다.
			cPAry* pTimeAry = PVPSCRIPT->GetStartTimeAry();
			if( pTimeAry == NULL )
			{
				assert(NULL);
				return;
			}

			/// 시작 시간이 없으면 더 진행을 시키지 않는다.
			if( pTimeAry->GetSize() == 0 )
			{
				assert(NULL);
				return;
			}

			unsigned long serverPlusTime = 0;
			if( NETWORK2->GetServerNum() > 0 )
				serverPlusTime = ( NETWORK2->GetServerNum() - 1 ) * 10 * MINUTE;

			/// 마지막 시간가지 사용했으면 날짜를 하루 넘기고 처음 시작시간으로 설정한다.
			if( mStartTimeScriptPos + 1 >= pTimeAry->GetSize() )
			{
				/// 스크립트 항목 로드
				sPvPStartTime* pStartTime = (sPvPStartTime*)(*pTimeAry)[0];
				if( pStartTime == NULL )
				{
					assert(NULL);
					return;
				}

				/// 날짜를 하루 다음날로 놓고 시간을 스크립트 처음것으로 설정
				TIMESTAMP_STRUCT tomorrow = TIMER->GetRealPlusTime( 1 * DAY );
				tomorrow.hour = pStartTime->mHour;
				tomorrow.minute = pStartTime->mMinute;
				tomorrow.second = 0;

				struct tm tmTomorrow = TIMER->ConvertTStoTM( tomorrow );
				time_t result;

				/// 서버군별 시작시간+@ 시간 더하기
				tmTomorrow.tm_sec = tmTomorrow.tm_sec + ( serverPlusTime / 1000 );

				if( (result = mktime( &tmTomorrow )) != (time_t)-1 )
				{
					//datetime = *localtime( &result );
					errno_t err = localtime_s( &tmTomorrow , &result );
					if( err != 0 )
						NETWORK2->PostServerEvent("cPvPManager::Init errno_t err[%d]", err );
				}

				tomorrow.year	= (SQLSMALLINT)tmTomorrow.tm_year+1900;
				tomorrow.month  = (SQLSMALLINT)tmTomorrow.tm_mon+1;
				tomorrow.day    = (SQLSMALLINT)tmTomorrow.tm_mday;
				tomorrow.hour   = (SQLSMALLINT)tmTomorrow.tm_hour;
				tomorrow.minute = (SQLSMALLINT)tmTomorrow.tm_min;
				tomorrow.second = (SQLSMALLINT)tmTomorrow.tm_sec;
				tomorrow.fraction = 0;

				mNextStartTime = tomorrow;

				mStartTimeScriptPos = 0;

			}
			else
			{
				/// 스크립트 항목 로드
				sPvPStartTime* pStartTime = (sPvPStartTime*)(*pTimeAry)[mStartTimeScriptPos + 1];
				if( pStartTime == NULL )
				{
					assert(NULL);
					return;
				}

				/// 다음번에 있는 시간 값을 가지고와 셋팅한다.
				TIMESTAMP_STRUCT nextTime = mNextStartTime;
				nextTime.hour = pStartTime->mHour;
				nextTime.minute = pStartTime->mMinute;
				nextTime.second = 0;

				struct tm tmNextTime = TIMER->ConvertTStoTM( nextTime );
				time_t result;

				/// 서버군별 시작시간+@ 시간 더하기
				tmNextTime.tm_sec = tmNextTime.tm_sec + ( serverPlusTime / 1000 );

				if( (result = mktime( &tmNextTime )) != (time_t)-1 )
				{
					//datetime = *localtime( &result );
					errno_t err = localtime_s( &tmNextTime , &result );
					if( err != 0 )
						NETWORK2->PostServerEvent("cPvPManager::Init errno_t err[%d]", err );
				}

				nextTime.year	= (SQLSMALLINT)tmNextTime.tm_year+1900;
				nextTime.month  = (SQLSMALLINT)tmNextTime.tm_mon+1;
				nextTime.day    = (SQLSMALLINT)tmNextTime.tm_mday;
				nextTime.hour   = (SQLSMALLINT)tmNextTime.tm_hour;
				nextTime.minute = (SQLSMALLINT)tmNextTime.tm_min;
				nextTime.second = (SQLSMALLINT)tmNextTime.tm_sec;
				nextTime.fraction = 0;

				mNextStartTime = nextTime;

				++mStartTimeScriptPos;
			}

			/// 공지 발송 시작 설정
			mProcess = ePVPPROCESS_NOTICE;
			/// 공지발송 끝나는 시간
			mNextEventTime = accumTime + PVP_DM_NOTICE_TIME;

			mNoticeCount = PVP_DM_NOTICE_TIME / MINUTE;
			mNoticeSendTime = 0;

			NETWORK2->PostPvPNotice( mNoticeCount, NETWORK2->GetInDunMapNum() );
			mDMMaxCnt = 0;				
		}
		break;
	case ePVPPROCESS_NOTICE:
		{
			if( mNextEventTime <= accumTime )
			{
				/// 공지끝 - 참여자 신청받음
				NETWORK2->PostPVPStatus( _E_PVP_READY_, NETWORK2->GetInDunMapNum() );
				mProcess = ePVPPROCESS_READY;
				mNextEventTime = accumTime + PVP_DM_JOIN_TIME;
			}
			else
			{
				/// pvp 공지 발송
				mNoticeSendTime += elapsedTime;
				if( mNoticeSendTime > MINUTE )
				{
					mNoticeCount--;
					mNoticeSendTime = mNoticeSendTime-MINUTE;
					NETWORK2->PostPvPNotice( mNoticeCount, NETWORK2->GetInDunMapNum() );
				}
			}
		}
		break;
	case ePVPPROCESS_READY:
		{
			if( mNextEventTime <= accumTime )
			{
				/// 참여자 신청 마감
				NETWORK2->PostPVPStatus( _E_PVP_PLAYERLISTWAIT_, NETWORK2->GetInDunMapNum() );
				mProcess = ePVPPROCESS_JOINDBWAIT;
				mNextEventTime = 0;	/// DB 최대 대기시간 설정

				/// 게임서버로 부터 등록된 플레이어 DB정보를 읽어 온다.
				HANDLE            handle         = NULL;
				PVP_USER_SELECT* pPvPSelect = (PVP_USER_SELECT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_PVP_USER_SELECT );
				if( pPvPSelect != NULL )
				{
					pPvPSelect->pvpIdx = NETWORK2->GetInDunMapNum();
					NETWORK2->SendSQL( handle, sizeof(PVP_USER_SELECT) );
				}	
			}
		}
		break;
	case ePVPPROCESS_JOINDBWAIT:
		/// db처리가 안올경우를 대비하여 일정시간후에 진행 불가 메세지와 진행상태를 바꿔주자!
		break;
	case ePVPPROCESS_JOINREADY:
		{
			if( mNextEventTime <= accumTime )
			{
				/// 인던 객체들 전체 레디
				cBaseDeathMatch* pPvPObject = (cBaseDeathMatch*)mPvPObjectRoot.pool;
				while( pPvPObject )
				{
					pPvPObject->Ready();

					pPvPObject = (cBaseDeathMatch*)pPvPObject->next;
				}

				mProcess = ePVPPROCESS_RUN;
			}
		}
		break;
	case ePVPPROCESS_RUN:
		{
			/// 개별 전장 프로세싱
			ObjectProcess( elapsedTime, accumTime );

			/// 진행중인 객체가 없으면 상태 바꿈
			if( mPvPObjectRoot.count == 0 )
			{
				mProcess = ePVPPROCESS_WAIT;
				NETWORK2->PostPVPStatus( _E_PVP_CLOSED_, NETWORK2->GetInDunMapNum() );

				for( unsigned long i = 0 ; i < mLevelForceAry.GetSize() ; ++i )
				{
					sLevelForceInfo* pLvForceInfo = (sLevelForceInfo*)mLevelForceAry[i];
					if( pLvForceInfo != NULL )
						pLvForceInfo->Init();
				}
			}
			else
			{
				/// 시간이 다되도록 인던이 사라지지 않으면 정리해준다.
				if( TIMER->DiffRealTime( mNextStartTime ) <= 0 )
				{
					cBaseDeathMatch* pPvPObject = (cBaseDeathMatch*)mPvPObjectRoot.pool;
					while( pPvPObject )
					{
						/// 사라지지 않은 방정보 로그남김
						NETWORK2->PostServerEvent("cPVPManager::Process TIMER->DiffRealTime pPvPObject[%d,%d,%d][%d,%d]", 
							pPvPObject->GetType(), pPvPObject->GetDMIdx(), pPvPObject->GetProcess(), 
							pPvPObject->GetProcessEndTime(), NETWORK2->GetAccumTime());

						pPvPObject->EndDM();

						pPvPObject = (cBaseDeathMatch*)pPvPObject->next;
					}

					PVPOBJECTPOOL->Shutdown();
					mPvPObjectRoot.pool = NULL;
					mPvPObjectRoot.root = NULL;
					mPvPObjectRoot.count = 0;

					NETWORK2->PostServerEvent("cPVPManager::Process TIMER->DiffRealTime( [%d,%d,%d] [%d,%d,%d] ) <= 0",
						mNextStartTime.hour, mNextStartTime.minute, mNextStartTime.second, TIMER->GetRealTime().hour,
						TIMER->GetRealTime().minute, TIMER->GetRealTime().second);

					mProcess = ePVPPROCESS_WAIT;
					NETWORK2->PostPVPStatus( _E_PVP_CLOSED_, NETWORK2->GetInDunMapNum() );

					for( unsigned long i = 0 ; i < mLevelForceAry.GetSize() ; ++i )
					{
						sLevelForceInfo* pLvForceInfo = (sLevelForceInfo*)mLevelForceAry[i];
						if( pLvForceInfo != NULL )
							pLvForceInfo->Init();
					}
				}
			}	
		}
		break;
	}
	*/
}


void cPVPManager::ObjectProcess( unsigned long elapsedTime, unsigned long accumTime )
{
	/// 개별 오브젝트 프로세스
	cBaseDeathMatch* pPvPObject = (cBaseDeathMatch*)mPvPObjectRoot.pool;
	while( pPvPObject )
	{
		pPvPObject->Process( elapsedTime, accumTime );

		pPvPObject = (cBaseDeathMatch*)pPvPObject->next;
	}

	/// 삭제 예약 오브젝트 삭제
	for( unsigned long i = 0 ; i < mDelDMAry.GetSize() ; ++i )
	{
		PVPOBJECTPOOL->ReleasePvPObject( &mPvPObjectRoot, mDelDMAry[i] );
	}
	mDelDMAry.Clear();	
}


cBaseDeathMatch* cPVPManager::GetPvPDMObject( unsigned long dmIdx )
{
	return PVPOBJECTPOOL->GetPvPObject( &mPvPObjectRoot, dmIdx );
}

void cPVPManager::TeamDivision( PVP_USER_SELECT* select )
{
	if( mSectionInfo[select->aryPos].mSecProcess != ePVPPROCESS_RUNOBJECT )
		return;

	///// 프로세스 상태 변경
	//mSectionInfo[select->aryPos].mSecProcess = ePVPPROCESS_JOINREADY;

	///// 플레이어들이 접속하기 까지 기다려주는시간 PVP_DM_READY_PRINT1_TIME 시간이되면 레디로 바꿈
	//mSectionInfo[select->aryPos].mNextEventTime = NETWORK2->GetAccumTime() + PVP_DM_READY_TIME - PVP_DM_READY_PRINT1_TIME;

	/// 기존 정보 초기화
	for( unsigned long i = 0 ; i < mLevelForceAry.GetSize() ; ++i )
	{
		sLevelForceInfo* pInfo = (sLevelForceInfo*)mLevelForceAry[i];
		if( pInfo == 0 )
		{
			assert(0);
			continue;
		}

		pInfo->Init();
	}

	/// 레벨 구간별 인원카운트
	for( int i = 0 ; i < select->rowCount ; ++i )
	{
		sLevelForceInfo* pLevelForceInfo = (sLevelForceInfo*)mLevelForceMap.GetAt( select->table[i].characterLevel );
		if( pLevelForceInfo == NULL )
		{
			assert(0);
			continue;
		}
		pLevelForceInfo->mForceCnt[select->table[i].characterForceType]++;
	}

	/// 레벨 구간별 팀세력 결정
	for( unsigned long i = 0 ; i < mLevelForceAry.GetSize() ; ++i )
	{
		sLevelForceInfo* pLevelForceInfo = (sLevelForceInfo*)mLevelForceAry[i];
		if( pLevelForceInfo == NULL )
		{
			assert(0);
			continue;
		}

		/// 전체 인원 계산 및 인원이 제일 많은 세력과 제일 적은 세력 선정
		int maxForce = eFORCETYPE_FIRE;
		int minForce = eFORCETYPE_EARTH;

		unsigned long totalCnt = 0;
		for( int i2 = eFORCETYPE_FIRE ; i2 < eFORCETYPE_MAX ; ++i2 )
		{
			totalCnt = totalCnt + pLevelForceInfo->mForceCnt[i2];

			if( pLevelForceInfo->mForceCnt[maxForce] < pLevelForceInfo->mForceCnt[i2] )
				maxForce = i2;
			if( pLevelForceInfo->mForceCnt[minForce] > pLevelForceInfo->mForceCnt[i2] )
				minForce = i2;
		}

		
		if( totalCnt > pLevelForceInfo->mForceCnt[maxForce] )
		{
			/// 인원이 가장 많은 세력은 a팀
			pLevelForceInfo->mATeam[maxForce] = true;

			unsigned long teamCnt[ePVPDM_TEAMTYPE_MAX] = {0,};
			teamCnt[ePVPDM_TEAMTYPE_A] = pLevelForceInfo->mForceCnt[maxForce];
			teamCnt[ePVPDM_TEAMTYPE_B] = totalCnt - pLevelForceInfo->mForceCnt[maxForce];

			if( pLevelForceInfo->mForceCnt[maxForce] < totalCnt - pLevelForceInfo->mForceCnt[maxForce] )
			{
				unsigned long gap1 = abs((long)( totalCnt - pLevelForceInfo->mForceCnt[maxForce]*2 ));
				unsigned long gap2 = abs((long)( totalCnt - (pLevelForceInfo->mForceCnt[maxForce] + pLevelForceInfo->mForceCnt[minForce])*2 ));
				if( gap1 >= gap2 )
				{
					pLevelForceInfo->mATeam[minForce] = true;

					teamCnt[ePVPDM_TEAMTYPE_A] = pLevelForceInfo->mForceCnt[maxForce] + pLevelForceInfo->mForceCnt[minForce];
					teamCnt[ePVPDM_TEAMTYPE_B] = totalCnt - pLevelForceInfo->mForceCnt[maxForce] + pLevelForceInfo->mForceCnt[minForce];
				}
			}

			/// 생성 방의 갯수 선정
			pLevelForceInfo->mCreateMaxRoom = 0;
			ePVPDM_TEAM_TYPE minTeam = ePVPDM_TEAMTYPE_A;
			if( teamCnt[ePVPDM_TEAMTYPE_A] > teamCnt[ePVPDM_TEAMTYPE_B] )
				minTeam = ePVPDM_TEAMTYPE_B;

			if( mpDMInfo->mTeamPlayerMax == 0 )
				continue;

			pLevelForceInfo->mCreateMaxRoom = teamCnt[minTeam] / mpDMInfo->mTeamPlayerMax;
			if( teamCnt[minTeam] % mpDMInfo->mTeamPlayerMax >= mpDMInfo->mTeamPlayerMin )
				++pLevelForceInfo->mCreateMaxRoom;
		}
	}

	/// 내부서버간 통신(udp)을위한 메모리를 얻음
	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_POSTPVP_PLAYERLIST* pList = (MSG_SYN_POSTPVP_PLAYERLIST*)perIoContext->buffer;

	pList->mAryPos = select->aryPos;

	/// 참여 가능자와 불가능자 구별
	for( int i = 0 ; i < select->rowCount ; ++i )
	{
		/// 내부서버 전송용 메세지 작성
		pList->mList[i].mCharacterChannel = select->table[i].characterChannel;
		pList->mList[i].mPlayerIdx = select->table[i].characterIdx;
		pList->mList[i].mLevel = select->table[i].characterLevel;
		pList->mList[i].mForceType = select->table[i].characterForceType;

		pList->mList[i].mSelection = false;

		sLevelForceInfo* pLevelForceInfo = (sLevelForceInfo*)mLevelForceMap.GetAt( select->table[i].characterLevel );
		if( pLevelForceInfo == NULL )
			continue;

		if( pLevelForceInfo->mCreateMaxRoom == 0 )
			continue;

		/// a팀 b팀 구분
		ePVPDM_TEAM_TYPE teamType = ePVPDM_TEAMTYPE_A;
		if( pLevelForceInfo->mATeam[select->table[i].characterForceType] == false )
			teamType = ePVPDM_TEAMTYPE_B;

		if( mpDMInfo->mTeamPlayerMax == 0 )
			continue;

		/// 입장 가능 확인 - 인원 배분
		if( pLevelForceInfo->mTeamCnt[teamType] <= mpDMInfo->mTeamPlayerMax * pLevelForceInfo->mCreateMaxRoom )
		{
			++pLevelForceInfo->mTeamCnt[teamType];

			pList->mList[i].mSelection = true;
		}
	}
	pList->mCount = select->rowCount;

	/// 참여 가능자와 불가능자 목록을 게임서버로 발송
	NETWORK2->PostPVPPlayerList( (char*)pList, pList->GetMsgLength() );

	/// 내부서버간 통신용 메모리 해제
	perIoContext->offset = pList->GetMsgLength();
	NETWORK2->ReleaseIoContext( perIoContext, "cPVPManager::TeamDivision ReleaseIoContext" );
}

unsigned short cPVPManager::JoinDeathMatch( cPlayer* player, float* x, float* y )
{
	if( player == 0 )
	{
		NETWORK2->PostServerEvent( "cPVPManager::JoinDeathMatch player is null" );
		return 0;
	}

	if( x == NULL || y == NULL )
	{
		NETWORK2->PostServerEvent( "cPVPManager::JoinDeathMatch x,y is null" );	
		return 0;
	}

	/// gm툴 입장은 방생성없이 강제 마지막맵 진입
	if( IsGmPlayer( player->GetObjectID() ) == true )
	{
		player->SetPvPDMTeam( ePVPDM_TEAMTYPE_MAX );
		unsigned long posIdx = GetTeamGenPosIdx( ePVPDM_TEAMTYPE_A );
		if( posIdx == 0 )
		{
			NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch GM posIdx == 0");
		}
		else
		{
			if ( STAGESCRIPT->CalcTargetMapPos( posIdx, x, y ) == false )
				NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch GM if( STAGESCRIPT->CalcTargetMapPos( %d, %.0f, %.0f ) == false", posIdx, x, y);
		}

		return INDUN_MAP_MAX;
	}

	sLevelForceInfo* pInfo = (sLevelForceInfo*)mLevelForceMap.GetAt( player->GetLevel() );
	if( pInfo == NULL )
	{
		NETWORK2->PostServerEvent( "cPVPManager::JoinDeathMatch pInfo is null(%d)", player->GetLevel() );	
		return 0;
	}

	/// 생성된 방에 넣어본다.
	unsigned int roomCnt = pInfo->mPvPObjectIdxAry.GetSize();
	for( unsigned int i=0; i<roomCnt; ++i )
	{
		cBaseDeathMatch* pObj = PVPOBJECTPOOL->GetPvPObject( &mPvPObjectRoot, pInfo->mPvPObjectIdxAry[i] );
		if( pObj == NULL )
			continue;

		/// 플레이어가 입장 가능하면 입장 후 리턴
		if( pObj->JoinTeam( player ) == 0 )
		{
			ePVPDM_TEAM_TYPE type = (ePVPDM_TEAM_TYPE)player->GetPvPDMTeam();
			unsigned long posIdx = GetTeamGenPosIdx( type );
			if( posIdx == 0 )
			{
				NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch posIdx == 0");
			}
			else
			{
				if ( STAGESCRIPT->CalcTargetMapPos( posIdx, x, y ) == false )
					NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch if( STAGESCRIPT->CalcTargetMapPos( %d, %.0f, %.0f ) == false", posIdx, x, y);
			}

			return (unsigned short)pInfo->mPvPObjectIdxAry[i];
		}
	}

	/// 신규방 생성
	if( mDMMaxCnt >= INDUN_MAP_MAX )
	{
		NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch (mDMMaxCnt > %d)", INDUN_MAP_MAX );
		return 0;
	}

	cBaseDeathMatch* newPvPObj = 0;
	switch( NETWORK2->GetInDunMapNum() )
	{
	case MAP_DEATHMATCH:
		{
			cDeathMatchObject* pDm = new cDeathMatchObject;
			newPvPObj = (cBaseDeathMatch*)pDm;
		}
		break;
	case MAP_FLAG:
		{
			cDeathMatch2Object* pDm = new cDeathMatch2Object;
			newPvPObj = (cBaseDeathMatch*)pDm;
		}
		break;
	default:
		NETWORK2->PostServerEvent( "cPVPManager::JoinDeathMatch mPvPType error(%d)", NETWORK2->GetInDunMapNum() );	
		return 0;
	}

	++mDMMaxCnt;
	unsigned short dmIdx = mDMMaxCnt;

	if( newPvPObj->Init( dmIdx, pInfo ) == false )
	{
		PVPOBJECTPOOL->ReleasePvPObject( &mPvPObjectRoot, dmIdx );
		NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch pDMObject->Init( %d ) == false", dmIdx );
		return 0;
	}

	int retcode = newPvPObj->JoinTeam( player );
	if( retcode != 0 )
	{
		PVPOBJECTPOOL->ReleasePvPObject( &mPvPObjectRoot, dmIdx );
		NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch pDMObject->JoinTeam DMIdx(%d,%d)", dmIdx, retcode );
		return 0;
	}

	ePVPDM_TEAM_TYPE type = (ePVPDM_TEAM_TYPE)player->GetPvPDMTeam();
	unsigned long posIdx = GetTeamGenPosIdx( type );
	if( posIdx == 0 )
	{
		NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch posIdx == 0");
	}
	else
	{
		if ( STAGESCRIPT->CalcTargetMapPos( posIdx, x, y ) == false )
			NETWORK2->PostServerEvent("cPVPManager::JoinDeathMatch if( STAGESCRIPT->CalcTargetMapPos( %d, %.0f, %.0f ) == false", posIdx, x, y);
	}

	/// 생성된 객체를 레벨구간별 목록에도 넣어준다.
	pInfo->mPvPObjectIdxAry.PushBack( dmIdx );

	return dmIdx;
}


void cPVPManager::DelAryInsertPVP( unsigned long pvpDMIdx )
{
	mDelDMAry.PushBack( pvpDMIdx );
}


void cPVPManager::MemberOut( unsigned long pvpIdx, unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType )
{
	cBaseDeathMatch* pPvPDM = GetPvPDMObject( pvpIdx );
	if( pPvPDM != NULL )
	{
		pPvPDM->OutMember( playerIdx, teamType );
	}
}

unsigned long cPVPManager::GetTeamPlayerMax()
{
	if( mpDMInfo )
		return mpDMInfo->mTeamPlayerMax;
	return 0;
}

unsigned long cPVPManager::GetTeamPlayerMin()
{
	if( mpDMInfo )
		return mpDMInfo->mTeamPlayerMin;
	return 0;
}


void cPVPManager::AddReadyPlayer( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	if( pPlayer->GetPvPJoinAryPos() != ULONG_MAX )
		return;
	
	for( unsigned long i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
	{
		if( pPlayer->GetLevel() < mSectionInfo[i].mMinLevel || mSectionInfo[i].mMaxLevel < pPlayer->GetLevel() )
			continue;
		
		pPlayer->SetPvPJoinAryPos( i );
		mSectionInfo[i].mReadyPlayerSet.Insert( pPlayer->GetObjectID() ); 
		return;
	}

	NETWORK2->PostServerEvent( "cPVPManager::AddReadyPlayer[%d]", pPlayer->GetObjectID() ); 
}

void cPVPManager::DelReadyPlayer( cPlayer* pPlayer )
{ 
	if( pPlayer == NULL )
		return;

	unsigned long aryPos = pPlayer->GetPvPJoinAryPos();
	if( aryPos >= PVP_DM_LEVEL_SECTION_MAX )
	{
		NETWORK2->PostServerEvent( "cPVPManager::DelReadyPlayer[%d]", aryPos );
		return;
	}

	pPlayer->SetPvPJoinAryPos( ULONG_MAX );

	mSectionInfo[aryPos].mReadyPlayerSet.Erase( pPlayer->GetObjectID() ); 
}


void cPVPManager::SendNotPvPIn( unsigned long aryPos )
{
	if( aryPos >= PVP_DM_LEVEL_SECTION_MAX )
		return;

	cHashSet::cIterator b = mSectionInfo[aryPos].mReadyPlayerSet.Begin();
	cHashSet::cIterator e = mSectionInfo[aryPos].mReadyPlayerSet.End();
	for( ; b != e ; ++b )
	{
		unsigned long playerIdx = *b;
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
		if( pPlayer == NULL )
			continue;

		if( pPlayer->GetPvPJoinAryPos() == ULONG_MAX )
			continue;

		pPlayer->SetPvPJoinAryPos( ULONG_MAX );

		/// 입장 불가메세지 발송
		if( pPlayer->GetGameIn() == true )
			NETWORK2->SendMsgError( pPlayer->GetConnectionIdx( ), NM_PVP, NM_PVP_JOIN_SYN, 99 );
	}

	mSectionInfo[aryPos].mReadyPlayerSet.Clear();
}


bool cPVPManager::IsGmPlayer( unsigned long playerIdx )
{
	cHashSet::cIterator b = mGmPlayer.Find( playerIdx );
	cHashSet::cIterator e = mGmPlayer.End();

	if( b == e )
		return false;

	mGmPlayer.Erase( playerIdx );

	return true;
}


unsigned long cPVPManager::GetTeamGenPosIdx( ePVPDM_TEAM_TYPE teamType )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		return 0;
	}

	return mTeamGenPos[teamType];
}

//void cPVPManager::SendReadyLeftTime( cPlayer* pPlayer )
//{
//	if( pPlayer == NULL )
//		return;
//
//	/// 입장후 전투 시작까지 남은시간 발송
//	if( mProcess == ePVPPROCESS_JOINREADY )
//	{		
//		/// 남은시간 계산
//		unsigned long leftTime = 0;
//		if( mNextEventTime > NETWORK2->GetAccumTime() )
//			leftTime = mNextEventTime - NETWORK2->GetAccumTime();
//
//		/// 전투시작까지 남은시간 발송
//		HANDLE handle  = NULL;
//		MSG_SYN_PVP_JOINTIME* sendMsg = (MSG_SYN_PVP_JOINTIME*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ), 
//			NM_PVP, NM_PVP_DM_JOINTIME_SYN );
//		if ( sendMsg != NULL )
//		{
//			sendMsg->mFightLeftTime = leftTime;
//			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PVP_JOINTIME) );
//		}		
//
//		return;
//	}
//	/// 입장후 이미 전투가 시작했으면 시작을 알림
//	else if( mProcess == ePVPPROCESS_RUN )
//	{
//		cBaseDeathMatch* pDM = GetPvPDMObject( pPlayer->GetPvPDMIdx() );
//		if( pDM != NULL && pDM->SendFight( pPlayer ) == true )
//			return;
//	}
//
//	/// 전투진행 불가를 알림
//	NETWORK2->SendMsgRoot( pPlayer->GetConnectionIdx( ), NM_PVP, NM_PVP_DM_NOTREADY_SYN );
//}


unsigned long cPVPManager::GetSectionAryPos( unsigned char level )
{
	for( unsigned long i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
	{
		if( mSectionInfo[i].mMinLevel <= level && level <= mSectionInfo[i].mMaxLevel )
			return i;
	}

	return ULONG_MAX;
}


sSecionInfo* cPVPManager::GetSectionInfo( unsigned long aryPos )
{
	if( aryPos < PVP_DM_LEVEL_SECTION_MAX )
		return &mSectionInfo[aryPos];

	return NULL;
}

void cPVPManager::SendMsgSynPvPReadyClose( unsigned long aryPos )
{
	if( aryPos >= PVP_DM_LEVEL_SECTION_MAX )
		return;

	cHashSet::cIterator b = mSectionInfo[aryPos].mReadyPlayerSet.Begin();
	cHashSet::cIterator e = mSectionInfo[aryPos].mReadyPlayerSet.End();
	for( ; b != e ; ++b )
	{
		unsigned long playerIdx = *b;
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
		if( pPlayer == NULL )
			continue;

		if( pPlayer->GetPvPJoinAryPos() == ULONG_MAX )
			continue;

		/// 입장 불가메세지 발송
		if( pPlayer->GetGameIn() == true )
			NETWORK2->SendMsgRoot( pPlayer->GetConnectionIdx( ), NM_PVP, NM_PVP_READYCLOSE_SYN );
	}
}
