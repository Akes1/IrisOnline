#include "stdafx.h"
#include "MakeStage.h"
#include "StageManager.h"

#include "MakeUIManager.h"
#include "MakeWindow.h"

#include "DisplayObject.h"
#include "SceneManager.h"
#include "LightAgent.h"

#include "inputsystem.h"
#include "GameResourceManager.h"
#include "CameraManager.h"

#include "RenderSystem.h"

cMakeStage::cMakeStage()
: mpMakeUIManager(0)
, mpMakeCharacter(0)
, mViewState(0)
, mCurrentSelectIndex(0)
, mUpdateDisplayObj(false)
{
	mIsZoom = false;
	mZoomFlag = eZoom_None;
	mCameraDefaultPos = NiPoint3::ZERO;
	mDummyPos = NiPoint3::ZERO;
}

cMakeStage::~cMakeStage()
{
	SAFE_DELETE(mpMakeCharacter);

	for( unsigned int i=0; i< mViewItemArray.GetSize(); i++ )
	{
		sViewItem* p = (sViewItem*)mViewItemArray[i];
		SAFE_DELETE(p);
	}
	mViewItemArray.Clear();
}

void cMakeStage::Init( cStageManager* pStageManager )
{
	mpStageManager = pStageManager;
}

void cMakeStage::InitStage()
{
	if( mInit )	return;

	/// UI
	mpMakeUIManager = new cMakeUIManager;
	mpMakeUIManager->Init();

	mInit = true;

	if( LoadDisplayItemInfo() == false )
	{
		assert(0);
	}
}

void cMakeStage::Exit()
{
	/// UI
	SAFE_DELETE( mpMakeUIManager );

	mpStageManager = 0;
}

void cMakeStage::Open()
{
	NiDirectionalLight* light = LIGHTAGENT->GetSceneDirLight( 1 );
	light->SetDimmer( 0.1f );
	light->SetAmbientColor( NiColor(0.2f,0.2f,0.2f) );
	light->SetDiffuseColor( NiColor::BLACK );
	light->SetSpecularColor( NiColor::BLACK );

	NiMeshUpdateProcess kUpdateProcess;
	light->Update( kUpdateProcess );

	mUpdateDisplayObj = false;

	/// UI
	mpMakeUIManager->Open();

	sPlayerInfo sBaseInfo;
	sPlayerWearInfo sWearInfo;
	sPlayerWeaponInfo sWeaponInfo;

	NiCamera* cam = CAMERAMAN->GetCurrentNi();

	NiNode* node = CAMERAMAN->GetCurrent()->GetOrientNode();
	mCameraDefaultPos = node->GetTranslate();

	NiPoint3 pos = cam->GetWorldTranslate();
	pos = pos + cam->GetWorldDirection()* 500.0f;
	pos.z = 0.0f;

	UpdateDisplayEquipInfo( &sBaseInfo, &sWearInfo, &sWeaponInfo );
	mpMakeCharacter = new cDisplayObject;
	if( !mpMakeCharacter->Create( &sBaseInfo, &sWearInfo, &sWeaponInfo, 0, 
		pos, NiMatrix3::IDENTITY, 1.2f ) )
	{
		assert(0);
		mpStageManager->ChangeStage( eStage_Select );
		return;
	}

	mpMakeCharacter->GetDummyPos( eLINK_HEAD, mDummyPos );
	mpMakeCharacter->ChangeAnimation( ANITYPE_PEACE_IDLE );

	mIsZoom = false;
	mZoomFlag = eZoom_None;
}

void cMakeStage::Close()
{
	NiNode* node = CAMERAMAN->GetCurrent()->GetOrientNode();
	node->SetTranslate( mCameraDefaultPos );

	mIsZoom = false;
	mZoomFlag = eZoom_None;

	mCameraDefaultPos = NiPoint3::ZERO;
	mDummyPos = NiPoint3::ZERO;

	/// UI
	mpMakeUIManager->Close();
	SAFE_DELETE(mpMakeCharacter);
}

void cMakeStage::Process( unsigned long deltaTime, unsigned long accumTime )
{
	/// UI
	mpMakeUIManager->Process( accumTime );

	if( mpMakeCharacter )
	{
		unsigned char state = mpMakeUIManager->CheckDisplayUpdate();
		if( state == eChange_Form )
		{
			sPlayerInfo sBaseInfo;
			sPlayerWearInfo sWearInfo;
			sPlayerWeaponInfo sWeaponInfo;
			UpdateDisplayEquipInfo( &sBaseInfo, &sWearInfo, &sWeaponInfo );

			/// 외적인 것들은 추가 변경
			mpMakeCharacter->UpdateDisplayForm( &sBaseInfo, &sWearInfo, &sWeaponInfo );
		}
		else if( state == eChange_Object )
		{
			sPlayerInfo sBaseInfo;
			sPlayerWearInfo sWearInfo;
			sPlayerWeaponInfo sWeaponInfo;
			UpdateDisplayEquipInfo( &sBaseInfo, &sWearInfo, &sWeaponInfo );

			NiCamera* cam = CAMERAMAN->GetCurrentNi();
			NiPoint3 pos = mCameraDefaultPos;
			pos = pos + cam->GetWorldDirection()* 500.0f;
			pos.z = 0.0f;

			if( mpMakeCharacter->ReCreate( &sBaseInfo, &sWearInfo, &sWeaponInfo, 
				pos, NiMatrix3::IDENTITY, 1.2f ) == false )
			{
				assert(0);
				return;
			}

			if( mZoomFlag == eZoom_In )
			{
				mpMakeCharacter->GetDummyPos( eLINK_HEAD, mDummyPos );
				NiPoint3 dir = mDummyPos - mCameraDefaultPos;
				dir.Unitize();

				NiNode* node = CAMERAMAN->GetCurrent()->GetOrientNode();
				node->SetTranslate( mCameraDefaultPos + dir*mZoomValue );
			}

			mpMakeCharacter->ChangeAnimation( ANITYPE_PEACE_IDLE );
		}
		mpMakeCharacter->Update( deltaTime, accumTime );

		if( mZoomFlag == eZoom_In )
		{
			mZoomValue += deltaTime*1.5f;
			if( mZoomValue > 320.0f )
			{
				mZoomValue = 320.0f;
				mZoomFlag = eZoom_None;
				mpMakeUIManager->SetEnableWindow( true );
			}

			NiPoint3 dir = mDummyPos - mCameraDefaultPos;
			dir.Unitize();

			NiNode* node = CAMERAMAN->GetCurrent()->GetOrientNode();
			node->SetTranslate( mCameraDefaultPos + dir*mZoomValue );
		}
		else if( mZoomFlag == eZoom_Out )
		{
			mZoomValue -= deltaTime*1.5f;
			if( mZoomValue < 0.0f )
			{
				mZoomValue = 0.0f;
				mZoomFlag = eZoom_None;
				mpMakeUIManager->SetEnableWindow( true );
			}

			NiPoint3 dir = mDummyPos - mCameraDefaultPos;
			dir.Unitize();

			NiNode* node = CAMERAMAN->GetCurrent()->GetOrientNode();
			node->SetTranslate( mCameraDefaultPos + dir*mZoomValue );
		}
	}
}

void cMakeStage::NetworkMsgParser( unsigned long /*connectionindex*/, char* pMsg, unsigned long /*length*/ )
{
	MSGROOT* pTemMsg = (MSGROOT*)pMsg;
	switch( pTemMsg->Category )
	{
	case NM_USER:
		{
			switch( pTemMsg->Protocol )
			{
			case NM_USER_CHARACTER_NAME_RES:
				{
					MSG_ERROR* msg = (MSG_ERROR*)pMsg;
					if( msg->ErrorCode == 0 )
					{
						/// 성공
						UIMAN->SendEventTopMsgBox( true );

						UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 46 ), 
							GAMERESOURCEMAN->GetGameText( 2 )
							);
					}
					else
					{
						UIMAN->SendEventTopMsgBox( false );

						/// 실패
						UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 44 ), 
							GAMERESOURCEMAN->GetGameText( 2 )
							);
					}
				}
				break;
			case NM_USER_CHARACTER_CREATE_RES:
				{
					MSG_ERROR* msg = (MSG_ERROR*)pMsg;

					mpMakeUIManager->SetEnableWindow( true );

					if( msg->ErrorCode == 0 )
					{
						/// 성공

						UIMAN->SendEventTopMsgBox( true );
						mpStageManager->ChangeStage( eStage_Select );
					}
					else
					{
						/// 실패

						UIMAN->SendEventTopMsgBox( false );
						Msg_UserCharacterCreateNack( pTemMsg );		
					}
				}
			}
		}
		break;
	}
}

void cMakeStage::MsgBoxEventParser( unsigned int event, unsigned int result, void* exData )
{
	switch( event )
	{
	case eMBEVENT_VIDEO_CAPTURE:
		if( result == eMESSAGE_BUTTON_YES )
		{
			if( RENDERSYS )
				RENDERSYS->ToggleVideoCapture();
		}
		break;
	}
}

/*
#define ERROR_CHARACTER_CREATE_SUCCESS		0	/// 성공.
#define ERROR_CHARACTER_CREATE_EXIST		1	/// 실패-중복.
#define ERROR_CHARACTER_CREATE_OVER			2	/// 실패-더이상 생성할 수 없음.
#define ERROR_CHARACTER_CREATE_STAND_IDX	3	/// 실패-STAND IDX 오류.
#define ERROR_CHARACTER_CREATE_ITEMS		4	/// 실패-기본 아이템 지급 오류.
#define ERROR_CHARACTER_CREATE_SKILLS		5	/// 실패-기본 스  킬 지급 오류.
#define ERROR_CHARACTER_CREATE_RULE			6	/// 실패-생성 룰 오류(이름,종족,성별,직업,헤어,얼굴,헤어칼라...).
#define ERROR_CHARACTER_CREATE_FAIL			7	/// 실패.
*/
void cMakeStage::Msg_UserCharacterCreateNack( MSGROOT* pMsg )
{
	MSG_ERROR* pmsg = (MSG_ERROR*)pMsg;
	switch( pmsg->ErrorCode )
	{
	case ERROR_CHARACTER_CREATE_OVER:
		{
			UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 23 ), 
				GAMERESOURCEMAN->GetGameText( 2 )
				);

//			::MessageBox( NULL, _T("Full CharacterSlot"), _T("Error"), MB_OK );
			return;
		}
		break;
	case ERROR_CHARACTER_CREATE_EXIST:
		{
			UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 44 ), 
				GAMERESOURCEMAN->GetGameText( 2 )
				);

//			::MessageBox( NULL, _T("Already CharacterName"), _T("Error"), MB_OK );
			return;
		}
		break;
	default:
		{
			cStringT Message;
			Message.Format( GAMERESOURCEMAN->GetGameText( 45 ), pmsg->ErrorCode );

			UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				Message.Cstr(), 
				GAMERESOURCEMAN->GetGameText( 2 )
				);
		}
		return;
	}
}

bool cMakeStage::LoadDisplayItemInfo()
{
	/// 파일 열기
	cFileLoader loader;
	cString pathName = "./Script/Resource/MakeStageViewItem.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MakeStageViewItem.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;
	sViewItem* pViewItem = 0;

	while( tokenizer.IsEnd() == false )
	{
		pViewItem = new sViewItem;

		/// 직업 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto error; }
		pViewItem->mJobIdx = str.ToInt();

		// Item 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto error; }
		pViewItem->mItemIdx = str.ToInt();

		/// 인벤번호
		if( tokenizer.GetNext( &str ) == false )	{ goto error; }
		pViewItem->mInvenNum = (unsigned short)str.ToInt();

		mViewItemArray.PushBack( pViewItem );
		pViewItem = 0;
	}
	return true;

error:
	SAFE_DELETE(pViewItem);
	return false;
}

void cMakeStage::UpdateDisplayEquipInfo( sPlayerInfo* pBaseOut, sPlayerWearInfo* pWearOut, sPlayerWeaponInfo* pWeaponOut )
{
	assert(pBaseOut);
	assert(pWearOut);
	assert(pWeaponOut);

	if( !mpMakeUIManager )
	{
		assert(0);
		return;
	}

	/// make window - 기본 정보 획득
	mpMakeUIManager->GetDisplayInfo( pBaseOut );
	if( mpMakeCharacter )
	{
		mpMakeCharacter->SetRace( pBaseOut->Race );
		mpMakeCharacter->SetGender( pBaseOut->Gender );
		mpMakeCharacter->SetJob(  pBaseOut->Job );
	}

	/// 기본 정보를 토대로 착용 장비 셋팅
	for( unsigned int i=0; i< mViewItemArray.GetSize(); i++ )
	{
		sViewItem* p = (sViewItem*)mViewItemArray[i];
		if( p->mJobIdx == (unsigned long)pBaseOut->Job )
		{
			switch( p->mInvenNum )
			{
			case INVENTORY_WEAR_BODY1:	pWearOut->WearIdx[eWEAR_BODY1] = p->mItemIdx;		break;
			case INVENTORY_WEAR_BODY2:	pWearOut->WearIdx[eWEAR_BODY2] = p->mItemIdx;		break;
			case INVENTORY_WEAR_HAND:	pWearOut->WearIdx[eWEAR_HAND] = p->mItemIdx;		break;
			case INVENTORY_WEAR_FOOT:	pWearOut->WearIdx[eWEAR_FOOT] = p->mItemIdx;		break;
			case INVENTORY_HAND_LEFT2:	pWeaponOut->WeaponIdx[eHAND_LEFT] = 0;				break;
			case INVENTORY_HAND_RIGHT2:	pWeaponOut->WeaponIdx[eHAND_RIGHT] = 0;				break;
			}
		}
	}
}

void cMakeStage::ProcessLButtonDClick( cMouseAgent* pMouse )
{
	pMouse;
/*
	int mouseX = pMouse->GetMouseEventX();//pMouse->GetMouseX();
	int mouseY = pMouse->GetMouseEventY();//pMouse->GetMouseY();

	/// 한번만 검사하도록..
	mCollidableArray.Clear();
	if( SCENEMAN->PickDynamics(&mCollidableArray, mouseX, mouseY, true) == true )
	{
		cSceneNode* n = (cSceneNode*)mCollidableArray[0];

		if( n->GetType() != cSceneNode::eCARDDRAMA )
			return;

		cSelectStageCardSceneNode* p = (cSelectStageCardSceneNode*)n;
		unsigned int index = p->GetOverIndex();
		if( index != 0 )
		{
			switch( mViewState )
			{
			case eVIEW_RACE:
				{
					if( index >= 26 && index <= 28 )
					{
						unsigned char race = (unsigned char)(index - 26);
						if( race != mpMakeCharacter->GetRace() )
						{
							mpMakeCharacter->SetRace( race );
							mUpdateDisplayObj = true;
						}

						mCardDramaObject->SetMakeCardImage( race );

						SetViewState( eVIEW_GENDER_CREATING );
					}
					else
					{
						STAGEMAN->ChangeStage( eStage_Select );
					}
				}
				break;
			case eVIEW_GENDER:
				{
					if( index >= 29 && index <= 30 )
					{
						unsigned char gender = (unsigned char)index -29;
						if( gender != mpMakeCharacter->GetGender() )
						{
							mpMakeCharacter->SetGender( gender );
							mUpdateDisplayObj = true;
						}

						SetViewState( eVIEW_JOB_CREATING );
					}
					else
					{
						SetViewState( eVIEW_RACE_RETURNING );
					}
				}
				break;
			case eVIEW_JOB:
				{
					if( index >= 31 && index <= 32 )
					{
						unsigned char race = mpMakeCharacter->GetRace();

						tArray<unsigned long> jobArray;
						GAMERESOURCEMAN->GetDefaultJob( race, &jobArray );

						if( mpMakeCharacter->GetJob() != (ePLAYER_JOB)jobArray[index-31] )
						{
							mpMakeCharacter->SetJob( (ePLAYER_JOB)jobArray[index-31] );
							mUpdateDisplayObj = true;
						}

						mCardDramaObject->SetCardImage( 4, race, mpMakeCharacter->GetGender() );

						SetViewState( eVIEW_DETAIL_CREATING );
					}
					else
					{
						SetViewState( eVIEW_GENDER_RETURNING );
					}
				}
				break;
			case eVIEW_DETAIL:
				{
					if( index >= 26 && index <= 32 )
					{
						SetViewState( eVIEW_JOB_RETURNING );
					}
				}
				break;
			}
		}

	}
		*/
	return;
}

void cMakeStage::ProcessMouseover( unsigned long /*time*/, cMouseAgent* pMouse )
{
	int dx = pMouse->GetDeltaX();

	if( pMouse->RButtonPressed() )
	{
		float rspeed = 0.3f;//50.0f * time;
		if( dx )
		{
			float angle = float(dx) * rspeed * NI_PI / 180.f;

			if( mpMakeCharacter )
				mpMakeCharacter->Rotate( -angle );
		}
	}
/*
	int mouseX = pMouse->GetMouseX();
	int mouseY = pMouse->GetMouseY();

	mCollidableArray.Clear();
	if( SCENEMAN->PickDynamics(&mCollidableArray, mouseX, mouseY, true) == true )
	{
		cSceneNode* n = (cSceneNode*)mCollidableArray[0];
	}
*/
}

void cMakeStage::RotateMakeCharacter( int dx )
{
	float rspeed = 0.3f;
	if( dx )
	{
		float angle = float(dx) * rspeed * NI_PI / 180.f;

		if( mpMakeCharacter )
			mpMakeCharacter->Rotate( -angle );
	}
}

void cMakeStage::SendMakeCharacter( LPCTSTR name, unsigned char gender, unsigned char hair, unsigned char hairColor, unsigned char face )
{
	if( ::_tcslen( name ) == 0 )
		return;

	MSG_REQ_CHARACTER_CREATE Msg;			
	Msg.Category = NM_USER;
	Msg.Protocol = NM_USER_CHARACTER_CREATE_REQ;

	Sstrncpy( Msg.Name, MAX_NAME_BUFFER_SIZE, name, MAX_NAME_SIZE );

	Msg.Hair = hair;
	Msg.Face = face;
	Msg.Gender = gender;
	Msg.HairColor = hairColor;

	Msg.Race = mpMakeCharacter->GetRace();
	Msg.Job = (unsigned short)mpMakeCharacter->GetJob();

	Msg.StandIdx = mpMakeCharacter->GetStandIdx();

	NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

	/// Msgbox 출력
	UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
		eMBEVENT_NONE, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 15 ), // 캐릭터 생성중..
		GAMERESOURCEMAN->GetGameText( 2 ) // 알림
		);

	mpMakeUIManager->SetEnableWindow( false );
}

void cMakeStage::SetMakeStandIndex( unsigned int index )
{
	if( index < 0 && index > 4 )
	{
		assert(0);
		return;
	}

	if( mpMakeCharacter == 0 )
	{
		assert(0);
		return;
	}

	mpMakeCharacter->SetStandIdx( (char)index );
}

void cMakeStage::ToggleZoom()
{
	mIsZoom = !mIsZoom;

	if( mIsZoom == true )
	{
		mZoomFlag = eZoom_In;
		mZoomValue = 0.0f;
	}
	else
	{
		mZoomFlag = eZoom_Out;
		mZoomValue = 320.0f;
	}

	mpMakeUIManager->SetEnableWindow( false );
}
