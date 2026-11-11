#include "stdafx.h"
#include "LoginStage.h"
#include "LoginUIManager.h"
#include "StageManager.h"
#include "IntroStage.h"

#include "UIMsgboxEvent.h"
#include "UIID.h"

#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "worldmanager.h"
#include "SceneManager.h"
#include "StaticSceneNode.h"

#include "CameraManager.h"
#include "Application.h"
#include "UIManager.h"

#include "RenderSystem.h"

#ifdef _GMTOOL
#include "GMTool.h"
#endif

cLoginStage::cLoginStage()
: mpLoginUIManager(0)
, mLoadComplete(false)
, mCheckLoginComplete(false)
{	
}

cLoginStage::~cLoginStage()
{
}

/// 주의사항 :
void cLoginStage::Init( cStageManager* pStageManager )
{	
	mpStageManager = pStageManager;
}

/// 주의사항 :
void cLoginStage::InitStage()
{
	if( mInit )	return;

	if( GAMERESOURCEMAN->InitLogin() == false )
	{
		assert(0);
	}

	/// UI
	mpLoginUIManager = new cLoginUIManager;
	mpLoginUIManager->Init();
	
#ifdef _GMTOOL
	if( THEAPP->InitGMTool() == false )
	{
		assert(0);
	}

	if( GMTOOL->LoadData() == false )
	{
		assert(0);
	}
#endif


	mInit = true;
}

/// 주의사항 :
void cLoginStage::Exit()
{
	if( GAMERESOURCEMAN )
		GAMERESOURCEMAN->LoadEnd();

	SAFE_DELETE( mpLoginUIManager );
	mpStageManager = 0;
}

/// 주의사항 :
void cLoginStage::Open()
{
	if( !mpStageManager )	return;

	mLoadComplete = false;
	mCheckLoginComplete = false;

	OBJECTMAN->ClearHeroData();
	WORLDMAN->Close();	

	/// ui
	mpLoginUIManager->Open();

	if( GAMERESOURCEMAN->LoadInitData2() == false )
	{
		assert(0);
	}

#ifndef NOCONNECT_SERVER
	/// 기존 연결 제거
	char* ip = THEAPP->GetIP();
	if( NETWORK->PostConnect( ip, 14400 ) == false )
	{
		/// 오류 작업 추가 해야됨.
		assert(0);
		mpLoginUIManager->Close();
		return;
	}

	/// 연결 대기창 등록
	UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
		eMBEVENT_CONNECT_LOGINSRV, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 0 ),// 서버 연결 메세지
		GAMERESOURCEMAN->GetGameText( 2 ) // 알림
		);

#else
	/// 네트웤 접속 없이 임시로 켜는 부분
	mpStageManager->ChangeStage( eStage_Select );
#endif
}

/// 주의사항 :
void cLoginStage::Close()
{
	if( GAMERESOURCEMAN )
		GAMERESOURCEMAN->LoadEnd();

	/// UI
	mpLoginUIManager->Close();
}

/// 주의사항 :
void cLoginStage::Process( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	cIntroStage* intro = (cIntroStage*)STAGEMAN->GetStage( eStage_Intro );
	if( intro && intro->IsProcessEnd() )
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
	}

	if( mLoadComplete == true && mCheckLoginComplete == true )
	{
		/// 모달 처리
		UIMAN->SendEventTopMsgBox( true );

		/// 스테이지 변경
		STAGEMAN->ChangeStage( eStage_Server );
	}

	/// UI
	mpLoginUIManager->Process( accumTime );
}

/// 주의사항 :
void cLoginStage::NetworkMsgParser( unsigned long , char* pMsg, unsigned long /*length*/ )
{
	/// 070403 추가 - 곽철중 [로그인 인증 응답 - 성공 / 실패]
	MSGROOT* pMsgRoot = (MSGROOT*)pMsg;

	switch( pMsgRoot->Category )
	{
	case NM_USER:
		{
			switch( pMsgRoot->Protocol )
			{
			case NM_USER_WEB_LAUNCHER_RES:
				{
					MSG_RES_WEB_LAUNCHER* pmsg = (MSG_RES_WEB_LAUNCHER*)pMsgRoot;

					switch( pmsg->ErrorCode )
					{
 					case ERROR_WEB_LAUNCHER_SUCCESS:
 						{
							assert(0);
 						}
 						break;
					case ERROR_WEB_LAUNCHER_FAIL:	// 인증코드 에러
						{
							// 서버로부터 연결이 해제 되었을 경우...
							UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
								eMBEVENT_CONNECT_DEAD, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 33 ),
								GAMERESOURCEMAN->GetGameText( 301 )
								);
						}
						break;
					}
				}
				break;
			case NM_USER_LOGIN_RES:
				{
					MSG_DIST_USERINFO* pMsgDistUserinfo = (MSG_DIST_USERINFO*)pMsgRoot;
					
					mpLoginUIManager->SetSendLoginFlag( false );

					if( pMsgDistUserinfo->ErrorCode == ERROR_DIST_USERINFO_SUCCESS )
					{
						/// 성공
						STAGEMAN->SetUserIdx( pMsgDistUserinfo->UserIdx );
						STAGEMAN->SetLoginIdx( pMsgDistUserinfo->LoginIdx );
						STAGEMAN->SetTwitterUid( pMsgDistUserinfo->twitterUid );
						STAGEMAN->SetTwitterPwd( pMsgDistUserinfo->twitterPwd );

						mCheckLoginComplete = true;
						if( mLoadComplete == true )
						{
							/// 모달 처리
							UIMAN->SendEventTopMsgBox( true );

							/// 스테이지 변경
							STAGEMAN->ChangeStage( eStage_Server );
						}
					}
					else
					{
						/// 기존 모달 정보는 제거 한다.
						UIMAN->SendEventTopMsgBox( false );

						/// 새로운 모달 정보를 띄운다.
						LPCTSTR msg;
						switch( pMsgDistUserinfo->ErrorCode )
						{
						case ERROR_DIST_USERINFO_ID_PWD: /// 실패-ID/PWD
							msg = GAMERESOURCEMAN->GetGameText(5);
							break;
						case ERROR_DIST_USERINFO_SLEVEL: /// 실패-보안등급 적용중
							msg = GAMERESOURCEMAN->GetGameText(6);
							break;
						case ERROR_DIST_USERINFO_BLOCK:  /// 실패-BLOCK 됨
							msg = GAMERESOURCEMAN->GetGameText(7);
							break;
						case ERROR_DIST_USERINFO_EXISTS: /// 실패-접속중
							msg = GAMERESOURCEMAN->GetGameText(8);
							break;
						case ERROR_DIST_USERINFO_IS_PIN:
							msg = GAMERESOURCEMAN->GetGameText(50);
							break;
						case ERROR_DIST_USERINFO_PIN:	/// 실패-PIN
							msg = GAMERESOURCEMAN->GetGameText(51);
							break;
						case ERROR_DIST_USERINFO_AGE:	/// 사용 연령제한
							msg = GAMERESOURCEMAN->GetGameText(185);
							break;
						case ERROR_DIST_USERINFO_FAIL:	/// 실패
							msg = GAMERESOURCEMAN->GetGameText(183);
							break;
						case ERROR_DIST_USERINFO_LOGIN_LIMIT: ///로그인 제한
							msg = GAMERESOURCEMAN->GetGameText( 39 );
							break;
						default:
							return;
						};
						
						if( pMsgDistUserinfo->ErrorCode == ERROR_DIST_USERINFO_EXISTS )
						{
							/// 중복 로그인에 대한 확인
							UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
								eMBEVENT_DOUBLE_LOGIN, eSKIN_OK,
								msg,
								GAMERESOURCEMAN->GetGameText( 2 ) // 알림
								);
						}
						else if( pMsgDistUserinfo->ErrorCode == ERROR_DIST_USERINFO_SLEVEL )
						{
							/// 인증절차 오류에 대한 확인
							UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
								eMBEVENT_PWD_FAIL, eSKIN_OK,
								msg,
								GAMERESOURCEMAN->GetGameText( 2 ) // 알림
								);
						}
						else if( pMsgDistUserinfo->ErrorCode ==  ERROR_DIST_USERINFO_IS_PIN )
						{
							UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
								eMBEVENT_IS_PIN_FAIL, eSKIN_OK,
								msg,
								GAMERESOURCEMAN->GetGameText( 2 ) // 알림
								);
						}
						else
						{
							UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
								eMBEVENT_LOGIN_ERROR, eSKIN_OK,
								msg,
								GAMERESOURCEMAN->GetGameText( 2 ) // 알림
								);
						}

					}
				}
				break;
			}
		}
		break;
	}
}

void cLoginStage::MsgBoxEventParser( unsigned int event, unsigned int result, void* /*exData*/ )
{
	switch( event )
	{
	case eMBEVENT_LOGIN_ERROR:
		{
			mpLoginUIManager->FocusIDEdit();
		}
		break;
	case eMBEVENT_IS_PIN_FAIL:
	case eMBEVENT_PWD_FAIL:
		{
			mpLoginUIManager->FocusPassEdit();
		}
		break;
	case eMBEVENT_DOUBLE_LOGIN:
		{
			if( result == eMESSAGE_BUTTON_YES )
			{
				/// 기존 연결을 제거 하는 루틴을 넣는다.
			}
			else
			{
				/// 기존 연결을 유지한다.
				mpLoginUIManager->FocusIDEdit();
			}
		}
		break;
	case eMBEVENT_CONNECT_LOGINSRV:
		if( result == eMESSAGE_BUTTON_YES )
		{
			// 웹런쳐를 통한 로그인이 아니면,
			if( THEAPP->GetAuthCode() == -1 )
			{
				mpLoginUIManager->Visible();
			}
		}
		break;
	case eMBEVENT_CONNECT_DEAD:
	case eMBEVENT_CONNECT_LOGINFAILE:
		{
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	case eMBEVENT_VIDEO_CAPTURE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			if( RENDERSYS )
				RENDERSYS->ToggleVideoCapture();
		}
		break;
	}

}

void cLoginStage::CreateLoginMap()
{
	/// 데이터 등록
	cStaticSceneNodeParam param;

	/// 기본 속성 인자들을 설정
	param.mPathName = "./data/map/character_select_bg.nif";
	param.mOccludeLevel = 3;
	param.mNoAttachLight = true;
	param.mFogApplied = false;
	param.mDisableAnimation = false;

	/// 장면을 초기화
	SCENEMAN->Init( NiPoint3::ZERO, 100.0f, 100000.0f );

	/// 노드를 생성
	cStaticSceneNode* n = SCENEMAN->CreateStatic( param );
	if( n == 0 )
		return;

	NiNode* orien = NiDynamicCast( NiNode, n->GetNiObj()->GetObjectByName("SelectCamera") );
	if( orien == 0 )
	{
		assert(0);
		return;
	}

	NiAVObject* child = 0;
	NiCamera* cam = 0;
	for(unsigned int i=0;i<orien->GetArrayCount();i++)
	{
		child = orien->GetAt(i);
		if( child && NiIsKindOf( NiCamera, child ) == true )
		{
			cam = (NiCamera*)child;
			break;
		}
	}
	if( cam == 0 )
	{
		assert(0);
		return;
	}

	CAMERAMAN->GetCurrent()->ChangeNiCamera( NiSmartPointerCast( NiCamera, cam->CreateDeepCopy() ) );

	NiNode* node = CAMERAMAN->GetCurrent()->GetOrientNode();
	node->SetWorldTranslate( orien->GetWorldTranslate() );
	node->SetWorldRotate( orien->GetWorldRotate() );
	node->SetWorldScale( orien->GetWorldScale() );
	node->SetLocalTransform( orien->GetLocalTransform() );
}
