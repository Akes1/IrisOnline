#include "stdafx.h"
#include "ServerStage.h"
#include "ServerUIManager.h"

#include "StageManager.h"
#include "GameResourceManager.h"
#include "IntroStage.h"

#include "ServerWindow.h"
#include "Application.h"

cServerStage::cServerStage()
: mpServerUIManager(0)
, mLoadComplete( false )
, mCheckServerComplete( false )
{

}

cServerStage::~cServerStage()
{

}

void cServerStage::Init( cStageManager* pStageManager )
{	
	mpStageManager = pStageManager;
}

void cServerStage::InitStage()
{
	if( mInit )	return;

	if( GAMERESOURCEMAN->InitServer() == false )
	{
		assert(0);
	}

	/// UI
	mpServerUIManager = new cServerUIManager;
	mpServerUIManager->Init();
	mInit = true;
}

void cServerStage::Exit()
{
	if( GAMERESOURCEMAN )
		GAMERESOURCEMAN->LoadEnd();

	SAFE_DELETE( mpServerUIManager );
	mpStageManager = 0;
}

void cServerStage::Open()
{
	/// ui
	mpServerUIManager->Open();

	mLoadComplete = false;
	mCheckServerComplete = false;

	/// 070404 추가 - 곽철중 [서버리스트 요청]
	if ( NETWORK->IsConnected( ) == true )
		ReqServerList();

#ifdef NOCONNECT_SERVER
	mpStageManager->ChangeStage( eStage_Intro );
#endif

	cIntroStage* stage = (cIntroStage*)mpStageManager->GetStage( eStage_Intro );
	if( stage )
		stage->ClearBackImage();

	if( GAMERESOURCEMAN->LoadInitData3() == false )
	{
		assert(0);
	}
}

void cServerStage::Close()
{
	if( GAMERESOURCEMAN )
		GAMERESOURCEMAN->LoadEnd();

	/// UI
	mpServerUIManager->Close();
}

void cServerStage::Process( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	unsigned int flag = GAMERESOURCEMAN->CheckInitData();
	if( flag == eData_Error )
	{
		assert(0);
		NiMessageBox( "Failed load gameData. Please reinstall client or patch.", "GameData Error" );
		::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
		::PostQuitMessage(0);
		return;
	}
	else if( flag == eData_End )
	{
		mLoadComplete = true;
		GAMERESOURCEMAN->LoadEnd();
	}

	if( mLoadComplete == true && mCheckServerComplete == true )
	{
		/// 모달 처리
		UIMAN->SendEventTopMsgBox( true );

		/// 스테이지 변경
		STAGEMAN->ChangeStage( eStage_Select );
	}

	/// UI
	mpServerUIManager->Process( accumTime );
}

void cServerStage::ReqServerList()
{
	MSGROOT msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_USER;
	msg.Protocol = NM_USER_SERVER_LIST_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// Msgbox 출력
	UIMAN->CreateMsgBox( eStage_Server, eMSGBOX_MODAL,
		eMBEVENT_NONE, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 9 ), 
		GAMERESOURCEMAN->GetGameText( 2 ) // 알림
		);

	mpServerUIManager->SetEnableConnectBtn( false );
}


void cServerStage::NetworkMsgParser( unsigned long , char* pMsg, unsigned long /*length*/ )
{	
	/// 070404 추가 - 곽철중 [서버리스트 & 채널리스트 & 게임접속 응답]
	MSGROOT* pMsgRoot = (MSGROOT*)pMsg;

	switch( pMsgRoot->Category )
	{
	case NM_USER:
		{
			switch( pMsgRoot->Protocol )
			{
			case NM_USER_SERVER_LIST_RES:
				{
					/// 서버리스트 응답 - 성공시 서버리스트 / 실패시 응답 없음
					MSG_RES_SERVERLIST*  pMsgResServerlist = (MSG_RES_SERVERLIST*)pMsg;

					cServerWindow* pSrvWindow  = (cServerWindow*)UIMAN->FindNodeByID( eUIID_SERVER_WINDOW );

					/// 서버리스트 삭제.
					if( pSrvWindow )
					{
						pSrvWindow->Clear();

						for ( unsigned int i = 0; i < pMsgResServerlist->RowCount; i++ )
						{
							pSrvWindow->AddServer( pMsgResServerlist->Servers[ i ].NumOfServer,
													pMsgResServerlist->Servers[ i ].Status,
													pMsgResServerlist->Servers[ i ].Name,
													pMsgResServerlist->Servers[ i ].Age );

						}

						pSrvWindow->UpdateServer( pMsgResServerlist->ServerNo );
						pSrvWindow->UpdateButton();
					}
					else
						assert(0);

					/// 모달 정보 갱신
					UIMAN->SendEventTopMsgBox( true );

					/// 윈도우 활성화
					mpServerUIManager->Visible();

					/// 비활성화
					bool enable = ( pMsgResServerlist->RowCount == 0 ) ? false : true;
					mpServerUIManager->SetEnableConnectBtn( enable );
				}
				break;
			case NM_USER_COME_IN_GAME_RES:
				{
					/// 게임접속승인 응답 - 0성공시 / 1실패시 
					MSG_RES_COMEIN_GAME* msg = (MSG_RES_COMEIN_GAME*)pMsg;

					mpServerUIManager->SetEnableWindow( true );

					if ( msg->ErrorCode == ERROR_COMEIN_GAME_SUCCESS )
					{
						///
					}
					else
					{
						/// 실패시 처리
						UIMAN->DestroyMsgBox( eMBEVENT_CONNECT_GAMESRV );

						// 성인 서버 
						STAGEMAN->SetAdultServer( false );

						switch( msg->ErrorCode )
						{
						case ERROR_COMEIN_GAME_FAIL:
							{
								UIMAN->CreateMsgBox( eStage_Server, eMSGBOX_MODAL,
									eMBEVENT_SERVER_REFRESH, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 190 ), 
									GAMERESOURCEMAN->GetGameText( 2 ) // 알림
									);
							}
							break;
						case ERROR_COMEIN_GAME_DB:
						case ERROR_COMEIN_GAME_SYSTEM:
							{
								UIMAN->CreateMsgBox( eStage_Server, eMSGBOX_MODAL,
									eMBEVENT_SERVER_REFRESH, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 188 ), 
									GAMERESOURCEMAN->GetGameText( 2 ) // 알림
									);
							}
							break;
						case ERROR_COMEIN_GAME_AGE:
							{
								UIMAN->CreateMsgBox( eStage_Server, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 189 ), 
									GAMERESOURCEMAN->GetGameText( 2 ) // 알림
									);
							}
							break;
						}
					}
				}
				break;
			case NM_USER_GAMESRV_RES:
				{
					/// 게임서버인증 응답 - 0성공시 / 1실패시 
					MSG_RES_GAMESRV* msg = (MSG_RES_GAMESRV*)pMsg;

					mpServerUIManager->SetEnableConnectBtn( true );

					if ( msg->ErrorCode == 0 )
					{
						mCheckServerComplete = true;
						if( mLoadComplete == true )
						{
							/// 모달 처리
							UIMAN->SendEventTopMsgBox( true );

							/// 스테이지 변경
							STAGEMAN->ChangeStage( eStage_Select );
						}
					}
					else
					{
						UIMAN->SendEventTopMsgBox( false );

						UIMAN->CreateMsgBox( eStage_Server, eMSGBOX_MODAL,
							eMBEVENT_CONNECT_DEAD, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 161 ), 
							GAMERESOURCEMAN->GetGameText( 2 ) // 알림
							);
					}
				}
				break;
			}
		}
		break;
	}
}

void cServerStage::MsgBoxEventParser( unsigned int event, unsigned int result, void* /*exData*/ )
{
	switch( event )
	{
	case eMBEVENT_CONNECT_DEAD:
		{
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	case eMBEVENT_SERVER_REFRESH:
		{
			/// 서버 목록 재요청
			ReqServerList();
		}
		break;
	}

}
