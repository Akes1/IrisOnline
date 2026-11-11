#include "stdafx.h"
#include "SelectStage.h"
#include "StageManager.h"
#include "MakeStage.h"

#include "ObjectManager.h"
#include "SelectUIManager.h"
#include "GameResourceManager.h"

#include "SelectWindow.h"
#include "ChannelWindow.h"


#include "CameraManager.h"

#include "worldmanager.h"
#include "SceneManager.h"
#include "DisplayObject.h"
#include "cardsceneNode.h"

#include "LightAgent.h"

#include "Application.h"

#include "EffectSceneNode.h"

#include "RenderSystem.h"

cSelectStage::cSelectStage()
: mpSelectUIManager(0)
{
	mCurrentSelectIndex = 0;
	mSelectEffectIdx = UINT_MAX;
	mDisplayObject = 0;

	mCardMoving = false;
	mCardMoveState = cCardSceneNode::eCard_Stop;
	mCardMoveCount = 0;

	//mChangeLogin = false;

	::memset( mCardDramaObj, 0, sizeof( mCardDramaObj ) );
}

cSelectStage::~cSelectStage()
{
	ClearDisplayObject();

	for(unsigned int i=0;i<5; i++)
	{
		mCardDramaObj[i] = 0;
	}
}

void cSelectStage::Init( cStageManager* pStageManager )
{
	mpStageManager = pStageManager;

	for(unsigned int i=0;i<5; i++)
	{
		mCardDramaObj[i] = 0;
	}

	mCardInitTime[0] = 0.0f;
	mCardInitTime[1] = 1.0f;
	mCardInitTime[2] = 50.0f/30.0f;
	mCardInitTime[3] = 70.0f/30.0f;
	mCardInitTime[4] = 3.0f;

	mCardMoveState = cCardSceneNode::eCard_Stop;
}

void cSelectStage::InitStage()
{
	if( mInit )	return;

	/// UI
	mpSelectUIManager = new cSelectUIManager;
	mpSelectUIManager->Init();

	mInit = true;
}

void cSelectStage::Exit()
{
	/// UI
	SAFE_DELETE( mpSelectUIManager );

	mpStageManager = 0;
}

void cSelectStage::Open()
{
	if( !mpStageManager )	return;

//	mCurrentSelectIndex = 0;

	mSelectEffectIdx = UINT_MAX;

	WORLDMAN->Close();

	/// UI
	mpSelectUIManager->Open();

	/// 070406 추가 - 곽철중 [채널 & 캐릭터 리스트 요청]
	if ( NETWORK->IsConnected( ) == true )
	{
		MSGROOT msg;

		///// INIT MSGROOT.
		//memset( &msg, 0, sizeof(msg) );
		///// SET MSGROOT [CATEGORY & PROTOCOL].
		//msg.Category    = NM_USER;
		//msg.Protocol    = NM_USER_CHANNEL_LIST_REQ;
		///// SEND MSGROOT.
		//NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// INIT MSGROOT.
		memset( &msg, 0, sizeof(msg) );
		/// SET MSGROOT [CATEGORY & PROTOCOL].
		msg.Category = NM_USER;
		msg.Protocol = NM_USER_CHARACTER_LIST_REQ;
		/// SEND MSGROOT.
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// Msgbox 출력
		UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_NONE,
			GAMERESOURCEMAN->GetGameText( 11 ), // 캐릭터 목록
			GAMERESOURCEMAN->GetGameText( 2 )   // 알림
			);
	}

#ifdef NOCONNECT_SERVER
	mpStageManager->ChangeStage( eStage_Load );
#endif

	//mChangeLogin = false;
}

void cSelectStage::Close()
{
	/// UI
	mpSelectUIManager->Close();

	ClearDisplayObject();

//	SCENEMAN->Clear();
//*
	for( unsigned int i=0; i<5;i++ )
	{
		if( mCardDramaObj[i] == 0 )
			continue;

		mCardDramaObj[i]->SetViewNode( false );
	}
	mCardMoveState = cCardSceneNode::eCard_Stop;
//*/
}

void cSelectStage::Process( unsigned long deltaTime, unsigned long accumTime )
{
	//if( mChangeLogin == true )
	//{
	//	mChangeLogin = false;
	//	STAGEMAN->ChangeStage( eStage_Login );
	//	return;
	//}

	/// UI
	mpSelectUIManager->Process( accumTime );

	if( mCardMoving == true )
	{
		bool cardStop=true;
		for( unsigned int i=0;i<5;i++ )
		{
			if( mCardDramaObj[i]->GetMoveState() != cCardSceneNode::eCard_Stop )
				cardStop = false;

			if( mCardDramaObj[i]->GetCurrentPosIdx() == 0 )
				mCurrentSelectIndex = i;
		}

		if( mCardMoveCount )
		{
			if( mCardMoveState == cCardSceneNode::eCard_Rew )
				RewCard();
			else if( mCardMoveState == cCardSceneNode::eCard_Play )
				PlayCard();

			mCardMoveCount--;
		}
		else
		{
			if( cardStop == true )
			{
				mCardMoveState = cCardSceneNode::eCard_Stop;
				mCardMoving = false;

				cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt(mCurrentSelectIndex);
				if( p )
				{
					NiTransform trans;
					trans.MakeIdentity();
					cEffectSceneNode* eff = p->SetLinkdEffect( eLINK_FOOT, "./data/map/character_select_effect.nif", &trans, false, true );
					if( eff )
						mSelectEffectIdx = eff->GetIndexByManger();

					/// setting window info
					bool delWait = ( p->GetDeleteWaitTime() != LONG_MAX );
					mpSelectUIManager->SetSelectInfo( (int)p->GetLevel(), p->GetName(), p->GetJob(), p->GetMapNum() );
					mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_StartButton, true, delWait );
				}
				else
					mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_MakeButton, true, false );

				mDisplayObject = p;
			}
		}
	}

	for( unsigned int i=0;i<5;i++ )
	{
		cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt(i);
		if( p )
		{
			// 갱신
			p->Update( deltaTime, accumTime );

			if( p->IsDeleteLock() == false && p->GetDeleteWaitTime() <= 0 )
			{
				SendDeleteCharacter( p );
			}
		}
	}
}

void cSelectStage::ProcessLButtonClick( cMouseAgent* pMouse )
{
	pMouse;
}

void cSelectStage::ProcessLButtonDClick( cMouseAgent* pMouse )
{
	pMouse;
/*
	int mouseX = pMouse->GetMouseEventX();
	int mouseY = pMouse->GetMouseEventY();

	/// 한번만 검사하도록..
	mCollidableArray.Clear();
	if( SCENEMAN->PickDynamics(&mCollidableArray, mouseX, mouseY, true) == true )
	{
		cSceneNode* n = (cSceneNode*)mCollidableArray[0];

		if( n->GetType() == cSceneNode::eCARDDRAMA )
		{
			cSelectStageCardSceneNode* p = (cSelectStageCardSceneNode*)n;
			unsigned int index = p->GetOverIndex();

			if( index != 0 )
			{
				switch( mViewState )
				{
				case eVIEW_LIST:
					{
						mCurrentSelectIndex = index;
						if( mDisplayObjectMap.GetAt( mCurrentSelectIndex ) == 0 )
						{
							if( 0 < mCurrentSelectIndex && mCurrentSelectIndex < 6 )
							{
								STAGEMAN->ChangeStage( eStage_Make );

								cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
								if( p )
									p->SetMakeStandIndex( mCurrentSelectIndex - 1 );
							}

							return;
						}
						SetViewState( eVIEW_GATHERING );
					}
					break;
				case eVIEW_CHANNEL:
					{
						if( 0 < index && index < 6 )
						{
							SetViewState( eVIEW_RETURNING );
							mCurrentSelectIndex = 0;
						}
					}
					break;
				}
			}
		}
	}
*/
}

void cSelectStage::ProcessMouseover( unsigned long /*time*/, cMouseAgent* pMouse )
{
	pMouse;
/*
//	if( mViewState <= eVIEW_CHANNEL )
//		return;

	int mouseX = pMouse->GetMouseX();
	int mouseY = pMouse->GetMouseY();

	mCollidableArray.Clear();
	if( SCENEMAN->PickDynamics(&mCollidableArray, mouseX, mouseY, true) == true )
	{
		cSceneNode* n = (cSceneNode*)mCollidableArray[0];

		if( n->GetType() == cSceneNode::eCARDDRAMA )
		{

		}
	}
*/
}

void cSelectStage::NetworkMsgParser( unsigned long /*connectionindex*/, char* pMsg, unsigned long /*length*/ )
{
	MSGROOT* pTemMsg = (MSGROOT*)pMsg;
	switch( pTemMsg->Category )
	{
	case NM_USER:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_USER_CHANNEL_LIST_RES:
				{
					Msg_UserChannelListAck( pTemMsg );
				}
				break;
			case NM_USER_CHARACTER_LIST_RES:
				{
					MSG_ERROR* pmsg = (MSG_ERROR*)pTemMsg;

					SetEnableWindow( true );

					if( pmsg->ErrorCode == 0 )
					{
						Msg_UserCharacterListAck( pTemMsg );
					}
					else
					{
						/// 에러 처리..
						return;
					}
				}
				break;
			case NM_USER_CHARACTER_DELETE_RES:
				{
					MSG_RES_CHARACTER_DELETE* pmsg = (MSG_RES_CHARACTER_DELETE*)pTemMsg;
					
					SetEnableWindow( true );
					UIMAN->SendEventTopMsgBox( true );

					cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt( pmsg->StandIdx );
					if( p )
					{
						p->SetDeleteLock( false );
					}

					switch( pmsg->ErrorCode )
					{
					case ERROR_CHARACTER_DELETE_SUCCESS:
						{
							DeleteDisplayObject( pmsg->StandIdx );
						}
						break;
					case ERROR_CHARACTER_DELETE_FAIL:
						{
							/// Msgbox 출력
							UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 13 ), // 캐릭터 목록
								GAMERESOURCEMAN->GetGameText( 2 )   // 알림
								);
						}
						break;
					case ERROR_CHARACTER_DELETE_GUILDMASTER:
						{
							UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 27 ), // 캐릭터 목록
								GAMERESOURCEMAN->GetGameText( 2 )   // 알림
								);

						}
						break;
					}
				}
				break;
			case NM_USER_CHARACTER_DELETE_WAIT_RES:
				{
					MSG_RES_CHARACTER_DELETE_WAIT* pmsg = (MSG_RES_CHARACTER_DELETE_WAIT*)pTemMsg;

					UIMAN->DestroyMsgBox( eMBEVENT_CHARACTER_DELETE_WAIT );

					switch( pmsg->ErrorCode )
					{
					case ERROR_CHARACTER_DELETE_WAIT_SUCCESS:
						{
							// 성공시 시간 표시
							if( pmsg->StandIdx >= 5 )
								assert(0);

							cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt( pmsg->StandIdx );
							if( p )
							{
								p->SetDeleteWaitTime( pmsg->DelWaitTime );

								if( mDisplayObject == p )
								{
									bool delWait = ( p->GetDeleteWaitTime() != LONG_MAX );
									mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_StartButton, true, delWait );
								}
							}
							else
								assert(0);
						}
						break;
					case ERROR_CHARACTER_DELETE_WAIT_FAIL:
						{
							UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 13 ), // 캐릭터 목록
								GAMERESOURCEMAN->GetGameText( 2 )   // 알림
								);
						}
						break;
					case ERROR_CHARACTER_DELETE_WAIT_GUILDMASTER:
						{
							UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 27 ), // 캐릭터 목록
								GAMERESOURCEMAN->GetGameText( 2 )   // 알림
								);
						}
						break;
					}
				}
				break;
			case NM_USER_CHARACTER_DELETE_CANCEL_RES:
				{
					MSG_RES_CHARACTER_DELETE_CANCEL* pmsg = (MSG_RES_CHARACTER_DELETE_CANCEL*)pTemMsg;

					UIMAN->DestroyMsgBox( eMBEVENT_CHARACTER_DELETE_CANCELWAIT );

					switch( pmsg->ErrorCode )
					{
					case ERROR_CHARACTER_DELETE_CANCEL_SUCCESS:
						{
							if( pmsg->StandIdx >= 5 )
								assert(0);

							cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt( pmsg->StandIdx );
							if( p )
							{
								p->SetDeleteWaitTime( LONG_MAX );

								if( mDisplayObject == p )
								{
									bool delWait = ( p->GetDeleteWaitTime() != LONG_MAX );
									mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_StartButton, true, delWait );
								}
							}
							else
								assert(0);
						}
						break;
					case ERROR_CHARACTER_DELETE_CANCEL_FAIL:
						{
							UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
								eMBEVENT_NONE, eSKIN_OK,
								GAMERESOURCEMAN->GetGameText( 163 ),
								GAMERESOURCEMAN->GetGameText( 2 )   // 알림
								);
						}
						break;
					}
				}
				break;
			case NM_USER_COME_IN_GAME_RES:
				{
					/// 게임접속승인 응답 - 0성공시 / 1실패시 
					MSG_RES_COMEIN_GAME* msg = (MSG_RES_COMEIN_GAME*)pMsg;

					if ( msg->ErrorCode == 0 )
					{
						SetEnableWindow( false );
					}
					else
					{
						SetEnableWindow( true );

						/// 실패시 처리
						UIMAN->SendEventTopMsgBox( false );
					}
				}
				break;
			case NM_USER_GAMESRV_RES:
				{
					/// 게임서버인증 응답 - 0성공시 / 1실패시 
					MSG_RES_GAMESRV* msg = (MSG_RES_GAMESRV*)pMsg;
					if ( msg->ErrorCode != 0 )
					{
						UIMAN->SendEventTopMsgBox( false );

						// 					
						UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
							eMBEVENT_CONNECT_DEAD, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 161 ), 
							GAMERESOURCEMAN->GetGameText( 2 ) // 알림
							);
					}
				}
				break;
			case NM_USER_CHARACTER_SELECT_RES:
				{
					MSG_ERROR* pmsg = (MSG_ERROR*)pTemMsg;
					if( pmsg->ErrorCode == 0 )
					{
						UIMAN->SendEventTopMsgBox( true );

						/// 성공
						Msg_HeroSelectAck( pTemMsg );
					}
					else
					{
						UIMAN->SendEventTopMsgBox( false );

						/// 실패
						Msg_HeroSelectNack( pTemMsg );
					}
				}
				break;
			}
		}
		break;
	}
}

/// msg funtions
void cSelectStage::Msg_UserChannelListAck( MSGROOT* pMsg )
{
	/// 채널리스트 응답 - 성공시 채널리스트 / 실패시 응답 없음
	MSG_RES_CHANNELLIST* pMsgResChannellist = (MSG_RES_CHANNELLIST*)pMsg;

	/// 채널윈도우 보여주기.
	cChannelWindow* pChannelWindow = (cChannelWindow*)UIMAN->FindNodeByID( eUIID_CHANNEL_WINDOW );
	if( pChannelWindow )
		pChannelWindow->Clear();

	/// 이전에 접속했떤 채널에 세팅
	STAGEMAN->SetChannelIdx( pMsgResChannellist->ChannelNo );

	unsigned long color = COLOR_GRAY;
	for ( unsigned long i = 0; i < pMsgResChannellist->RowCount; i++ )
	{
		cStringT Name;
		TCHAR   buffer[260];

		UIMAN->GetUIText( &Name, 704);
		::_stprintf_s( buffer, _T("%02d"), pMsgResChannellist->Channels[ i ].NumOfChannel );
		Name += buffer;

		unsigned int textId = 0;
		switch ( pMsgResChannellist->Channels[ i ].Status )
		{
		case 5: textId = 705; color = COLOR_RED;	break;	/// 만원
		case 4: textId = 700; color = COLOR_RED;	break;	/// 혼잡
		case 3: textId = 701; color = COLOR_ORANGE;	break;	/// 양호
		case 2: textId = 702; color = COLOR_GREEN;	break;	/// 원활
		case 1: textId = 703; color = COLOR_GRAY;	break;	/// 준비중
		default:textId = 703; color = COLOR_GRAY;	break;
		}

		if( pChannelWindow && textId > 0 )
		{
			pChannelWindow->AddChannel( pMsgResChannellist->Channels[ i ].NumOfChannel,
				pMsgResChannellist->Channels[ i ].Status,
				Name.Cstr(), UIMAN->GetUIText( textId ), 
				color,
				pMsgResChannellist->ChannelNo == (unsigned long)pMsgResChannellist->Channels[ i ].NumOfChannel
				);			
		}
	}

	if( pChannelWindow )
		pChannelWindow->UpdateButton();

	UIMAN->SendEventTopMsgBox( true );

	mpSelectUIManager->VisibleChannelState();
}

void cSelectStage::Msg_UserCharacterListAck( MSGROOT* pMsg )
{
	mCurrentSelectIndex = 0;

	UIMAN->SendEventTopMsgBox( true );
	ClearDisplayObject();

	mpSelectUIManager->VisibleListState();
	mpSelectUIManager->ClearSelectInfo();
	mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_MakeButton, true, false );

	MSG_CHARACTER_LIST* pmsg = (MSG_CHARACTER_LIST*)pMsg;

	//////////////////////////////////////////////////////////////////////////
	/// 현재 임시 코드 - 데빌서버 신규 캐릭터 생성 제한
	//long serverNum = STAGEMAN->GetServerIdx();
	//if( serverNum == 3 )
	//{
	//	if( pmsg->CharacterNum == 0 )
	//	{
	//		/// MessageBox
	//		UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
	//			eMBEVENT_CHARACTER_CREATE_LIMIT, eSKIN_OK,
	//			GAMERESOURCEMAN->GetGameText( 162 ), // 캐릭터 목록 전송
	//			GAMERESOURCEMAN->GetGameText( 2 ) // 알림
	//			);
	//		return;
	//	}
	//}	
	//////////////////////////////////////////////////////////////////////////

	if( 0 >= pmsg->CharacterNum )	return;

	mCurrentSelectIndex = pmsg->StandIdx;
	if( mCurrentSelectIndex >= 5 )
	{
		assert(0);
		mCurrentSelectIndex = 0;
	}

	MSG_CHARACTER_BASEINFO* pInfo = pmsg->BaseInfo;
	for( unsigned short i=0;i<pmsg->CharacterNum; ++i )
	{
		cDisplayObject* p = new cDisplayObject;

		p->EnableNameCard( true );
		if( !p->Create( &pInfo[i].PlayerInfo, &pInfo[i].WearInfo, &pInfo[i].WeaponInfo, 0, NiPoint3(0.0f, 0.0f, 0.0f)) )
		{
			assert(0);
			SAFE_DELETE(p);
			return;
		}
		p->SetStandIdx( pInfo[i].StandIdx );
		p->SetMapNum( pInfo[i].MapNum );

		// 삭제대기 시간값 세팅
		long waitTime = LONG_MAX;
		if( pInfo[i].DelWaitTime > 0 )
			waitTime = pInfo[i].DelWaitTime;

		p->SetDeleteWaitTime( waitTime );

		/// 관리 구조..
		if( mDisplayObjectMap.Insert( pInfo[i].StandIdx, p ) == false )
		{
			SAFE_DELETE(p);
			assert(0);
		}

		if( p )
		{
			p->SetPos( mCardDramaObj[ pInfo[i].StandIdx ]->GetDummyTrans() );
			p->SetRot( mCardDramaObj[ pInfo[i].StandIdx ]->GetDummyRot() );

			mCardDramaObj[ pInfo[i].StandIdx ]->SetDisplayObj( p );

			if( mCurrentSelectIndex == (unsigned int)pInfo[i].StandIdx )
			{
				NiTransform trans;
				trans.MakeIdentity();

				cEffectSceneNode* eff = p->SetLinkdEffect( eLINK_FOOT, "./data/map/character_select_effect.nif", &trans, false, true );
				if( eff )
					mSelectEffectIdx = eff->GetIndexByManger();

				bool delWait = ( p->GetDeleteWaitTime() != LONG_MAX );
				mpSelectUIManager->SetSelectInfo( (int)p->GetLevel(), p->GetName(), p->GetJob(), p->GetMapNum() );
				mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_StartButton, true, delWait );

				mDisplayObject = p;
			}
		}
	}

	unsigned int idx = mCurrentSelectIndex;
	for( unsigned int i=0; i<5; i++ )
	{
		if( mCardDramaObj[idx] ) 
		{
			mCardDramaObj[idx]->UpdateCard( mCardInitTime[i] );
			mCardDramaObj[idx]->SetFrameTime( mCardInitTime[i] );

			mCardDramaObj[idx]->SetCurrentPosIdx(i);
		}

		idx++;
		idx = idx%5;
	}

}

void cSelectStage::Msg_HeroSelectAck( MSGROOT* pMsg )
{
	MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsg;
	mpStageManager->SetMapIdx( pmsg->MapNum );
	mpStageManager->SetMapMode( pmsg->MapMode );

	mpStageManager->ChangeStage( eStage_Load );

	/// 스테이지 변경후 HERO객체 생성
	OBJECTMANAGER->AddHeroFromServerMsg( pMsg );
}

void cSelectStage::Msg_HeroSelectNack( MSGROOT* /*pMsg*/ )
{
#ifdef _DEBUG
	NiMessageBox( "NM_HERO_SELECT_NACK", "Error!!" );
#endif
}

void cSelectStage::ClearDisplayObject()
{
	mDisplayObject = 0;
	
	if( mDisplayObjectMap.GetSize() )
	{
		cDisplayObjectMap::cIterator i = mDisplayObjectMap.Begin();
		cDisplayObjectMap::cIterator end = mDisplayObjectMap.End();
		for( ; i != end; ++i )
		{
			cDisplayObject* p = (cDisplayObject*)(*i).mSecond;
			SAFE_DELETE( p );
		}

		mDisplayObjectMap.Clear();
	}

	for(unsigned int i=0;i<5; i++)
	{
		if( mCardDramaObj[i] )
			mCardDramaObj[i]->SetDisplayObj( 0 );
	}

	if( mSelectEffectIdx != UINT_MAX )
	{
		cEffectSceneNode* eff = SCENEMAN->GetEffectSceneNode( mSelectEffectIdx );
		if( eff )
			eff->Remove();

		mSelectEffectIdx = UINT_MAX;
	}
}

void cSelectStage::DeleteDisplayObject( unsigned int standIdx )
{
	cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt( standIdx );
	if( p == 0 )
		return;

	/// option file 삭제
	char name[MAX_NAME_BUFFER_SIZE];	/// 아이디
	ConvertToAscii( p->GetName(), name, MAX_NAME_BUFFER_SIZE );

	cString str;
	str.Format( "./Option/%s_%d.opc",  name, p->GetObjectID()%1000 );
	FileRemove( str );

	///
	mCardDramaObj[standIdx]->SetDisplayObj( 0 );

	if( mSelectEffectIdx != UINT_MAX )
	{
		cEffectSceneNode* eff = SCENEMAN->GetEffectSceneNode( mSelectEffectIdx );
		if( eff )
			eff->Remove();

		mSelectEffectIdx = UINT_MAX;
	}

	bool update = ( mDisplayObject == p );

	/// 객체 제거
	mDisplayObjectMap.Erase(standIdx);

	SAFE_DELETE(p);

	if( update == true )
	{
		mpSelectUIManager->ClearSelectInfo();
		mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_MakeButton, true, false );

		mDisplayObject = 0;
	}
}

void cSelectStage::CreateSelectMap( bool creatmap )
{
	NiDirectionalLight* light = LIGHTAGENT->GetSceneDirLight( 1 );
	light->SetDimmer( 0.1f );
	light->SetAmbientColor( NiColor(0.2f,0.2f,0.2f) );
	light->SetDiffuseColor( NiColor::BLACK );
	light->SetSpecularColor( NiColor::BLACK );

	NiMeshUpdateProcess kUpdateProcess;
	light->Update( kUpdateProcess );

	if( creatmap )
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

		if( n->GetNiObj() == 0 )
		{
			assert(0);
			return;
		}
		NiNode* orien = NiDynamicCast( NiNode, n->GetNiObj()->GetObjectByName("SelectCamera") );
		if( orien == 0 )
		{
			assert(0);
			return;
		}

		NiCamera* cam = 0;
		NiAVObject* child = 0;
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

	cCardSceneNodeParam cparam;
	cparam.mPathName = "./data/map/character_select_card.nif";

	bool init = false;
	for(unsigned char i=0;i<5; i++)
	{
		cparam.mCardIndex = i;

		if( mCardDramaObj[i] == 0 )
		{
			mCardDramaObj[i] = SCENEMAN->CreateCard( cparam );
			init = true;
		}

		if( mCardDramaObj[i]->GetCurrentPosIdx() == 0 )
			mCurrentSelectIndex = i;

		mCardDramaObj[i]->SetViewNode( true );
	}

	if( init )
	{
		for( unsigned int i=0; i<5; ++i )
		{
			if( mCardDramaObj[i] )
			{
				mCardDramaObj[i]->UpdateCard( mCardInitTime[i] );
				mCardDramaObj[i]->SetFrameTime( mCardInitTime[i] );
			}
		}
	}
}

void cSelectStage::SendDeleteWaitCharacter()
{
	if( mDisplayObject == 0 )
		return;

	if ( NETWORK->IsConnected( ) == true )
	{
		// 이미 삭제대기중
		if( mDisplayObject->GetDeleteWaitTime() != LONG_MAX )
			return;

		// 캐릭터 삭제 대기 요청
		MSG_REQ_CHARACTER_DELETE_WAIT msg;
		::memset( &msg, 0, sizeof(msg) );

		msg.Category	= NM_USER;
		msg.Protocol	= NM_USER_CHARACTER_DELETE_WAIT_REQ;
		msg.CharacterIdx	= mDisplayObject->GetObjectID();
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
				eMBEVENT_CHARACTER_DELETE_WAIT, eSKIN_NONE,
				GAMERESOURCEMAN->GetGameText( 16 ), 
				GAMERESOURCEMAN->GetGameText( 2 ) // 알림
				);
	}
}

void cSelectStage::SendDeleteCharacter( cDisplayObject* p )
{
	if( p == 0 )
		return;

	if ( NETWORK->IsConnected( ) == true )
	{
		MSG_REQ_CHARACTER_DELETE msg;
		/// INIT MSGROOT.
		::memset( &msg, 0, sizeof(msg) );
		/// SET MSGROOT [CATEGORY & PROTOCOL].
		msg.Category     = NM_USER;
		msg.Protocol     = NM_USER_CHARACTER_DELETE_REQ;
		/// SET MSG_REQ_COMEIN_GAME.
		msg.CharacterIdx = p->GetObjectID();
		/// SEND MSGROOT
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// Msgbox 출력
		UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_NONE,
			GAMERESOURCEMAN->GetGameText( 16 ), // 캐릭터 목록 전송
			GAMERESOURCEMAN->GetGameText( 2 ) // 알림
			);

		SetEnableWindow( false );

		p->SetDeleteLock( true );
	}
}

void cSelectStage::SendConnect( long channelNum, bool istutorial )
{
	if( mDisplayObject == 0 )
		return;

	if ( NETWORK->IsConnected( ) == true )
	{
		/// 선택한 채널 저장
		STAGEMAN->SetChannelIdx( channelNum );

		/// 070404 추가 - 곽철중 [채널리스트 요청]
		MSG_REQ_COMEIN_GAME msg;
		/// INIT MSGROOT.
		memset( &msg, 0, sizeof(msg) );
		/// SET MSGROOT [CATEGORY & PROTOCOL].
		msg.Category     = NM_USER;
		msg.Protocol     = NM_USER_COME_IN_GAME_REQ;
		/// SET MSG_REQ_COMEIN_GAME.
		msg.NumOfChannel = channelNum;
		msg.CharacterIdx = mDisplayObject->GetObjectID();
		msg.Tutorial = ( istutorial == true );
		
		/// SEND MSGROOT
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 연결 대기창 등록
		UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
			eMBEVENT_CONNECT_GAMESRV, eSKIN_NONE,
			GAMERESOURCEMAN->GetGameText( 0 ), // 캐릭터 목록 전송
			GAMERESOURCEMAN->GetGameText( 2 ) // 알림
			);

		//
		SetEnableWindow( false );
	}
}

void cSelectStage::PlayCard()
{
	for( unsigned int i=0;i<5; i++)
	{
		if( mCardDramaObj[i] != 0 )
		{
			mCardDramaObj[i]->Play( THEAPP->GetWorldAccumTime() );
		}
	}
}

void cSelectStage::RewCard()
{
	for( unsigned int i=0;i<5; i++)
	{
		if( mCardDramaObj[i] != 0 )
			mCardDramaObj[i]->Rew( THEAPP->GetWorldAccumTime() );
	}
}

void cSelectStage::MoveCard( bool rew )
{
	if( rew )
	{
		if( mCardMoveState == cCardSceneNode::eCard_Play )
			return;

		if( mCardMoveState == cCardSceneNode::eCard_Stop )
			mCardMoveState = cCardSceneNode::eCard_Rew;
	}
	else
	{
		if( mCardMoveState == cCardSceneNode::eCard_Rew )
			return;

		if( mCardMoveState == cCardSceneNode::eCard_Stop )
			mCardMoveState = cCardSceneNode::eCard_Play;
	}

	mCardMoveCount++;
	if( mCardMoveCount > 3 )
		mCardMoveCount = 3;

	if( mCardMoveCount == 1 )
	{
		mCardMoving = true;

		if( mSelectEffectIdx != UINT_MAX )
		{
			cEffectSceneNode* eff = SCENEMAN->GetEffectSceneNode( mSelectEffectIdx );
			if( eff )
				eff->Remove();

			mSelectEffectIdx = UINT_MAX;
		}
/*
		/// effect clear
		for( unsigned int i=0; i<5; i++ )
		{
			cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt(i);
			if( p )
				p->ClearEffect();
		}
*/
		/// reset window info
		mpSelectUIManager->ClearSelectInfo();
		mpSelectUIManager->SetSelectWindowFrame( cSelectWindow::eShow_None, false, false );
	}

	mDisplayObject = 0;
}

void cSelectStage::MoveMakeStage()
{
	STAGEMAN->ChangeStage( eStage_Make );

	cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
	if( p )
		p->SetMakeStandIndex( mCurrentSelectIndex );
}

void cSelectStage::ClearSelectMap()
{
	for( unsigned int i=0;i<5; i++)
		mCardDramaObj[i] = 0;
}

void cSelectStage::CancelChannel()
{
	mpSelectUIManager->VisibleListState();
}

void cSelectStage::MsgBoxEventParser( unsigned int event, unsigned int result, void* exData )
{
	switch( event )
	{
	case eMBEVENT_CHARACTER_DELETE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			SendDeleteWaitCharacter();
		}
		break;
	case eMBEVENT_CHARACTER_DELETE_CANCEL:
		if( result == eMESSAGE_BUTTON_YES )
		{
			if ( NETWORK->IsConnected( ) == true )
			{
				char standIdx = (char)(exData);
				if( standIdx >= 5 )
					assert(0);

				cDisplayObject* p = (cDisplayObject*)mDisplayObjectMap.GetAt(standIdx);
				if( p )
				{
					// 캐릭터 삭제 대기 요청
					MSG_REQ_CHARACTER_DELETE_CANCEL msg;
					::memset( &msg, 0, sizeof(msg) );

					msg.Category	= NM_USER;
					msg.Protocol	= NM_USER_CHARACTER_DELETE_CANCEL_REQ;
					msg.CharacterIdx	= p->GetObjectID();
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
						eMBEVENT_CHARACTER_DELETE_CANCELWAIT, eSKIN_NONE,
						GAMERESOURCEMAN->GetGameText( 164 ), 
						GAMERESOURCEMAN->GetGameText( 2 ) // 알림
						);
				}
			}
			else
				assert(0);
		}
		break;
	case eMBEVENT_CONNECT_DEAD:
		{
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	case eMBEVENT_TUTORIAL_SELECT:
		{
			bool tutorial = ( result == eMESSAGE_BUTTON_YES );
			long channelNum = (long)exData;
			if( channelNum != -1 )
			{
				SendConnect( channelNum, tutorial );
			}
			else
				assert(0);
		}
		break;
	case eMBEVENT_VIDEO_CAPTURE:
		{
			if( result == eMESSAGE_BUTTON_YES )
			{
				if( RENDERSYS )
					RENDERSYS->ToggleVideoCapture();
			}
		}
		break;
	//case eMBEVENT_CHARACTER_CREATE_LIMIT:
	//	{
	//		mChangeLogin = true;
	//	}
	//	break;
	}
}

void cSelectStage::SetEnableWindow( bool enable )
{
	if( mpSelectUIManager )
		mpSelectUIManager->SetEnableWindow( enable );
}
