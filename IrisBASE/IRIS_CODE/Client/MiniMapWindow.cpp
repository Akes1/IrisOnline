#include "stdafx.h"
#include "MiniMapWindow.h"
#include "PushButton.h"
#include "Label.h"
#include "CheckBox.h"
#include "ResourceManager.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "NaviMesh.h"
#include "WorldManager.h"
#include "CameraManager.h"
#include "StageScript.h"
#include "FontAgent.h"
#include "Icon.h"
#include "NaviField.h"
#include "UICursor.h"
#include "PVPManager.h"
#include "Npc_Common.h"
#include "NpcScript.h"
#include "QuestManager.h"
#include "PlaneObject.h"
#include "Application.h"

cMiniMapWindow::cMiniMapWindow()
: mpExplain(0)
, mpPush(0)
, mpQuestViewText(0)
, mpQuestView(0)
, mpChaseSkin(0)
{
	mIsWidMap = true;

	mIsSetCursorDefault = false;
}


cMiniMapWindow::~cMiniMapWindow()
{
	Clear();

	if( mpPositionData )
	{
		SAFE_DELETE( mpPositionData->mAniParam );
		SAFE_DELETE( mpPositionData->mpImage );
		SAFE_DELETE( mpPositionData );
	}

	SAFE_DELETE( mpMapImage );
	SAFE_DELETE( mpHeroImage );
	SAFE_DELETE( mpCameraImage );
}

void cMiniMapWindow::Clear()
{
	cSmallMiniMapWindow::Clear();

	//
	ClearChaseImage();
}

void cMiniMapWindow::Open()
{
	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y ) );

	/// 현재 hero의 맵 번호 알아오기
	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( HERO->GetMapNum(), HERO->GetMapMode() );

	/// 미니맵 로딩
	cString pathName;
	pathName.Format( "./language/%s/MapImage_%02d.tga", cApplication::mLangaugeFolder.Cstr(), folderIdx );
	
	NiTexture* ptex = RESOURCEMAN->LoadTexture( pathName, false );
	if( ptex == 0 )
	{
		assert( 0 && "failed to load texture" );
		return;
	}

	if( mpMapImage )
	{
		assert(0);
		SAFE_DELETE( mpMapImage );
	}

	/// 텍스쳐 설정
	mpMapImage = new cUIImage( ptex );
	mpMapImage->SetTextureRect( 0, 0, MAPTEX_W, MAPTEX_H );
	mpMapImage->SetScreenRect( cUIRect( x, y, x + GetAbsoluteRect().GetWidth(), y + GetAbsoluteRect().GetHeight() ) );
	mpMapImage->SetAlpha( 0.7f );
}

void cMiniMapWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpPush )
	{
		if( mpPush->IsPress() )
		{
			if( mpExplain )
				mpExplain->Show();
		}
		else
		{
			if( mpExplain )
				mpExplain->Hide();
		}
	}
	else
	{
		if( mpExplain )
			mpExplain->Show();
	}

	mPosStr.Clear();
}

///
bool cMiniMapWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpExplain = (cLabel*)GetChild( eUIID_MINIMAP_EXPLAINIMAGE );
	mpPush = (cPushButton*)GetChild( eUIID_MINIMAP_EXPLAINPUSH );

	mpQuestView = (cCheckBox*)GetChild( eUIID_MINIMAP_QUESTVIEW );
	mpQuestViewText = (cLabel*)GetChild( eUIID_MINIMAP_QUESTVIEWTEXT );

	if( mpQuestView )
		mpQuestView->SetPress( true );

	if( mpExplain )
		mpExplain->Hide();

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	// HERO 이미지
	cUINodeSkin* heroSkin = pSkin->GetNodeSkin( "SmallMiniMap_Hero" );
	if( heroSkin && heroSkin->mpTexture )
	{
		mpHeroImage = new cUIImage( heroSkin->mpTexture );

		unsigned int tx = heroSkin->mSkinInfo->mTexX;
		unsigned int ty = heroSkin->mSkinInfo->mTexY;
		unsigned int tw = heroSkin->mSkinInfo->mWidth;
		unsigned int th = heroSkin->mSkinInfo->mHeight;
		unsigned int x = heroSkin->mSkinInfo->mX;
		unsigned int y = heroSkin->mSkinInfo->mY;

		mpHeroImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpHeroImage->SetScreenRect( cUIRect( x, y, x + tw, y + th ) );

		mHeroSize.mWidth = tw;
		mHeroSize.mHeight = th;
	}

	/// CAMERA 이미지 로드
	cUINodeSkin* cameraSkin = pSkin->GetNodeSkin( "SmallMiniMap_Camera" );
	if( cameraSkin && cameraSkin->mpTexture )
	{
		mpCameraImage = new cUIImage( cameraSkin->mpTexture );

		unsigned int tx = cameraSkin->mSkinInfo->mTexX;
		unsigned int ty = cameraSkin->mSkinInfo->mTexY;
		unsigned int tw = cameraSkin->mSkinInfo->mWidth;
		unsigned int th = cameraSkin->mSkinInfo->mHeight;
		unsigned int x = cameraSkin->mSkinInfo->mX;
		unsigned int y = cameraSkin->mSkinInfo->mY;

		mpCameraImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpCameraImage->SetScreenRect( cUIRect( x, y, x + tw, y + th ) );

		mCameraSize.mWidth = tw;
		mCameraSize.mHeight = th;
	}

	// 찾아가기 이미지 로드
	cUINodeSkin* findSkin = pSkin->GetNodeSkin( "SmallMiniMap_Position" );
	if( findSkin && findSkin->mpTexture )
	{
		sAniImageData* p = new sAniImageData;
		p->mShow = false;

		/// 애니메이션세팅
		sAniParam* param = new sAniParam;
		param->mTexX = findSkin->mSkinInfo->mTexX;
		param->mTexY = findSkin->mSkinInfo->mTexY;
		param->mWidth = findSkin->mSkinInfo->mWidth;
		param->mHeight = findSkin->mSkinInfo->mHeight;
		param->mTimeOut = 100;
		param->mLastAccumTime = 0;
		param->mFrame = 7;
		param->mAniOn = false;
		param->mCurFrame = 0;
		param->mReturn = false;
		p->mAniParam = param;

		p->mpImage = new cUIImage( findSkin->mpTexture );
		p->mSize.mWidth = param->mWidth;
		p->mSize.mHeight = param->mHeight;

		unsigned int tx = param->mTexX;
		unsigned int ty = param->mTexY;
		unsigned int w = p->mSize.mWidth;
		unsigned int h = p->mSize.mHeight;

		p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );
		p->mpImage->SetScreenRect( cUIRect( p->mPos.mX, p->mPos.mY, p->mPos.mX + w, p->mPos.mY + h ) );

		mpPositionData = p;
	}

	/// 전장에서 현재 수장 표시
	mPvpPartyLeader = pSkin->GetNodeSkin( "SmallMiniMap_PvpLeader" );
	if( !mPvpPartyLeader )
	{	
		assert(0);
		return false;
	}

	mpQuestNew = pSkin->GetNodeSkin( "SmallMiniMap_QuestNew" );
	if( mpQuestNew == 0 )
		return false;

	mpQuestPlay = pSkin->GetNodeSkin( "SmallMiniMap_QuestPlay" );
	if( mpQuestPlay == 0 )
		return false;

	mpQuestComplete = pSkin->GetNodeSkin( "SmallMiniMap_QuestComplete" );
	if( mpQuestComplete == 0 )
		return false;

	mpNexLevelQuestNew = pSkin->GetNodeSkin( "SmallMiniMap_HQuestComplete" );
	if( mpNexLevelQuestNew == 0 )
		return false;

	mpThemeQuestNew = pSkin->GetNodeSkin( "SmallMiniMap_TQuestNew" );
	if( mpThemeQuestNew == 0 )
		return false;

	mpThemeQuestComplete = pSkin->GetNodeSkin( "SmallMiniMap_TQuestComplete" );
	if( mpThemeQuestComplete == 0 )
		return false;

	mpRepeatQuestNew = pSkin->GetNodeSkin( "SmallMiniMap_RQuestNew" );
	if( mpRepeatQuestNew == 0 )
		return false;

	mpRepeatQuestComplete = pSkin->GetNodeSkin( "SmallMiniMap_RQuestComplete" );
	if( mpRepeatQuestComplete == 0 )
		return false;

	mPartyInSkin = pSkin->GetNodeSkin( "MiniMap_PartyIn" );
	if( mPartyInSkin == 0 )
		return false;

	mpChaseSkin = pSkin->GetNodeSkin( "MiniMap_QuestChase" );
	if( mpChaseSkin == 0 )
		return false;

	/// 전장에서 석상 이미지 표시
	mPvpTeamAStone = pSkin->GetNodeSkin( "SmallMiniMap_TeamAStone" );
	if( mPvpTeamAStone == 0 )
		return false;

	mPvpTeamBStone = pSkin->GetNodeSkin( "SmallMiniMap_TeamBStone" );
	if( mPvpTeamBStone == 0 )
		return false;

	return true;
}

void cMiniMapWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpMapImage )
		mpMapImage->SetScreenRect( mAbsoluteRect );

	mNeedUpdate = true;
}

void cMiniMapWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();
	
	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );

	/// 항상 갱신하지 않는 이미지 데이터들은
	/// 해상도가 변겨오디었을씨만 갱신해준다
	cDataHashMap::cIterator i, end;
	i = mNpcHashMap.Begin();
	end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mpImage && p->mpAniImage )
		{
			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
			p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
			p->mpAniImage->SetPos( p->mScreenPos.mX + PIXNPCANI_X, p->mScreenPos.mY - PIXNPCANI_Y );
		}
	}

	// portal
	i = mPortalHashMap.Begin();
	end = mPortalHashMap.End();
	for( ; i != end; ++i )
	{
		sPortalImageData* p = (sPortalImageData*)(i->mSecond);
		if( p && p->mpImage )
		{
			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
			p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
		}
	}

	if( HERO && HERO->IsActiveFindPositon() == true )
	{
		// find pos
		if( mpPositionData && mpPositionData->mpImage )
		{
			sAniImageData* p = mpPositionData;

			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			/// 텍스쳐상 의서의 위치 재계산
			p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
			p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
		}
	}

	// 추적이미지 갱신
	if( mpChaseSkin )
	{
		unsigned short tw = (unsigned short)mpChaseSkin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)mpChaseSkin->mSkinInfo->mTexHeight;

		for( unsigned int i = 0; i < mChaseImageArr.GetSize(); ++i )
		{
			sChaseData* data = (sChaseData*)mChaseImageArr[i];
			if( data && data->pImage )
			{
				short w = (short)(tw * data->scale);
				short h = (short)(th * data->scale);
				short x = (short)(GetAbsoluteRect().mLeft + data->x - (w * 0.5));
				short y = (short)(GetAbsoluteRect().mBottom - data->y - (h * 0.5));

				int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, data->title.Cstr(), data->title.GetLength() );
				int textHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );

				// 위치 조정
				data->textX = (int)(x + ( w - textWidth ) * 0.5);
				data->textY = (int)(y + ( h - textHeight) * 0.5);
				data->pImage->SetScreenRect( x, y, w, h );
			}
		}
	}
	
	mNeedUpdate = true;
}

void cMiniMapWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	cHero* hero = OBJECTMANAGER->GetHero();
	cCamera* camera = CAMERAMAN->GetCurrent();
	cNaviMesh* navi = WORLDMAN->GetNaviMesh();

	if( !hero || !camera || !navi )
		return;

	if( !mpHeroImage || !mpCameraImage )
		return;

	float segLen = navi->GetSegmentLength();

	float x = hero->GetXPos();
	float y = hero->GetYPos();

	if( mNeedUpdate || x != mOldX || y != mOldY )
	{
		mOldX = x;
		mOldY = y;

		/// 텍스쳐 상의 캐릭터 위치 GetSegmentLength : 맵의 변 길이 
		int heroX = (int)(x / segLen * MAPTEX_W);
		int heroY = (int)(MAPTEX_H - (y / segLen * MAPTEX_H ));

		// hero 
		short heroScreenX = (short)heroX - (short)(mHeroSize.mWidth * 0.5f );
		short heroScreenY = (short)heroY - (short)(mHeroSize.mHeight * 0.5f );
		mpHeroImage->SetTranslate( (short)GetAbsoluteRect().mLeft + heroScreenX, (short)GetAbsoluteRect().mTop + heroScreenY );

		// camera
		short camScreenX = (short)heroX - (short)(mCameraSize.mWidth * 0.5f );
		short camScreenY = (short)heroY - (short)(mCameraSize.mHeight * 0.5f );
		mpCameraImage->SetTranslate( (short)GetAbsoluteRect().mLeft + camScreenX, (short)GetAbsoluteRect().mTop + camScreenY );

		// 이동하는것만 갱신
		cDataHashMap::cIterator i, end;

		// party
		if( HERO->GetPartyIndex() > 0 )
		{
			unsigned short heroMapNum = HERO->GetMapNum();
			unsigned short heroRoomIndex = HERO->GetPartyRoomIndex();
			DWORD		   heroChannelNum = HERO->GetChannelNum();

			i = mPartyHashMap.Begin();
			end = mPartyHashMap.End();
			for( ; i != end; ++i )
			{
				sUserImageData* p = (sUserImageData*)(i->mSecond);
				if( p && p->mpImage )
				{
					// 동일 맵 여부
					bool isSameMap = false;	

					// 일반 맵인 경우 
					if( p->mMapNum >= MAP_MIN && p->mMapNum <= MAP_MAX )
					{
						isSameMap = ( heroMapNum == p->mMapNum && heroChannelNum == p->mChannelNum );
					}
					// 그외 맵 인 경우
					else
					{
						isSameMap = ( heroMapNum == p->mMapNum && heroRoomIndex == p->mRoomNum );
					}


					if( isSameMap == false )
						continue;

					int cx = (int)(p->mSize.mWidth * 0.5f);
					int cy = (int)(p->mSize.mHeight * 0.5f);

					p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
					p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;
					p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				}
			}
		}

		// party union
		if( HERO->GetPartyUnionIndex() > 0 )
		{
			i = mPartyUnionHashMap.Begin();
			end = mPartyUnionHashMap.End();
			for( ; i != end; ++i )
			{
				sUserImageData* p = (sUserImageData*)(i->mSecond);
				if( p && p->mpImage )
				{
					int cx = (int)(p->mSize.mWidth * 0.5f);
					int cy = (int)(p->mSize.mHeight * 0.5f);

					p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
					p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;
					p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				}
			}
		}
		
		if( PVPMAN->IsEnableBattle() == true )
		{
			// flag 
			i = mFlagMap.Begin();
			end = mFlagMap.End();
			for( ; i != end; ++i )
			{
				sUserImageData* p = (sUserImageData*)(i->mSecond);
				if( p )
				{
					/// 텍스쳐상 의서의 위치 재계산
					p->mScreenPos.mX = (int)p->mPos.mX + GetAbsoluteRect().mLeft; 
					p->mScreenPos.mY = (int)p->mPos.mY + GetAbsoluteRect().mTop - p->mSize.mHeight;
					p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				}
			}		

			// flag point
			i = mFlagPointMap.Begin();
			end = mFlagPointMap.End();
			for( ; i != end; ++i )
			{
				sAniImageData* p = (sAniImageData*)(i->mSecond);
				if( p )
				{
					int cx = (int)(p->mSize.mWidth * 0.5f);
					int cy = (int)(p->mSize.mHeight * 0.5f);

					/// 텍스쳐상 의서의 위치 재계산
					p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
					p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;
					p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				}
			}

			// stone point
			i = mPvPStoneMap.Begin();
			end = mPvPStoneMap.End();
			for( ; i != end; ++i )
			{
				sBaseImageData* p = (sBaseImageData*)(i->mSecond);
				if( p )
				{
					int cx = (int)(p->mSize.mWidth * 0.5f);
					int cy = (int)(p->mSize.mHeight * 0.5f);

					/// 텍스쳐상 의서의 위치 재계산
					p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
					p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;
					p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				}
			}
		}

		mNeedUpdate = false;
	}

	/// hero 회전
	NiPoint3 heroDir = hero->GetDirection();
	heroDir.z = 0.f;
	heroDir.Unitize();

	if( heroDir != mHeroDir )
	{
		mHeroDir = heroDir;

		float dot = heroDir.Dot( NiPoint3::UNIT_Y );
		float ang = NiACos( dot );

		if( heroDir.x > 0.f )
			ang = -ang;

		mpHeroImage->SetRotate( ang );
	}	

	/// 카메라 회전
	NiPoint3  camDir = camera->GetWorldDirection();
	camDir.z = 0.f;
	camDir.Unitize();

	if( camDir != mCameraDir )
	{
		mCameraDir = camDir;

		float dot = camDir.Dot( NiPoint3::UNIT_Y );
		float ang = NiACos( dot );

		if( camDir.x > 0.f )
			ang = -ang;

		mpCameraImage->SetRotate( ang );
	}

	/// 찾아가기 목표 지점
	if( mpPositionData && mpPositionData->mShow == true )
	{
		UpdateAni( accumTime, mpPositionData->mAniParam, mpPositionData->mpImage );
	}

	/// 게양 포인트
	cDataHashMap::cIterator i, end;

	i = mFlagPointMap.Begin();
	end = mFlagPointMap.End();
	for( ; i != end; ++i )
	{
		sAniImageData* p = (sAniImageData*)(i->mSecond);
		if( p && p->mShow == true )
		{
			UpdateAni( accumTime, p->mAniParam, p->mpImage );
		}
	}

	// 애니메이션 갱신
	i = mNpcHashMap.Begin();
	end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mQuestEffect != eNPCQUEST_EFFECT_NONE && p->mQuestEffect != eNPCQUEST_EFFECT_LOW_LEVEL_NEW )
		{
			UpdateAni( accumTime, p->mAniParam, p->mpAniImage );
		}
	}
}

void cMiniMapWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

 	if( mpMapImage )
		mpMapImage->Draw();

	// 추적 이미지 렌더링
	if( mChaseImageArr.GetSize() > 0 )
	{
		for( unsigned int i = 0; i < mChaseImageArr.GetSize(); ++i )
		{
			sChaseData* p = (sChaseData*)mChaseImageArr[i];
			if( p )
			{
				if( p->pImage )
					p->pImage->Draw();
				
				if( p->title.IsEmpty() == false )
					pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(p->title.Cstr()), p->textX, p->textY, COLOR_WHITE, true );
			}
		}
	}

	cDataHashMap::cIterator i, end;
	
	// npc
	i = mNpcHashMap.Begin();
	end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
		{
			p->mpImage->Draw();
		}
	}

	i = mNpcHashMap.Begin();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mShow && p->mpAniImage )
		{
			if( p->mQuestEffect != eNPCQUEST_EFFECT_NONE && p->mQuestEffect != eNPCQUEST_EFFECT_LOW_LEVEL_NEW )
				p->mpAniImage->Draw();
		}
	}

	// party
	i = mPartyHashMap.Begin();
	end = mPartyHashMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
	}

	// portal
	i = mPortalHashMap.Begin();
	end = mPortalHashMap.End();
	for( ; i != end; ++i )
	{
		sPortalImageData* p = (sPortalImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
	}

	// party union
	i = mPartyUnionHashMap.Begin();
	end = mPartyUnionHashMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
	}

	if( mpPositionData && mpPositionData->mpImage && mpPositionData->mShow == true )
		mpPositionData->mpImage->Draw();

	// 카메라
	if( mpCameraImage )
		mpCameraImage->Draw();

	// hero
	if( mpHeroImage )
		mpHeroImage->Draw();

	// flag point
	i = mFlagPointMap.Begin();
	end = mFlagPointMap.End();
	for( ; i != end; ++i )
	{
		sAniImageData* p = (sAniImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
		{
			p->mpImage->Draw();
		}
	}

	// flag
	i = mFlagMap.Begin();
	end = mFlagMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
	}

	// stone
	i = mPvPStoneMap.Begin();
	end = mPvPStoneMap.End();
	for( ; i != end; ++i )
	{
		sBaseImageData* p = (sBaseImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
	}

	// 자식 렌더링
	cChildList::cIterator ci = mChildList.Begin();
	cChildList::cIterator cend = mChildList.End();
	for( ; ci != cend; ++ci )
	{
		if( ((cUINode*)(*ci))->IsVisible() == true )
			((cUINode*)(*ci))->OnRender(pKeeper);
	}

	/// 이름 라벨 출력 
	if( mJobName )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mJobName.Cstr()), mJobPos.mX, mJobPos.mY, COLOR_WHITE, true );

	if( mName )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mName.Cstr()), mNamePos.mX, mNamePos.mY, COLOR_WHITE, true );

	if( !mPosStr.IsEmpty() )
	{
		int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mPosStr.Cstr(), mPosStr.GetLength() );	
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mPosStr.Cstr()), GetAbsoluteRect().mRight - width - 30, GetAbsoluteRect().mTop + 50, COLOR_WHITE );
	}

	pKeeper->DrawAll();
}

void cMiniMapWindow::OnPushButtonPressed( cUINode* , unsigned int id )
{
	/// 설명버튼 누르면
	if( mpPush && mpExplain && id == eUIID_MINIMAP_EXPLAINPUSH )
	{
		if( mpPush->IsPress() == true )
			mpExplain->Show();
		else
			mpExplain->Hide();
	}
}

void cMiniMapWindow::OnMouseMove( const cUIPos& pos )
{
	cUIWindow::OnMouseMove( pos );

	cUIRect rect;
	cDataHashMap::cIterator i, end;

	bool hoverd = false;

	// party
	if( HERO->GetPartyIndex() > 0 )
	{
		i = mPartyHashMap.Begin();
		end = mPartyHashMap.End();
		for( ; i != end; ++i )
		{
			sUserImageData* p = (sUserImageData*)(i->mSecond);
			if( p && p->mpImage && p->mShow )
			{
				rect.mLeft = p->mScreenPos.mX;
				rect.mTop = p->mScreenPos.mY;
				rect.mRight = rect.mLeft + p->mSize.mWidth;
				rect.mBottom = rect.mTop + p->mSize.mHeight;

				if( rect.ContainPoint( pos ) == true )
				{
					hoverd = true;

					if( mpHoverData != p )
					{
						mpHoverData = p;

						mName = p->mName;
						mJobName.Clear();
						UpdateNamePos();
					}
					break;
				}
			}
		}
	}

	// partyUnion
	if( HERO->GetPartyUnionIndex() > 0 )
	{
		i = mPartyUnionHashMap.Begin();
		end = mPartyUnionHashMap.End();
		for( ; i != end; ++i )
		{
			sUserImageData* p = (sUserImageData*)(i->mSecond);
			if( p && p->mpImage && p->mShow )
			{
				rect.mLeft = p->mScreenPos.mX;
				rect.mTop = p->mScreenPos.mY;
				rect.mRight = rect.mLeft + p->mSize.mWidth;
				rect.mBottom = rect.mTop + p->mSize.mHeight;

				if( rect.ContainPoint( pos ) == true )
				{
					hoverd = true;

					if( mpHoverData != p )
					{
						mpHoverData = p;

						mName = p->mName;
						mJobName.Clear();
						UpdateNamePos();
					}
					break;
				}
			}
		}
	}

	// npc
	i =	mNpcHashMap.Begin();
	end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mpImage && p->mShow )
		{
			rect.mLeft = p->mScreenPos.mX;
			rect.mTop = p->mScreenPos.mY;
			rect.mRight = rect.mLeft + p->mSize.mWidth;
			rect.mBottom = rect.mTop + p->mSize.mHeight;

			if( rect.ContainPoint( pos ) == true )
			{
				hoverd = true;

				if( mpHoverData != p )
				{
					mpHoverData = p;

					TCHAR temp[128] = {0,};
					::_stprintf_s( temp, _T("<%s>"), (LPCTSTR)p->mJobName.Cstr() );

					mName = p->mName;
					mJobName = temp;
					UpdateNamePos();

					// npc 마우스 오버시
					if( mpQuestView && mpQuestView->IsPress() == true )
					{
						QUESTMAN->SetQuestTip( p->mClassIndex );
					}
				}
				break;
			}
		}
	}

	// portal
	i = mPortalHashMap.Begin();
	end = mPortalHashMap.End();
	for( ; i != end; ++i )
	{
		sPortalImageData* p = (sPortalImageData*)(i->mSecond);
		if( p && p->mpImage )
		{
			rect.mLeft = p->mScreenPos.mX;
			rect.mTop = p->mScreenPos.mY;
			rect.mRight = rect.mLeft + p->mSize.mWidth;
			rect.mBottom = rect.mTop + p->mSize.mHeight;

			if( rect.ContainPoint( pos ) == true )
			{
				hoverd = true;

				if( mpHoverData != p )
				{
					mpHoverData = p;

					mName = p->mName;
					mJobName.Clear();
					UpdateNamePos();
				}
				break;
			}
		}
	}

	if( hoverd == false )
	{
		mpHoverData = 0;

		mName.Clear();
		mJobName.Clear();

		UIMAN->HideTip();
	}

	//
	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	cNaviField* field = WORLDMAN->GetNaviField();
	if( !navi || !field )
		return;

	/// 이동 가능영역 체크
	int mouseX = pos.mX - GetAbsoluteRect().mLeft;
	int mouseY = pos.mY - GetAbsoluteRect().mTop;
	
	float segLen = navi->GetSegmentLength();

	unsigned int x = (unsigned int)((mouseX * segLen) / MAPTEX_W * 0.01f );
	unsigned int y = (unsigned int)((MAPTEX_H - mouseY) * (segLen / MAPTEX_H) * 0.01f );

	mPosStr.Format( _T("%d, %d"), (int)x, (int)y );

	/// navifield check
	if( field->GetValue( x, y ) != 0 )
		CURSOR->SetCursor( eCURSOR_MOVESTOP );
	else
		CURSOR->SetCursor( eCURSOR_DEFAULT );

}

void cMiniMapWindow::OnMouseLeft(const cUIPos& pos )
{
	cUIWindow::OnMouseLeft( pos );

	mPosStr.Clear();
}

void cMiniMapWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool )
{
	unsigned long state = HERO->GetState();

	switch( state )
	{
	case eOBJECT_STATE_IDLE:
	case eOBJECT_STATE_MOVE:
	case ePLAYER_STATE_GATHERING:
	case ePLAYER_STATE_ITEMPICK:
	case ePLAYER_STATE_SITDOWN:
		break;
	case eOBJECT_STATE_STOP:
		{
			unsigned int stopflag = HERO->GetStopFlag();
			if( !(stopflag == eSTOP_ENHANCED || stopflag == eSTOP_ITEMMIX || stopflag == eSTOP_NPCSPEECH) )
				return;
		}
		break;
	default:
		return;
	}

	if( HERO->IsKeyMoving() == true )
		return;

	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	cNaviField* field = WORLDMAN->GetNaviField();
	if( !navi || !field )
		return;

	/// 이동 가능영역 체크
	int mouseX = pos.mX - GetAbsoluteRect().mLeft;
	int mouseY = pos.mY - GetAbsoluteRect().mTop;

	float segLen = navi->GetSegmentLength();

	float x = (float)((mouseX * segLen) / MAPTEX_W );
	float y = (float)((MAPTEX_H - mouseY) * (segLen / MAPTEX_H) );

	if( x < 0 || y < 0 )
	{
		assert(0);
		return;
	}

	/// 이동..
	HERO->ActivePosition( x, y );
}

void cMiniMapWindow::SetMapData()
{
	tPointerArray<void*>* pMapArray = STAGESCRIPT->GetMapChangeStarterArr( HERO->GetMapNum() );

	/// 맵포탈 정보 세팅
	if( pMapArray->IsEmpty() == false )
	{
		for( unsigned int i = 0; i < pMapArray->GetSize(); ++i )
		{
			sStageChangePos* p = (sStageChangePos*)(*pMapArray)[i];
			AddPortal( p->mPosIdx );
		}
	}
}

void cMiniMapWindow::SetPressExplain( bool press )
{
	if( mpPush )
		mpPush->SetPress( press );
}

bool cMiniMapWindow::IsPressExplain()
{
	return ( mpPush ) ? mpPush->IsPress() : false;
}

void cMiniMapWindow::SetPressQuestView( bool press )
{
	if( mpQuestView )
		mpQuestView->SetPress( press );
}

bool cMiniMapWindow::IsPressQuestView()
{
	return ( mpQuestView ) ? mpQuestView->IsPress() : false;
}

void cMiniMapWindow::AddNpc( sNpcData* data )
{
	if( data == 0 )
	{
		assert(0);
		return;
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return;

	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	float segLen = navi->GetSegmentLength();

	sNPCList* pNpcList = NPCSCRIPT->GetNPCList( data->mNpcClassIdx );
	if( !pNpcList )
	{
		assert(0);
		return;
	}

	sNpcImageData* npcData = new sNpcImageData;
	npcData->mClassIndex	= data->mNpcClassIdx;
	npcData->mQuestEffect	= eNPCQUEST_EFFECT_NONE;
	npcData->mPos.mX		= (int)( data->mPosX / segLen * MAPTEX_W );
	npcData->mPos.mY		= (int)( MAPTEX_H - ( data->mPosY / segLen * MAPTEX_H ) );
	npcData->mName			= pNpcList->mName;
	npcData->mJobName		= pNpcList->mJobName;

	unsigned long iconId = ( mIsWidMap == false ) ? pNpcList->mJobIconIndexSmall : pNpcList->mJobIconIndexLarge;
	cIconParam* param = UIMAN->GetIconParam( iconId );
	if( param && param->mpTexture )
	{
		/// 이미지세팅
		cUIImage* image = new cUIImage( param->mpTexture );
		unsigned short tx = (unsigned short)param->mTexPos.mX;
		unsigned short ty = (unsigned short)param->mTexPos.mY;
		npcData->mSize.mWidth = NPCICON_SIZE;
		npcData->mSize.mHeight = NPCICON_SIZE;

		int cx = (int)(NPCICON_SIZE * 0.5f);
		int cy = (int)(NPCICON_SIZE * 0.5f);
		npcData->mScreenPos.mX = (int)npcData->mPos.mX - cx + GetAbsoluteRect().mLeft; 
		npcData->mScreenPos.mY = (int)npcData->mPos.mY - cy + GetAbsoluteRect().mTop;

		image->SetTextureRect( tx, ty, tx + NPCICON_SIZE, ty + NPCICON_SIZE );
		image->SetScreenRect( cUIRect( npcData->mScreenPos.mX, npcData->mScreenPos.mY, 
									   npcData->mScreenPos.mX + NPCICON_SIZE, npcData->mScreenPos.mY + NPCICON_SIZE ) );

		npcData->mpImage = image;
	}

	/// 애니메이션세팅
	npcData->mAniParam = new sAniParam;
	npcData->mAniParam->mTimeOut = 100;
	npcData->mAniParam->mLastAccumTime = 0;
	npcData->mAniParam->mFrame = 5;
	npcData->mAniParam->mTexX = 0;
	npcData->mAniParam->mTexY = 0;
	npcData->mAniParam->mAniOn = false;
	npcData->mAniParam->mCurFrame = 0;
	npcData->mAniParam->mReturn = false;

	/// 애니용 이미지 생성
	if( mpQuestNew && mpQuestNew->mpTexture )
	{
		cUIImage* image = new cUIImage( mpQuestNew->mpTexture );

		unsigned int tx = mpQuestNew->mSkinInfo->mTexX;
		unsigned int ty = mpQuestNew->mSkinInfo->mTexY;
		unsigned int tw = npcData->mAniParam->mWidth = mpQuestNew->mSkinInfo->mWidth;
		unsigned int th = npcData->mAniParam->mHeight = mpQuestNew->mSkinInfo->mHeight;

		image->SetTextureRect( tx, ty, tx + tw, ty + th );
		image->SetScreenRect( cUIRect( npcData->mScreenPos.mX + PIXNPCANI_X, npcData->mScreenPos.mY - PIXNPCANI_Y, 
									   npcData->mScreenPos.mX + tw + PIXNPCANI_X, npcData->mScreenPos.mY + th - PIXNPCANI_Y ) );
		
		npcData->mAniParam->mWidth = tw;
		npcData->mAniParam->mHeight = th;

		npcData->mpAniImage = image;
	}

	if( mNpcHashMap.Insert( data->mNpcIdx, npcData ) == false )
	{
		assert(0);
	}
}


void cMiniMapWindow::AddPortal( unsigned long index )
{
	sStageChangePos* data = STAGESCRIPT->GetPosScriptInfo( index );
	if( !data )
	{
		assert(0);
		return;
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return;

	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	float segLen = navi->GetSegmentLength();

	cUINodeSkin* potalSkin = pSkin->GetNodeSkin( "SmallMiniMap_Portal" );
	if( potalSkin && potalSkin->mpTexture )
	{
		sPortalImageData* p = new sPortalImageData;
		p->mTargetMapNum = ( data->mTargetInfo != NULL ) ? data->mTargetInfo->mMapNumber : data->mMapNumber; 
		p->mName = GAMERESOURCEMAN->GetMapName( p->mTargetMapNum );

		p->mpImage = new cUIImage( potalSkin->mpTexture );
		unsigned short tx = (unsigned short)potalSkin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)potalSkin->mSkinInfo->mTexY;
		unsigned short w = (unsigned short)potalSkin->mSkinInfo->mTexWidth;
		unsigned short h = (unsigned short)potalSkin->mSkinInfo->mTexHeight;
		p->mPos.mX = (int)( data->mPosX / segLen * MAPTEX_W );
		p->mPos.mY = (int)( MAPTEX_H - ( data->mPosY / segLen * MAPTEX_H ) );

		p->mSize.mWidth = w;
		p->mSize.mHeight = h;

		int cx = (int)(p->mSize.mWidth * 0.5f);
		int cy = (int)(p->mSize.mHeight * 0.5f);
		p->mScreenPos.mX = (int)p->mPos.mX - cx + GetAbsoluteRect().mLeft; 
		p->mScreenPos.mY = (int)p->mPos.mY - cy + GetAbsoluteRect().mTop;

		p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );
		p->mpImage->SetScreenRect( cUIRect( p->mScreenPos.mX, p->mScreenPos.mY, p->mScreenPos.mX + w, p->mScreenPos.mY + h ) );

		if( mPortalHashMap.Insert( index, p ) == false )
		{
			assert(0);
		}
	}
}

void cMiniMapWindow::AddFindPosition( float xpos, float ypos )
{
	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	float segLen = navi->GetSegmentLength();

	if( mpPositionData )
	{
		mpPositionData->mShow = true;
		mpPositionData->mEnableShow = true;

		mpPositionData->mPos.mX = (int)( xpos / segLen * MAPTEX_W );
		mpPositionData->mPos.mY = (int)( MAPTEX_H - ( ypos / segLen * MAPTEX_H ) );

		int w = mpPositionData->mSize.mWidth;
		int h = mpPositionData->mSize.mHeight;

		int cx = (int)(w * 0.5f);
		int cy = (int)(h * 0.5f);
		mpPositionData->mScreenPos.mX = (int)mpPositionData->mPos.mX - cx + GetAbsoluteRect().mLeft; 
		mpPositionData->mScreenPos.mY = (int)mpPositionData->mPos.mY - cy + GetAbsoluteRect().mTop;
		mpPositionData->mpImage->SetScreenRect( cUIRect( mpPositionData->mScreenPos.mX, mpPositionData->mScreenPos.mY, 
														 mpPositionData->mScreenPos.mX + w, mpPositionData->mScreenPos.mY + h ) );
	}
}

/// 이름 정보 위치 업데이트
void cMiniMapWindow::UpdateNamePos()
{
	if( mName.IsEmpty() )
		return;

	if( !(mpHoverData && mpHoverData->mpImage && mpHoverData->mShow) )
		return;

	cUIRect rc;
	rc.mLeft = mpHoverData->mScreenPos.mX;
	rc.mTop = mpHoverData->mScreenPos.mY;
	rc.mRight = rc.mLeft + mpHoverData->mSize.mWidth;
	rc.mBottom = rc.mTop + mpHoverData->mSize.mHeight;

	/// 맵 이미지 좌표 비교후 보정
	int mapLeft = GetAbsoluteRect().mLeft;
	int mapTop = GetAbsoluteRect().mTop;

	int textHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_System );

	/// 직업이름이 있으면 좌표 세팅
	if( mJobName.IsEmpty() == false )
	{
		/// 직업좌표 세팅
		int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, mJobName.Cstr(), mJobName.GetLength() );

		mJobPos.mX = (int)(rc.mLeft + ( rc.GetWidth() - textWidth ) * 0.5);
		mJobPos.mY = rc.mTop - 10 - textHeight;

		/// 이름이 튀어나가지 않도록 보정
		if( mJobPos.mX <= mapLeft )
			mJobPos.mX = mapLeft;
		else if( mJobPos.mX + textWidth >= mapLeft + GetAbsoluteRect().GetWidth() )
			mJobPos.mX = mapLeft + GetAbsoluteRect().GetWidth() - textWidth;

		/// 너무 위에 찍히는것을 방지
		if( mJobPos.mY <= mapTop )
			mJobPos.mY = rc.mBottom + 2;
	}

	/// 이름좌표 세팅
	int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, mName.Cstr(), mName.GetLength() );

	mNamePos.mX = (int)(rc.mLeft + ( rc.GetWidth() - textWidth ) * 0.5);
	mNamePos.mY = rc.mTop - 10;

	/// 이름이 튀어나가지 않도록 보정
	if( mNamePos.mX <= mapLeft )
		mNamePos.mX = mapLeft;
	else if( mNamePos.mX + textWidth >= mapLeft + GetAbsoluteRect().GetWidth() )
		mNamePos.mX = mapLeft + GetAbsoluteRect().GetWidth() - textWidth;

	/// 너무 위에 찍히는 것을 방지
	if( mJobName.IsEmpty() == false )
		mNamePos.mY = mJobPos.mY + 2 + textHeight;
}

// 퀘스트 추적이미지 세팅
void cMiniMapWindow::UpdateQuestChase()
{
	// 추적 이미지 삭제
	ClearChaseImage();

	//
	long questIndex = QUESTMAN->GetChangeQuestIndex();
	cQuestDefine* define = QUESTMAN->GetQuestDefine( questIndex );
	if( !define )
		return;
	
	if( !mpChaseSkin )
	{
		assert(0);
		return;
	}

	cQuestInfo* info = define->mQuestInfo;
	if( !info )
		return;

	unsigned short mapnum = HERO->GetMapNum();
	unsigned int tx = (unsigned short)mpChaseSkin->mSkinInfo->mTexX;
	unsigned int ty = (unsigned short)mpChaseSkin->mSkinInfo->mTexY;
	unsigned int tw = (unsigned short)mpChaseSkin->mSkinInfo->mTexWidth;
	unsigned int th = (unsigned short)mpChaseSkin->mSkinInfo->mTexHeight;

	sQuestInfoData* data = info->mData;
	for( int i = 0; i < MAX_DUTY; ++i, ++data ) 
	{
		if( data->mapIndex == 0 )
			continue;
		
		// hero 의 맵 번호랑 맞아야 출력
		if( data->mapIndex != mapnum )
			continue;

		// 
		cPlaneObject* chaseImg = new cPlaneObject;
		if( chaseImg->Create( mpChaseSkin->mpTexture, 0, 0, tw, th, tx, ty, tx+tw, ty+th ) == false )
		{
			SAFE_DELETE( chaseImg );
			assert( 0 && "failed to create chase image");
			continue;
		}

		sChaseData* p = new sChaseData;
		p->pImage = chaseImg;
		p->x = data->x;
		p->y = data->y;
		p->scale = data->scale;

		if( data->showName == 1 )
			p->title = GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex );

		short w = (short)(tw * data->scale);
		short h = (short)(th * data->scale);
		short x = (short)(GetAbsoluteRect().mLeft + data->x - (w * 0.5));
		short y = (short)(GetAbsoluteRect().mBottom - data->y - (h * 0.5));
		
		int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, p->title.Cstr(), p->title.GetLength() );
		int textHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );

		p->textX = (int)(x + ( w - textWidth ) * 0.5);
		p->textY = (int)(y + ( h - textHeight) * 0.5);

		// 위치 조정
		chaseImg->SetScreenRect( x, y, w, h );

		mChaseImageArr.PushBack( p );
	}
}

void cMiniMapWindow::ClearChaseImage()
{
	for( unsigned int i = 0; i < mChaseImageArr.GetSize(); ++i )
	{
		sChaseData* p = (sChaseData*)mChaseImageArr[i];
		if( p )
			SAFE_DELETE( p->pImage );

		SAFE_DELETE( p );
	}
	mChaseImageArr.Clear();
}
