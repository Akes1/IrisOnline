#include "stdafx.h"
#include "TutorialManager.h"
#include "Protocol.h"
#include "UIMsgboxEvent.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Tutorial_Common.h"
#include "ChatManager.h"
#include "UIManager.h"
#include "GameResourceManager.h"
#include "PlaneObject.h"
#include "AniPlane.h"
#include "RenderSystem.h"
#include "ItemManager.h"
#include "WorldManager.h"
#include "InputSystem.h"
#include "Npc.h"
#include "Monster.h"

cTutorialMode::cTutorialMode()
: mModeIndex(UINT_MAX)
, mStartPosIndex(0)
, mDieTextIndex(0)
{

}

cTutorialMode::~cTutorialMode()
{
	//
	cTutorialObjectImageMap::cIterator ob = mObjectImageMap.Begin();
	cTutorialObjectImageMap::cIterator oend = mObjectImageMap.End();
	for( ; ob != oend; ++ob )
	{
		sTutorialBaseImage* p = (sTutorialBaseImage*)(ob->mSecond);
		SAFE_DELETE( p );
	}
	mObjectImageMap.Clear();

	//
	for( unsigned int i = 0; i < mObjectArr.GetSize(); ++i )
	{
		sTutorialObject* p = (sTutorialObject*)mObjectArr[i];
		SAFE_DELETE( p );
	}
	mObjectArr.Clear();

	//
	for( unsigned int i = 0; i < mImageArr.GetSize(); ++i )
	{
		sTutorialImage* p = (sTutorialImage*)mImageArr[i];
		SAFE_DELETE( p );
	}
	mImageArr.Clear();
}

///////////////////////////////////////////////////////////
cTutorialManager* cTutorialManager::mpSingleton = NULL;

cTutorialManager::cTutorialManager()
: mModeIndex(-1)
{
	mpSingleton = this;
}

cTutorialManager::~cTutorialManager()
{
	cModeMap::cIterator i = mModeMap.Begin();
	cModeMap::cIterator end = mModeMap.End();

	for( ; i != end; ++i )
	{
		cTutorialMode* define = (cTutorialMode*)(i->mSecond);
		SAFE_DELETE( define );
	}
	mModeMap.Clear();

	//
	mTextureMap.Clear();

	//
	mpSingleton = 0;
}

void cTutorialManager::Clear()
{
	ClearOldMode();

	mModeIndex = -1;
}

bool cTutorialManager::Init()
{
	if( LoadScript( "./Script/Resource/tutorialmode.txt"  ) == false )
	{
		assert( 0 && "failed to read tutorial script" );
		return false;
	}
	return true;
}

void cTutorialManager::Open()
{
	// 세팅
}

void cTutorialManager::Close()
{
	Clear();
}

void cTutorialManager::Process( unsigned long deltatime, unsigned long accumtime )
{
	if( IsTutorialMode() == false )
		return;

	if( mModeIndex == -1 )
		return;

	// 키 인풋 체크
	for( unsigned int i = 0; i < mImageIDArr.GetSize(); ++i )
	{
		cImageDataPair& pair = mImageIDArr[i];
		unsigned long id = pair.mFirst;
		sTutorialImage* data = (sTutorialImage*)pair.mSecond;
		if( data && data->mKey > 0 )
		{
			cPlaneObject* image = UIMAN->GetImage( id );
			if( !image )
				continue;

			switch( data->mDramaInfo )
			{
			case eDRAMA_DOWN:
				{	
					unsigned short w = data->mWidth;
					unsigned short h = data->mHeight;

					if( KEYBOARD->GetKeyPressed( data->mKey ) == true )
					{
						// 눌림 처리
						unsigned short tx = data->mDownTexX;
						unsigned short ty = data->mDownTexY;
						image->SetTextureRect( tx, ty, tx+w, ty+h );

						data->mIsDowned = true;
					}
					else
					{
						// 기존에 눌려있었으면 풀어낸다
						if( data->mIsDowned == true )
						{
							unsigned short tx = data->mTexPosX;
							unsigned short ty = data->mTexPosY;
							image->SetTextureRect( tx, ty, tx+w, ty+h );

							data->mIsDowned = false;
						}
					}
				}
				break;
			case eDRAMA_SHOW:
				if( KEYBOARD->GetKeyPressed( data->mKey ) == true )
				{
					UIMAN->UpdateImage( id, true );
				}
				break;
			case eDRAMA_HIDE:
				if( KEYBOARD->GetKeyPressed( data->mKey ) == true )
				{
					UIMAN->UpdateImage( id, false );
				}
				break;
			}
		}
	}
}

bool cTutorialManager::IsTutorialMode()
{
	if( HERO && HERO->GetMapNum() == MAP_TUTORIAL )
		return true;
	return false;
}

void cTutorialManager::ClearOldMode()
{
	for( unsigned int i = 0; i < mImageIDArr.GetSize(); ++i )
	{
		cImageDataPair& data = mImageIDArr[i];
		unsigned long id = data.mFirst;

		UIMAN->DeleteImage( id );
	}
	mImageIDArr.Clear();

	// 클리어
	OBJECTMAN->ClearTutorialImage();
	ITEMMAN->ClearTutorialDrop();
}

// 모드 변경
void cTutorialManager::ChangeMode( int modeIndex )
{
	if( mModeIndex == modeIndex )
		return;

	// 이전 모드관련 데이터 클리어
	ClearOldMode();

	mModeIndex = modeIndex;

	if( modeIndex == -1 )
		return;

	cTutorialMode* newMode = GetTutorialMode( modeIndex );
	if( !newMode )
	{
 		assert(0);
		return;
	}

	// 셋팅
	// 화면 이미지 생성
	unsigned int screenWidth = RENDERSYS->GetScreenWidth();
	unsigned int screenHeight = RENDERSYS->GetScreenHeight();

	for( unsigned int i = 0; i < newMode->mImageArr.GetSize(); ++i )
	{
		sTutorialImage* data = (sTutorialImage*)newMode->mImageArr[i];
		if( data )
		{
			unsigned short tx = data->mTexPosX;
			unsigned short ty = data->mTexPosY;
			unsigned short w = data->mWidth;
			unsigned short h = data->mHeight;

			cPlaneObject* image = 0;

			if( data->mFrameCount > 1 )
			{
				cBaseAniPlane* aniPlane = new cBaseAniPlane;
				aniPlane->SetAnimationInfo( data->mFrameCount, data->mOneFrameTime, data->mRepeat );
				aniPlane->BeginAnimation();

				image = aniPlane;
			}
			else
			{
				image = new cPlaneObject;
			}

			int x = 0;
			int y = 0;

			if( data->mLeftTop == true )
			{
				x = ( screenWidth * data->mPosX ) / DEFAULT_SCREENWIDTH;
				y = ( screenHeight * data->mPosY ) / DEFAULT_SCREENHEIGHT;
			}
			else
			{
				// right bottom 기준임
				x = ( screenWidth - data->mPosX );
				y = ( screenHeight - data->mPosY );
			}

			// 정렬 정보 셋팅
			if( data->mAlign == 1 )
			{
				// width 
				x = (int)(( screenWidth - w ) * 0.5f);
			}
			else if( data->mAlign == 2 )
			{
				// height 
				y = (int)(( screenHeight - h ) * 0.5f);
			}
			else if( data->mAlign == 3 )
			{
				// width & height
				x = (int)(( screenWidth - w ) * 0.5f);
				y = (int)(( screenHeight - h ) * 0.5f);
			}

			// 현재 사이즈에 맞추어 x y 삽입
			if( image->Create( data->mpTexture, x, y, w, h, tx, ty, tx + w, ty + h ) == false )
			{
				assert( 0 && "failed to create mode image");
				SAFE_DELETE( image );
				continue;
			}	

			// 화면사이즈가 갱신될것을 고려해 mPosX, mPosY 삽입
			unsigned long ID = UIMAN->AddImage( image, data->mDefaultShow, data->mPosX, data->mPosY, data->mAlign, data->mLeftTop );
			if( ID == ULONG_MAX )
			{
				assert( 0 && "failed to add mode image");
				SAFE_DELETE( image );
				continue;
			}

			mImageIDArr.PushBack( cImageDataPair(ID, data) );
		}
		else
		{
			assert(0);
		}
	}

	// 오브젝트 이미지 셋팅
	unsigned int size = newMode->mObjectArr.GetSize();
	if( size > 0 )
	{
		for( unsigned int i = 0; i < size; ++i )
		{
			sTutorialObject* obj = (sTutorialObject*)newMode->mObjectArr[i];
			if( obj )
			{
				switch( obj->mObjectType )
				{
				case eOBJECTTYPE_NPC:
				case eOBJECTTYPE_MONSTER:
					OBJECTMAN->UpdateTutorialImage( obj->mObjectType );
					break;
				case eOBJECTTYPE_ITEM:
					ITEMMAN->UpdateTutorialDrop();
					break;
				}
			}
		}
	}
}

void cTutorialManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
	case NM_TUTORIAL_GIVEUP_RES:
		{
			/// 맵이동 성공/실패 처리
			MSG_ERROR* pmsg = (MSG_ERROR*)msg;
			switch( pmsg->ErrorCode )
			{
			case ERROR_TUTORIAL_GIVEUP_SUCCESS:
				break;
			case ERROR_TUTORIAL_GIVEUP_MAP:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 98 ) );
				}
				break;
			case ERROR_TUTORIAL_GIVEUP_TUTORIAL:
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11000 ) );
				}
				break;
			}

			if( pmsg->ErrorCode != ERROR_TUTORIAL_GIVEUP_SUCCESS )
			{
				HERO->SetStopFlag( eSTOP_NONE );
				UIMAN->DestroyMsgBox( eMBEVENT_MAPCHANGEWAIT );
			}
		}
		break;
	case NM_TUTORIAL_CHANGEMODE_SYN:
		{
			MSG_SYN_TUTORIAL_CHANGEMODE* pmsg = (MSG_SYN_TUTORIAL_CHANGEMODE*)msg;

			// 모드 변경
			ChangeMode( pmsg->mModeIndex );
		}
		break;
	case NM_TUTORIAL_REVIVAL_RES:
		{
			MSG_RES_TUTORIAL_RESURRECTION* pmsg = (MSG_RES_TUTORIAL_RESURRECTION*)msg;
		
			if( pmsg->ErrorCode == ERROR_TUTORIAL_RESURRECTION_SUCCESS )
			{
				// 성공
				if( HERO->GetState() != eOBJECT_STATE_DIE )
				{
					assert(NULL);
				}

				HERO->SetState( eOBJECT_STATE_IDLE );
				HERO->HPChange( pmsg->mHP, pmsg->mMaxHP );
				HERO->MPChange( pmsg->mMP, pmsg->mMaxMP );
				HERO->FixPos( pmsg->mPosX, pmsg->mPosY, true );
				HERO->UpdateHeroInfo();

				//
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 904 ) );
			}
			else
			{
				// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 903 ) );
			}
		}
		break;
	case NM_TUTORIAL_REVIVAL_SYN:
		{
			MSG_SYN_TUTORIAL_RESURRECTION* pmsg = (MSG_SYN_TUTORIAL_RESURRECTION*)msg;

			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pmsg->mCharacterIdx );
			if( pPlayer == NULL )
			{
				assert(NULL);
				return;
			}
	
			if( pPlayer->GetState() != eOBJECT_STATE_DIE )
			{
				assert(NULL);
				return;
			}

			pPlayer->SetState( eOBJECT_STATE_IDLE );
			pPlayer->HPChange( pmsg->mHP, pmsg->mMaxHP );
			pPlayer->MPChange( pmsg->mMP, pmsg->mMaxMP );

			NiPoint3 pos( pmsg->mPosX, pmsg->mPosY, 0.0f );
			if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
			{
				assert(0);
				return;
			}

			pPlayer->SetPos( pos );
		}
		break;
	}
}

void cTutorialManager::UpdateTarget( char oldtype, unsigned long oldobjectIdx, char type, unsigned long objectIdx )
{
	// 기존 릴리즈
	switch( oldtype )
	{
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* mon = OBJECTMAN->GetMonster( oldobjectIdx );
			if( mon )
				mon->UpdateTutorialBoard();
		}
		break;
	case eOBJECTTYPE_NPC:
		{
			cNpc* npc = OBJECTMAN->GetNpc( oldobjectIdx );
			if( npc )
				npc->UpdateTutorialBoard();
		}
		break;
	}

	// 새로 세팅
	switch( type )
	{
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* mon = OBJECTMAN->GetMonster( objectIdx );
			if( mon )
				mon->UpdateTutorialBoard();
		}
		break;
	case eOBJECTTYPE_NPC:
		{
			cNpc* npc = OBJECTMAN->GetNpc( objectIdx );
			if( npc )
				npc->UpdateTutorialBoard();
		}
		break;
	}
}

//
sTutorialBaseImage* cTutorialManager::GetObjectImage( char type, unsigned long classIndex, unsigned long objectindex )
{
	if( mModeIndex == -1 )
		return 0;

	cTutorialMode* mode = GetTutorialMode( mModeIndex );
	if( !mode )
		return 0;

	unsigned int size = mode->mObjectArr.GetSize();
	if( size == 0 )
		return 0;

	unsigned long imageIndex = ULONG_MAX;
	for( unsigned int i = 0; i < size; ++i )
	{
		sTutorialObject* data = (sTutorialObject*)mode->mObjectArr[i];
		if( !data )
		{
			assert(0);
			continue;
		}

		// 전체(1) : 해당 타입 객체 전부 적용
		// 한개(0) : 해당 타입의 해당 고유인덱스를 가진 객체에게만 적용
		// 타겟(2) : 해당 타입의 해당 고유인덱스를 가진 객체가 타겟팅 되었을때만 처리

		// 한 종류
		if( data->mSetAll == 0 )
		{
			// 동일 객체일때만 처리
			if( data->mObjectType == type && data->mClassIndex == classIndex )
				imageIndex = data->mImageIndex;
		}
		// 전체 객체
		else if( data->mSetAll == 1 )
		{
			// 동일 타입 처리
			if( data->mObjectType == type )
				imageIndex = data->mImageIndex;
		}
		// 타겟팅
		else if( data->mSetAll == 2 )
		{
			// 타겟(2) : 해당 타입의 해당 고유인덱스를 가진 객체가 타겟팅 되었을때만 처리
			if( data->mObjectType == type && data->mClassIndex == classIndex )
			{
				if( HERO && HERO->GetTargetInfo() )
				{
					const sObject* obj = HERO->GetTargetInfo();
					if( obj && obj->index == objectindex )
						imageIndex = data->mImageIndex;
				}
			}
		}
	}

	if( imageIndex != ULONG_MAX )
	{
		return (sTutorialBaseImage*)mode->mObjectImageMap.GetAt( imageIndex );
	}

	return 0;
}
