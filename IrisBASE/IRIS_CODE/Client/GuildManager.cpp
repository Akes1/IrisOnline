#include "stdafx.h"
#include "GuildManager.h"
#include "GameResourceManager.h"
#include "ChatManager.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "UIMsgboxEvent.h"
#include "Npc_Common.h"
#include "GuildWindow.h"
#include "ImageReader.h"
#include "EngineFile.h"
#include "FileSystem.h"

cGuildManager* cGuildManager::mpSingleton = 0;

cGuildManager::cGuildManager()
: mConnectCount(0)
, mTempNpcIndex(0)
, mTempCutPlayerIdx(0)
, mpPreviewMark(0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	::memset( &mGuildInfo, 0, sizeof(mGuildInfo) );
	::memset( &mMarkBuffer, 0, sizeof(mMarkBuffer) );

	mGuildInfoInit = false;
}

cGuildManager::~cGuildManager()
{
	{
		cGuildUserMap::cIterator i = mGuildUserMap.Begin();
		cGuildUserMap::cIterator end = mGuildUserMap.End();
		for( ; i != end; ++i )
		{
			sGuildUserInfo* p = (sGuildUserInfo*)(i->mSecond);
			SAFE_DELETE( p );
		}
		mGuildUserMap.Clear();
	}

	mMarkTextureMap.Clear();

	/// 
	mRequestMarkSet.Clear();

	///
	mpSingleton = 0;
}

bool cGuildManager::Init()
{
	return CheckGuildMarkFolder();
}

void cGuildManager::Open()
{
	Clear();
}

void cGuildManager::Close()
{
	Clear();
}

// 길드에서 나감
void cGuildManager::Clear()
{
	if( HERO )
		HERO->ClearGuild();

	//
	cGuildWindow* guildWindow = GAMEUI->GetGuildWindow();
	if( guildWindow )
	{
		guildWindow->Clear();
		guildWindow->HideMark();
	}

	//
	::memset( &mGuildInfo, 0, sizeof(mGuildInfo) );
	mConnectCount = 0;
	
	/// 
	cGuildUserMap::cIterator i = mGuildUserMap.Begin();
	cGuildUserMap::cIterator end = mGuildUserMap.End();
	for( ; i != end; ++i )
	{
		sGuildUserInfo* p = (sGuildUserInfo*)(i->mSecond);
		if( p )
		{
			if( HERO->GetObjectID() == p->characterIdx )
				continue;

			cPlayer* player = OBJECTMANAGER->GetPlayer( p->characterIdx );
			if( player )
			{
				player->UpdateNameCardTextValue();
			}
		}

		SAFE_DELETE( p );
	}

	mGuildUserMap.Clear();

	ClearTempData();

	mTempCutPlayerIdx = 0;

	//
	mpPreviewMark = 0;
}

void cGuildManager::ClearTempData()
{
	mTempName.Clear();

	mTempNpcIndex = 0;
}

bool cGuildManager::CheckGuildMarkFolder()
{
	if( ::FileExist( "GuildMark" ) == false )
	{
		::CreateDirectory( L"GuildMark", 0 );
	}
	return true;
}

void cGuildManager::ResGuildCreate( MSG_RES_NPC_GUILD_CREATE* pmsg )
{
	cStringT str;

	switch( pmsg->ErrorCode )
	{
	case ERROR_NPC_GUILD_CREATE_SUCESS:			// 성공
		str.Format( GAMERESOURCEMAN->GetGameText( 1200 ), (LPCTSTR)mTempName.Cstr() );

		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_NONE, eSKIN_OK,
							(LPCTSTR)str.Cstr(),
							GAMERESOURCEMAN->GetGameText( 301 )	// 알림
							);	

		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		break;
	case ERROR_NPC_GUILD_CREATE_FAIL:			// 실패
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 1201 ), 
							GAMERESOURCEMAN->GetGameText(301)
							);
		break;
	case ERROR_NPC_GUILD_CREATE_CONDITION:		// 실패 - 가입조건 안됨
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 1232 ), 
							GAMERESOURCEMAN->GetGameText(301)
							);
		break;
	case ERROR_NPC_GUILD_CREATE_ALREADY:		// 실패 - 이미 길드 가입중
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1202 ), 
			GAMERESOURCEMAN->GetGameText(301)
			);
		break;
	case ERROR_NPC_GUILD_CREATE_ERRORNAME:		// 실패 - 길드명 없거나 12자 넘음
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1203 ), 
			GAMERESOURCEMAN->GetGameText(301)
			);
		break;
	case ERROR_NPC_GUILD_CREATE_EXISTNAME:		// 실패 - 길드명 중복
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1204 ), 
			GAMERESOURCEMAN->GetGameText(301)
			);
		break;
	case ERROR_NPC_GUILD_CREATE_NPC:	// 실패 - npc인증
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 420 ), 
			GAMERESOURCEMAN->GetGameText(301)
			);
		break;
	}

	ClearTempData();
}

void cGuildManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	cStringT str;
	switch( tempMsg->Protocol )
	{
	case NM_GUILD_LIST_SYN:
		{
			MSG_SYN_GUILD_LIST* pmsg = (MSG_SYN_GUILD_LIST*)msg;

			/// 초기화
			Clear();

			/// 세팅
			mGuildInfo = pmsg->data;

			sGuildUserInfo* user = pmsg->table;
			for( unsigned char i = 0; i < pmsg->rowCount; ++i, ++user )
			{
				AddGuildUser( user );
			}

			sGuildUserInfo* info = (sGuildUserInfo*)mGuildUserMap.GetAt( HERO->GetObjectID() );
			if( info )
			{
				/// 기본으로 채널 세팅
				info->channelNum = HERO->GetChannelNum();
				info->mapnum	 = (short)HERO->GetMapNum();

				HERO->SetGuild( mGuildInfo.idx, info->position, mGuildInfo.name, mGuildInfo.markRegistDate, mGuildInfo.useMark );
			}

			// 각 길드유저의 텍스트 갱신
			cGuildUserMap::cIterator i = mGuildUserMap.Begin();
			cGuildUserMap::cIterator end = mGuildUserMap.End();
			for( ; i != end; ++i )
			{
				sGuildUserInfo* playerInfo = (sGuildUserInfo*)(i->mSecond);
				if( playerInfo && playerInfo->characterIdx != HERO->GetObjectID() )
				{
					cPlayer* player = OBJECTMANAGER->GetPlayer( playerInfo->characterIdx );
					if( player )
					{
						player->UpdateNameCardTextValue();
					}
				}
			}

			/// UI 갱신
			UpdateInfoSheet();

			///
			int len = ::_tcslen( mGuildInfo.notice );
			if( len > 0 )
			{
				if( mGuildInfoInit == false )
				{
					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( 1251 ), mGuildInfo.notice );
					CHATMANAGER->AddChatMsg( (LPCTSTR)str.Cstr(), eCHAT_GUILD_NOTICE  );
				}

				///
				cGuildWindow* win = GAMEUI->GetGuildWindow();
				if( win )
					win->UpdateGuildNotice( mGuildInfo.notice );
			}
			
			if( mGuildInfo.useMark == true )
			{
				/// 길드윈도우에 마크등록
				NiTexture* tex = CheckMarkVersion( mGuildInfo.idx, mGuildInfo.markRegistDate );
				if( tex )
				{
					cGuildWindow* win = GAMEUI->GetGuildWindow();
					if( win )
						win->ChangeMark( tex );
				}
				else
				{
					// 재요청
					GuildMarkRequest( mGuildInfo.idx );
				}
			}

			mGuildInfoInit = true;
		}
		break;
	case NM_GUILD_CREATEDATA_RES:
		{	
			/// 길드 생성
			MSG_RES_GUILD_CREATEDATA* pmsg = (MSG_RES_GUILD_CREATEDATA*)msg;

			/// 초기화
			Clear();

			/// 1. 길드 데이터 세팅
			mGuildInfo.idx = pmsg->idx;
			mGuildInfo.masterIndex = HERO->GetObjectID();
			mGuildInfo.position = eGUILD_MASTER;
			mConnectCount = 1;
			::Sstrncpy( mGuildInfo.name, MAX_GUILDNAME_BUFFER_SIZE, pmsg->name, MAX_GUILDNAME_SIZE );
			::Sstrncpy( mGuildInfo.masterName, MAX_NAME_BUFFER_SIZE, HERO->GetName(), MAX_NAME_SIZE );

			/// 2. 유저리스트 세팅
			sGuildUserInfo* userInfo = new sGuildUserInfo;
			userInfo->characterIdx = HERO->GetObjectID();
			userInfo->level = HERO->GetLevel();
			userInfo->job = (unsigned short)HERO->GetJob();
			userInfo->mapnum = (short)HERO->GetMapNum();
			userInfo->position = eGUILD_MASTER;
			userInfo->channelNum = HERO->GetChannelNum();
			::Sstrncpy( userInfo->name, MAX_NAME_BUFFER_SIZE, HERO->GetName(), MAX_NAME_SIZE );

			if( mGuildUserMap.Insert( userInfo->characterIdx, userInfo ) == false )
			{
				assert( 0 && "failed to insert guild user" );
			}

			/// UI 세팅
			UpdateInfoSheet();
			
			/// 
			HERO->SetGuild( mGuildInfo.idx, mGuildInfo.position, mGuildInfo.name, mGuildInfo.markRegistDate, false );
		}
		break;
	case NM_GUILD_ADD_RES:
		{
			// 길드초대 결과 ( 취소가능하게 해주어야함 )
			MSG_RES_GUILD_ADD* pmsg = (MSG_RES_GUILD_ADD*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_GUILD_ADD_SUCCESS:		// 성공 - 길드 신청 성공
				{
					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( 1216 ), (LPCTSTR)mTempAddName.Cstr() );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
				}
				break;
			case ERROR_GUILD_ADD_FAIL:			// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1206 ) );
				break;	
			case ERROR_GUILD_ADD_REQUEST:		// 실패 - 이미 신청중임
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1207 ) );
				break;
			case ERROR_GUILD_ADD_NOTEXIST:		// 실패 - 상대가 존재하지 않음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1208 ) );
				break;
			case ERROR_GUILD_ADD_HAVEGUILD:		// 실패 - 상대가 이미 길드 가입중
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1209 ) );
				break;
			case ERROR_GUILD_ADD_MAXGUILD:		// 실패 - 길드 인원 제한
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1210 ) );
				break;
			case ERROR_GUILD_ADD_REFUSE:		// 실패 - 상대방 가입 거절
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1218 ) );
				break;
			case ERROR_GUILD_ADD_REFUSETIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1269 ) );
				break;
			case ERROR_GUILD_ADD_STATUSHERO:	// 실패 - 신청가능 상태 아님
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1267 ) );
				break;
			case ERROR_GUILD_ADD_STATUS:		// 실패 - 상대방이 신청받기 가능 상태 아님
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1268 ) );
				break;
			}

			mTempAddName.Clear();
		}
		break;
	case NM_GUILD_ADD_SYN:
		{
			/// 다른 유저로부터 길드초대 받음
			MSG_SYN_GUILD_ADD* pmsg = (MSG_SYN_GUILD_ADD*)msg;
			
			str.Format( GAMERESOURCEMAN->GetGameText( 1215 ), pmsg->name, pmsg->guildName );

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_GUILD_ADD, eSKIN_YESNO,
				(LPCTSTR)str.Cstr(),					// 길드요청 메시지
				GAMERESOURCEMAN->GetGameText( 1214 ),	// 길드 요청
				0, 0, GUILD_WAIT_TIME, false, false );
		}
		break;
	case NM_GUILD_REPLY_RES:
		{
			/// 길드초대 답변에 대한 결과
			MSG_RES_GUILD_REPLY* pmsg = (MSG_RES_GUILD_REPLY*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_GUILD_ADDREPLY_SUCCESS:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1211 ) );
				break;
			case ERROR_GUILD_ADDREPLY_FAIL:
			case ERROR_GUILD_ADDREPLY_EXIST:
			case ERROR_GUILD_ADDREPLY_FAILUPDATE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1212 ) );
				break;
			case ERROR_GUILD_ADDREPLY_REFUSE:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1213 ) );
				break;
			case ERROR_GUILD_ADDREPLY_REFUSETIME:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1269 ) );
				break;
			}
		}
		break;
	case NM_GUILD_USERADD_SYN:
		{
			/// 추가된 길드유저 정보
			MSG_SYN_GUILD_USERADD* pmsg = (MSG_SYN_GUILD_USERADD*)msg;

			// 길드 유저 추가
			AddGuildUser( &pmsg->user );

			// UI 갱신
			UpdateInfoSheetByUser();

			//
			str.Format( GAMERESOURCEMAN->GetGameText( 1217 ), &pmsg->user.name );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, str.Cstr() );
		}
		break;
	case NM_GUILD_OUT_RES:
		{
			/// 길드 탈퇴 
			MSG_RES_GUILD_OUT* pmsg = (MSG_RES_GUILD_OUT*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_GUILD_OUT_SUCCESS:
				{
					// 탈퇴 성공
					Clear();

					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1219 ) );
				}
				break;
			case ERROR_GUILD_OUT_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1220 ) );
				break;
			case ERROR_GUILD_OUT_MASTER:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1221 ) );
				break;
			}
		}
		break;
	case NM_GUILD_OUT_SYN:
		{
			/// 길드 탈퇴 
			MSG_SYN_GUILD_OUT* pmsg = (MSG_SYN_GUILD_OUT*)msg;

			cGuildUserMap::cIterator i = mGuildUserMap.Find( pmsg->characterIdx );
			if( i != mGuildUserMap.End() )
			{
				sGuildUserInfo* user = (sGuildUserInfo*)(i->mSecond);
				
				// 탈퇴 알림
				str.Format( GAMERESOURCEMAN->GetGameText( 1222 ), user->name );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

				// 삭제
				SAFE_DELETE( user );
				mGuildUserMap.Erase( i );

				// UI 갱신
				UpdateInfoSheetByUser();
			}
		}
		break;
	case NM_GUILD_CUT_RES:
		{
			/// 길드원 추방요청 결과
			MSG_RES_GUILD_CUT* pmsg = (MSG_RES_GUILD_CUT*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_GUILD_CUT_SUCCESS:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1256 ) );
				break;
			case ERROR_GUILD_CUT_FAIL:		// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1223 ) );
				break;
			case ERROR_GUILD_CUT_MASTER:	// 실패 - 길드장만 추방가능
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1224 ) );
				break;
			}

			mTempCutPlayerIdx = 0;
		}
		break;
	case NM_GUILD_CUT_SYN:
		{
			/// 자기자신이 추방됨
			Clear();

			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1225 ) );
		}
		break;
	case NM_GUILD_USERCUT_SYN:
		{
			/// 길드원이 추방됨
			MSG_SYN_GUILD_USERCUT* pmsg = (MSG_SYN_GUILD_USERCUT*)msg;

			cGuildUserMap::cIterator i = mGuildUserMap.Find( pmsg->characterIdx );
			if( i != mGuildUserMap.End() )
			{
				sGuildUserInfo* user = (sGuildUserInfo*)(i->mSecond);

				// 탈퇴 알림
				str.Format( GAMERESOURCEMAN->GetGameText( 1226 ), user->name );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );

				// 삭제
				SAFE_DELETE( user );
				mGuildUserMap.Erase( i );

				// UI 갱신
				UpdateInfoSheetByUser();
			}
		}
		break;
	case NM_GUILD_DELETE_RES:
		{
			MSG_RES_GUILD_DELETE* pmsg = (MSG_RES_GUILD_DELETE*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_GUILD_DELETE_SUCCESS:
				{
					// 길드해체 성공
					Clear();

					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1230 ) );
				}
				break;
			case ERROR_GUILD_DELETE_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1229 ) );
				break;
			case ERROR_GUILD_DELETE_MASTER:		// 실패 - 길드장만 해체 가능
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1227 ) );
				break;
			case ERROR_GUILD_DELETE_USEREXIST:	// 실패 - 길드유저가 남아있음
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1228 ) );
				break;
			}
		}
		break;
	case NM_GUILD_UPDATE_SYN:
		{
			/// 시간에 의한 길드 정보 갱신
			MSG_SYN_GUILD_UPDATE* pmsg = (MSG_SYN_GUILD_UPDATE*)msg;
			
			// 갱신
			sGuildUserUpdate* data = pmsg->table;
			for( unsigned char i = 0; i < pmsg->rowCount; ++i, ++data )
			{
				sGuildUserInfo* user = (sGuildUserInfo*)mGuildUserMap.GetAt( data->characterIdx );
				if( user )
				{
					user->level  = data->level;
					user->job	 = data->job;
					user->mapnum = data->mapnum;
					user->channelNum = data->channelNum;					
				}
			}
		
			/// UI 갱신
            UpdateInfoSheetByUser();
		}
		break;
	case NM_GUILD_GIVEPOSITION_RES:
		{
			/// 직위변경 요청 결과
			MSG_RES_GUILD_GIVEPOSITION* pmsg = (MSG_RES_GUILD_GIVEPOSITION*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_GUILD_GIVEPOSITION_SUCCESS:
				break;
			case ERROR_GUILD_GIVEPOSITION_FAIL:			/// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1244 ) );
				break;
			case ERROR_GUILD_GIVEPOSITION_POSITION:		/// 권한
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1245 ) );
				break;
			}
		}
		break;
	case NM_GUILD_GIVEPOSITION_SYN:
		{
			/// 직위 변경됨
			MSG_SYN_GUILD_GIVEPOSITION* pmsg = (MSG_SYN_GUILD_GIVEPOSITION*)msg;

			/// 본인이 길드장인경우 
			if( pmsg->position == eGUILD_MASTER && HERO->GetObjectID() == mGuildInfo.masterIndex )
			{
				mGuildInfo.position = eGUILD_GENERAL;
			}
			else if( pmsg->characterIdx == HERO->GetObjectID() )
			{
				mGuildInfo.position = pmsg->position;
			}
	
			/// 길드 유저 리스트 갱신
			sGuildUserInfo* newInfo = (sGuildUserInfo*)mGuildUserMap.GetAt( pmsg->characterIdx );
			if( newInfo )
			{
				newInfo->position = pmsg->position;

				/// 새로운 길드장일 경우 추가 처리
				if( pmsg->position == eGUILD_MASTER )
				{
					/// 기존 길드장의 직위 변경
					sGuildUserInfo* userInfo = (sGuildUserInfo*)mGuildUserMap.GetAt( mGuildInfo.masterIndex );
					if( userInfo )
					{
						userInfo->position = eGUILD_GENERAL;
					}
					else
					{
						assert(0);
					}

					/// 새 길드장 정보 세팅
					mGuildInfo.masterIndex = pmsg->characterIdx;
					::Sstrncpy( mGuildInfo.masterName, MAX_NAME_BUFFER_SIZE, newInfo->name, MAX_NAME_SIZE );
				}

				/// 변경정보 알림
				str.Format( GAMERESOURCEMAN->GetGameText( 1246 ), newInfo->name, GetTextIndex( pmsg->position ) );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
			}
			else
			{
				assert(0);
			}

			/// UI 세팅
			UpdateInfoSheet();

			//
			cGuildWindow* win = GAMEUI->GetGuildWindow();
			if( win )
				win->UpdateGuildNotice( mGuildInfo.notice );

			///
			HERO->SetGuildPosition( mGuildInfo.position );
		}
		break;
	case NM_GUILD_NOTICE_RES:
		{
			MSG_RES_GUILD_NOTICE* pmsg = (MSG_RES_GUILD_NOTICE*)msg;

			if( pmsg->ErrorCode == ERROR_GUILD_NOTICE_FAIL )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1250 ) );
			}
			else if( pmsg->ErrorCode == ERROR_GUILD_NOTICE_MASTER )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1265 ) );
			}
		}
		break;
	case NM_GUILD_NOTICE_SYN:
		{
			MSG_SYN_GUILD_NOTICE* pmsg = (MSG_SYN_GUILD_NOTICE*)msg;

			/// 저장
			::Sstrncpy( mGuildInfo.notice, MAX_GUILD_NOTICE_BUFFER_SIZE, pmsg->notice, MAX_GUILD_NOTICE_SIZE );

			/// UI출력
			UpdateNotice();
		}
		break;
	case NM_GUILDMARK_UPDATE_RES:
		{
			/// 길드마크 변경 결과
			MSG_RES_GUILDMARK_UPDATE* pmsg = (MSG_RES_GUILDMARK_UPDATE*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_GUILDMARK_UPDATE_FAIL:			// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1252 ) );
				break;
			case ERROR_GUILDMARK_UPDATE_POSITION:		// 권한실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1253 ) );
				break;
			}

			//
			mpPreviewMark = 0;
		}
		break;
	case NM_GUILDMARK_SEND_RES:
		{
			MSG_RES_GUILDMARK_SEND* pmsg = (MSG_RES_GUILDMARK_SEND*)msg;

			/// 새로운 길드마크 정보 받음
			if( pmsg->ErrorCode == ERROR_GUILDMARK_SEND_SUCCESS )
			{
				/// 길드마크 받아서 새 파일로 생성
				CheckGuildMarkFolder();
			
				/// 새 길드마크 파일 생성( 버전 저장 )
				SaveGuildMark( pmsg->guildIndex, pmsg->registDate, pmsg->mark );
				
				NiTexture* tex = CheckMarkVersion( pmsg->guildIndex, pmsg->registDate );
				if( tex )
				{
					/// 길드마크 갱신
					OBJECTMANAGER->ChangeGuildMarkRegistDate( pmsg->guildIndex, pmsg->registDate );

					/// 길드 UI에 등록
					if( pmsg->guildIndex == HERO->GetGuildIndex() )
					{
						HERO->ChangeGuildMarkRegistDate( pmsg->registDate, true );

						cGuildWindow* win = GAMEUI->GetGuildWindow();
						if( win )
							win->ChangeMark( tex );
						else
							assert(0);
					}
				}
				else
				{
					assert(0);
				}
			}

			/// 요청 맵으로부터 삭제
			mRequestMarkSet.Erase( pmsg->guildIndex );
		}
		break;
	case NM_GUILDUSER_CONNECT_SYN:
		{
			MSG_SYN_GUILDUSER_CONNECT* pmsg = (MSG_SYN_GUILDUSER_CONNECT*)msg;
			
			sGuildUserInfo* pInfo = (sGuildUserInfo*)mGuildUserMap.GetAt( pmsg->characterIndex );
			if( pInfo != NULL )
			{
				cStringT str;
				unsigned long infoIndex = CHATMANAGER->GetInfoTextIndex(eCHAT_SYSTEM);				

				CHATMANAGER->AddChatMsgEx( (LPCTSTR)GAMERESOURCEMAN->GetGameText( 11100 ), COLOR_WHITE, infoIndex, false, true );
				CHATMANAGER->AddChatMsgEx( (LPCTSTR)pInfo->name, COLOR_GUILD_EX, 0, true, true );

				unsigned long textIndex = ( pmsg->connect ) ? 11103 : 11104;
				CHATMANAGER->AddChatMsgEx( (LPCTSTR)GAMERESOURCEMAN->GetGameText( textIndex ), COLOR_WHITE, 0, true, true );

				// 정보 저장
				pInfo->channelNum = pmsg->channelNum;

				/// UI 갱신
				UpdateInfoSheetByUser();
			}
			else
				assert(0);
		}
		break;
	}
}

void cGuildManager::AddGuildUser( sGuildUserInfo* user )
{
	sGuildUserInfo* userInfo = new sGuildUserInfo;

	/// 복사
	*userInfo = *user;

	if( mGuildUserMap.Insert( userInfo->characterIdx, userInfo ) == false )
	{
		assert( 0 && "failed to insert guild user" );
	}
}

void cGuildManager::SetTempData( LPCTSTR name, unsigned long npcIdx )
{
	mTempName = name;
	mTempNpcIndex = npcIdx;
}

/// 길드 생성 요청
bool cGuildManager::GuildCreate()
{
	if( HERO->GetGuildIndex() > 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1202 ) );
		return false;
	}

	// 길이 검사 
	if( mTempName.IsEmpty() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1203 ) );
		return false;
	}

	// npc 인증실패
	if( mTempNpcIndex == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 420 ) );
		return false;
	}

	MSG_REQ_NPC_GUILD_CREATE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_GUILD_CREATE_REQ;
	msg.npcIndex = mTempNpcIndex;
	::Sstrncpy( msg.name, MAX_GUILDNAME_BUFFER_SIZE, (LPCTSTR)mTempName.Cstr(), MAX_GUILDNAME_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

/// 길드 초대 요청
void cGuildManager::GuildAdd( cStringT name )
{
	if( HERO->GetGuildIndex() == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
		return;
	}

	if( name.IsEmpty() )
	{
		cBaseObject* pTarget = HERO->GetTargetObject();
		if( pTarget && pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			name = pTarget->GetName();
		}
		else
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 102 ) );
			return;
		}
	}

	/// 자기자신 초대불가
	if( ::_tcscmp( (LPCTSTR)name.Cstr(), HERO->GetName() ) == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
		return;
	}

	/// 초대 직위 검사
	char heroPos = HERO->GetGuildPosition();
	if( !(heroPos == eGUILD_MASTER || heroPos == eGUILD_SUBMASTER || heroPos == eGUILD_ELDER ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1266 ) );
		return;
	}

	///
	mTempAddName = name;

	///
	MSG_REQ_GUILD_ADD msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILD_ADD_REQ;
	::Sstrncpy( msg.name, MAX_NAME_BUFFER_SIZE, (LPCTSTR)name.Cstr(), MAX_NAME_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

/// 길드 탈퇴 요청
void cGuildManager::GuildOut()
{
	if( HERO->GetGuildPosition() == eGUILD_NONE )
	{
		assert(0);
		return;
	}
	if( HERO->GetGuildPosition() == eGUILD_MASTER )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1221 ) );
		return;
	}

	MSG_REQ_GUILD_OUT msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILD_OUT_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

/// 길드 추방 요청
void cGuildManager::GuildCut( unsigned long characterIdx )
{
	sGuildUserInfo* info = (sGuildUserInfo*)mGuildUserMap.GetAt( characterIdx );
	if( info != NULL )
	{
		mTempCutPlayerIdx = characterIdx;

		MSG_REQ_GUILD_CUT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_GUILD;
		msg.Protocol = NM_GUILD_CUT_REQ;
		msg.characterIdx = characterIdx;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	else
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1224 ) );
	}
}

///
void cGuildManager::GuildDelete()
{
	// 길드원 남아있는지 검사
	if( mGuildUserMap.GetSize() > 1 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1228 ) );
		return;
	}

	MSG_REQ_GUILD_DELETE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILD_DELETE_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

///
void cGuildManager::GuildGivePosition( unsigned long characterIdx, unsigned char position, unsigned char newPosition )
{
	if( HERO->GetGuildIndex() == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
		return;
	}
	
	char heroPos = HERO->GetGuildPosition();
	if( !(heroPos == eGUILD_MASTER || heroPos == eGUILD_SUBMASTER) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1245 ) );
		return;
	}
	
	/// 길드장이 아닌경우
	/// 해당 유저의 직위가 자신과 같거나 높을수 없다
	/// 변경할 직위가 자신과 같거나 높을수 없다
	if( heroPos != eGUILD_MASTER )
	{
		if( !(heroPos < position && heroPos < newPosition) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1245 ) );
			return;
		}
	}
	else if( heroPos == eGUILD_MASTER )
	{
		/// 해당유저가 길드장일수없음
		if( !(heroPos < position) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1245 ) );
			assert(0);
			return;
		}
	}

	/// 요청
	MSG_REQ_GUILD_GIVEPOSITION msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILD_GIVEPOSITION_REQ;
	msg.characterIdx = characterIdx;
	msg.position = newPosition;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

void cGuildManager::GuildNotice( LPCTSTR notice )
{
	if( HERO->GetGuildIndex() == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
		return;
	}

	///
	MSG_REQ_GUILD_NOTICE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILD_NOTICE_REQ;
	::Sstrncpy( msg.notice, MAX_GUILD_NOTICE_BUFFER_SIZE, notice, MAX_GUILD_NOTICE_SIZE );
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

void cGuildManager::GuildMarkUpdate()
{
	// 락걸기

	MSG_REQ_GUILDMARK_UPDATE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDMARK_UPDATE_REQ;
	memcpy( &msg.mark, &mMarkBuffer, MARK_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

void cGuildManager::GuildMarkRequest( unsigned long guildIndex )
{
	/// 서버에 해당 길드 마크 요청
	cRequestMarkSet::cIterator i = mRequestMarkSet.Find( guildIndex );
	if( i == mRequestMarkSet.End() )
	{
		MSG_REQ_GUILDMARK_SEND sendmsg;
		::memset( &sendmsg, 0, sizeof(sendmsg) );
		sendmsg.Category = NM_GUILD;
		sendmsg.Protocol = NM_GUILDMARK_SEND_REQ;
		sendmsg.guildIndex = guildIndex;
		NETWORK->SendNetworkMsg( (char*)&sendmsg, sizeof(sendmsg) );

		/// 요청 맵에 저장
		if( mRequestMarkSet.Insert( guildIndex ) == false )
		{
			assert(0);
		}
	}
}

bool GuildUserSort( const void *arg1, const void *arg2 )
{
	sGuildUserInfo* data1 = (sGuildUserInfo*)arg1;
	sGuildUserInfo* data2 = (sGuildUserInfo*)arg2;

	return (data2->position - data1->position > 0);
}

/// UI 세팅
void cGuildManager::UpdateInfoSheet()
{
	if( mGuildInfo.idx == 0 )
		return;

	typedef tList<void*> cSortList;
	cSortList list;

	cGuildWindow* guildWindow = GAMEUI->GetGuildWindow();
	if( guildWindow )
	{
		mConnectCount = 0;
		guildWindow->Clear();

		/// 길드 정보 세팅
		guildWindow->SetGuildName( mGuildInfo.name );
		guildWindow->SetMasterName( mGuildInfo.masterName );
		guildWindow->SetPosition( mGuildInfo.position );
		guildWindow->SetTotalCount( mGuildUserMap.GetSize() );

		/// 유저리스트 갱신
		cGuildUserMap::cIterator i = mGuildUserMap.Begin();
		cGuildUserMap::cIterator end = mGuildUserMap.End();
		for( ; i != end; ++i )
		{
			sGuildUserInfo* p = (sGuildUserInfo*)(i->mSecond);
			list.PushBack( p );
		}

		if( list.GetSize() > 1 )
			list.Sort( GuildUserSort );

		///
		cSortList::cIterator li = list.Begin();
		cSortList::cIterator lend = list.End();
		for( ; li != lend; ++li )
		{
			sGuildUserInfo* p = (sGuildUserInfo*)(*li);
			guildWindow->AddGuildPlayer( p );

			// 접속인수 계산
			if( p->channelNum > 0 )
				mConnectCount++;
		}

		guildWindow->SetConnectCount( mConnectCount );
	}
}

/// UI 세팅 - 길드원 가입/탈퇴에 따른 갱신
void cGuildManager::UpdateInfoSheetByUser()
{
	typedef tList<void*> cSortList;
	cSortList list;

	cGuildWindow* guildWindow = GAMEUI->GetGuildWindow();
	if( guildWindow )
	{
		mConnectCount = 0;

		guildWindow->SetTotalCount( mGuildUserMap.GetSize() );

		unsigned int selectrow = guildWindow->GetSelectRow();

		/// 유저리스트 갱신
		guildWindow->ClearList();
		cGuildUserMap::cIterator i = mGuildUserMap.Begin();
		cGuildUserMap::cIterator end = mGuildUserMap.End();
		for( ; i != end; ++i )
		{
			sGuildUserInfo* p = (sGuildUserInfo*)(i->mSecond);
			list.PushBack( p );
		}

		if( list.GetSize() > 1)
			list.Sort( GuildUserSort );

		///
		cSortList::cIterator li = list.Begin();
		cSortList::cIterator lend = list.End();
		for( ; li != lend; ++li )
		{
			sGuildUserInfo* p = (sGuildUserInfo*)(*li);
			guildWindow->AddGuildPlayer( p );

			// 접속인수 계산
			if( p->channelNum > 0 )
				mConnectCount++;
		}

		//
		guildWindow->SetConnectCount( mConnectCount );
		guildWindow->SetSelectRow( selectrow );
	}
}

/// 길드 공지 갱신
void cGuildManager::UpdateNotice()
{
	int len = ::_tcslen( mGuildInfo.notice );
	if( len > 0 )
	{
		cStringT str;
		str.Format( GAMERESOURCEMAN->GetGameText( 1251 ), mGuildInfo.notice );
		CHATMANAGER->AddChatMsg( (LPCTSTR)str.Cstr(), eCHAT_GUILD_NOTICE );

		///
		cGuildWindow* win = GAMEUI->GetGuildWindow();
		if( win )
			win->UpdateGuildNotice( mGuildInfo.notice );
	}
}

/// 직위에 따른 텍스트 인덱스 
LPCTSTR cGuildManager::GetTextIndex( unsigned char position )
{
	unsigned int textIdx = 0;
	switch( position )
	{
	case eGUILD_NONE:		textIdx = 1299;		break;		/// 길드장
	case eGUILD_MASTER:		textIdx = 1300;		break;		/// 길드장
	case eGUILD_SUBMASTER:	textIdx = 1301;		break;		/// 부 길드장
	case eGUILD_ELDER:		textIdx = 1302;		break;		/// 장로
	case eGUILD_UPPERHOUSE:	textIdx = 1303;		break;		/// 상원 의원
	case eGUILD_LOWERHOUSE:	textIdx = 1304;		break;		/// 하원 의원
	case eGUILD_GENERAL:	textIdx = 1305;		break;		/// 일반
	default:		
		assert(0);		
		return 0;
	}

	return GAMERESOURCEMAN->GetGameText( textIdx );
}

NiTexture* cGuildManager::CheckMarkVersion( unsigned long guildIndex, tm registDate )
{
	///  해당 길드마크가 존재하는지 검사
	cString str;
	str.Format( "./GuildMark/mark%d.bmp", guildIndex );

	cFileToRead* file = FILESYSTEM->OpenFileToRead_NoPack( str );
	if( file )
	{
		// 존재하면 버전 검사
		MARK_VERSION version;
		::memset( &version, 0, sizeof(MARK_VERSION) );

		file->Read( &version, sizeof(MARK_VERSION) );

		file->Close();
		SAFE_DELETE( file );

		// 버전 정보가 동일하면
		if( version.guildIndex == guildIndex )
		{
			mktime( &version.versionData );
			mktime( &registDate );

			int ret = ::memcmp( &version.versionData, &registDate, sizeof(version.versionData));
			if( ret == 0 )
			{
				// 이미 가지고있는 텍스쳐가 있으면 리턴
				NiTexture* tex = (NiTexture*)mMarkTextureMap.GetAt( guildIndex );
				if( tex )
					return tex;

				// 파일에서 새로 로드해서 존재하면 리턴
				tex = LoadGuildMark( guildIndex );
				if( tex )
					return tex;
			}
		}
	}

	return 0;
}

/// 길드 마크 등록 ui 용 로딩
NiTexture* cGuildManager::LoadGuildMark( unsigned long guildIndex )
{
	CheckGuildMarkFolder();

	cString fileName;
	fileName.Format( "./GuildMark/mark%d.bmp", guildIndex );
	cFileToRead* file = FILESYSTEM->OpenFileToRead_NoPack( fileName );

	/// 1. 파일 유무 검사
	if( !file )
	{
		assert(0);
		return 0;
	}

	///
	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;
	MARK_VERSION version;

	::memset( &bmpFileHeader, 0, sizeof(BITMAPFILEHEADER) );
	::memset( &version, 0, sizeof(MARK_VERSION) );
	
	// 버전정보 가져오기
	file->Read( &version, sizeof(MARK_VERSION) );

	file->Read( &bmpFileHeader, BMP_FILE_HEADER_SIZE );
	file->Read( &bmpInfoHeader, BMP_INFO_HEADER_SIZE );
	
	/// 파일 사이즈 검사 (제한 사이즈 검사)
	unsigned int len = file->GetSize();
	if( len > MARK_MAX_SIZE )
	{
		file->Close();
		SAFE_DELETE( file );
		return 0;
	}

	file->Close();
	SAFE_DELETE( file );

	// 텍스쳐 로딩
	NiTexture* nitex = 0;

	cString pathName;
	pathName.Format( "./GuildMark/mark%d.bmp", guildIndex );

	/// 텍스처 파일을 로딩
	NiPixelDataPtr pixelData = 0;
	NiFilename filename(pathName.Cstr());

	if( _strcmpi( filename.GetExt(), ".bmp" ) == 0 )
	{
		cEngineFile file( pathName.Cstr(), false );
		if( file == false )
			return 0;

		// 일정 헤더사이즈를 넘어서 이미지를 읽어들인다
		file.Seek( sizeof(MARK_VERSION), SEEK_SET );

		cBMPReader bmpReader;
		pixelData = bmpReader.ReadFile( file, 0 );
	}
	else
	{
		assert( 0 );
		return 0;
	}

	if( pixelData == 0 )
	{
		assert( 0 );
		return 0;
	}

	/// 픽셀 데이타로부터 엔진 텍스처를 생성
	NiTexture::FormatPrefs prefs;
	prefs.m_eMipMapped = pixelData->GetNumMipmapLevels() <= 1 ? NiTexture::FormatPrefs::NO : NiTexture::FormatPrefs::YES;

	nitex = NiSourceTexture::Create( pixelData, prefs );
	if( nitex == 0 )
	{
		assert( 0 );
		return 0;
	}

	if( mMarkTextureMap.Insert( guildIndex, nitex ) == false )
	{
		assert( 0 );
	}
	return nitex;
}

void cGuildManager::SaveGuildMark( unsigned long guildIndex, tm registDate, BYTE* mark )
{
	cString fileName;
	fileName.Format( "./GuildMark/mark%d.bmp", guildIndex );
	cFileToWrite* newfile = FILESYSTEM->OpenFileToWrite( fileName.Cstr() );
	if( newfile )
	{
		// 버전 만들기
		MARK_VERSION version;
		::memset( &version, 0, sizeof(MARK_VERSION) );
		version.guildIndex = guildIndex;
		version.versionData = registDate;

		// 새 파일 버퍼
		BYTE tempBuffer[MARK_MAX_SIZE];
		::memset( &tempBuffer, 0, MARK_MAX_SIZE );

		::memcpy( tempBuffer,							&version, sizeof(MARK_VERSION) );	// 버전담기
		::memcpy( tempBuffer + sizeof(MARK_VERSION),	mark,	  MARK_SIZE );				// 데이터 담기

		// 쓰기
		int writeLen = newfile->Write( tempBuffer, MARK_MAX_SIZE );
		if( writeLen > MARK_MAX_SIZE )
		{
			assert(0);
		}

		newfile->Close();
		SAFE_DELETE( newfile );

		/// 메모리에 있던 텍스쳐 삭제
		NiTexture* p = (NiTexture*)mMarkTextureMap.GetAt( guildIndex );
		if( p )
		{
			mMarkTextureMap.Erase( guildIndex );
		}
	}
	else
	{
		assert(0);
	}
}

void cGuildManager::ClearPreviewGuildMark()
{
	mpPreviewMark = 0;
}

NiTexture* cGuildManager::LoadPreviewGuildMark( const cString& pathName )
{
	CheckGuildMarkFolder();

	if( mpPreviewMark != NULL )
	{
		assert(0);
	}

	cFileToRead* file = FILESYSTEM->OpenFileToRead_NoPack( pathName );

	/// 1. 파일 유무 검사
	if( !file )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1270 ) );
		return 0;
	}

	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;

	::memset( &bmpFileHeader, 0, sizeof(BITMAPFILEHEADER) );

	file->Read( &bmpFileHeader, BMP_FILE_HEADER_SIZE );
	file->Read( &bmpInfoHeader, BMP_INFO_HEADER_SIZE );

	/// BMP 파일 형태 검사( 매직 넘버 검사 )
	if( bmpFileHeader.bfType != 0x4d42 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1271 ) );	

		file->Close();
		SAFE_DELETE( file );
		return 0;
	}

	/// BMP 압축 여부 검사(무압축이여야 함)
	if ((bmpInfoHeader.biCompression == BI_RLE4) || (bmpInfoHeader.biCompression == BI_RLE8))
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1271 ) );	

		file->Close();
		SAFE_DELETE( file );
		return 0;
	}

	/// BMP 이미지 크기 검사
	if( !(bmpInfoHeader.biWidth == 24 && bmpInfoHeader.biHeight == 24) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1262 ) );	

		file->Close();
		SAFE_DELETE( file );
		return 0;
	}

	/// BMP 해상도 검사 4, 8, 24, 32bit 해상도 지원
	if( !(bmpInfoHeader.biBitCount == 4  || bmpInfoHeader.biBitCount == 8 ||
		bmpInfoHeader.biBitCount == 24 || bmpInfoHeader.biBitCount == 32 ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1264 ) );	

		file->Close();
		SAFE_DELETE( file );
		return 0;
	}

	/// 파일 사이즈 검사 (제한 사이즈 검사)
	unsigned int len = file->GetSize();
	if( len > MARK_SIZE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1263 ) );	

		file->Close();
		SAFE_DELETE( file );
		return 0;
	}

	/// 버퍼 저장
	file->Seek( 0 );
	::memset( &mMarkBuffer, 0, sizeof(mMarkBuffer) );
	file->Read( &mMarkBuffer, MARK_SIZE );

	file->Close();
	SAFE_DELETE( file );

	/// 로드
	cEngineFile enginefile( pathName.Cstr(), false );
	if( enginefile == false )
		return 0;

	NiPixelDataPtr pixelData = 0;

	cBMPReader bmpReader;
	pixelData = bmpReader.ReadFile( enginefile, 0 );

	if( pixelData == 0 )
	{
		assert( 0 );
		return 0;
	}

	NiTexture::FormatPrefs prefs;
	prefs.m_eMipMapped = pixelData->GetNumMipmapLevels() <= 1 ? NiTexture::FormatPrefs::NO : NiTexture::FormatPrefs::YES;

	mpPreviewMark = NiSourceTexture::Create( pixelData, prefs );
	if( mpPreviewMark == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1263 ) );	

		assert( 0 );
		return 0;
	}
	return mpPreviewMark;
}
