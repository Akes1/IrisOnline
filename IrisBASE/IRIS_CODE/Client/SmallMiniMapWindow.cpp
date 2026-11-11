#include "stdafx.h"
#include "SmallMiniMapWindow.h"
#include "UIImage.h"
#include "ResourceManager.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameUIManager.h"
#include "WorldManager.h"
#include "NaviMesh.h"
#include "CameraManager.h"
#include "FontAgent.h"
#include "NpcScript.h"
#include "Icon.h"
#include "StageScript.h"
#include "Label.h"
#include "PartyUnionManager.h"
#include "PVPManager.h"
#include "Application.h"

cSmallMiniMapWindow::cSmallMiniMapWindow()
: mpMapImage(0)
, mpPosBackImage(0)
, mpHeroImage(0)
, mpCameraImage(0)
, mNeedUpdate( false )
, mPerTexW(0)
, mPerTexH(0)
, mPercent( MAX_PERCENT )
, mPos(0, 0)
, mOldX(-1.f)
, mOldY(-1.f)
, mHeroSize(0, 0)
, mCameraSize(0, 0)
, mpQuestNew(0)
, mpQuestPlay(0)
, mpQuestComplete(0)
, mpNexLevelQuestNew(0)
, mpThemeQuestNew(0)
, mpThemeQuestComplete(0)
, mpRepeatQuestNew(0)
, mpRepeatQuestComplete(0)
, mPartyInSkin(0)
, mPartyOutSkin(0)
, mPvpPartyLeader(0)
, mPvpTeamAStone(0)
, mPvpTeamBStone(0)
, mpMapName(0)
, mpChannelNum(0)
, mIsWidMap( false )
, mpPositionData(0)
, mNamePos(0, 0)
, mJobPos(0, 0)
, mpHoverData(0)
, mOldPvpLeaderIndex(0)
{
	mHeroDir = -NiPoint3::UNIT_Y;
	mCameraDir = -NiPoint3::UNIT_Y;
}

cSmallMiniMapWindow::~cSmallMiniMapWindow()
{
	Clear();

	if( mpPositionData )
	{
		SAFE_DELETE( mpPositionData->mAniParam );
		SAFE_DELETE( mpPositionData->mpImage );
		SAFE_DELETE( mpPositionData );
	}

	SAFE_DELETE( mpMapImage );
	SAFE_DELETE( mpPosBackImage );
	SAFE_DELETE( mpHeroImage );
	SAFE_DELETE( mpCameraImage );
}

void cSmallMiniMapWindow::Clear()
{
	mpHoverData = 0;

	// npc
	cDataHashMap::cIterator i, end;

	i =	mNpcHashMap.Begin();
	end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mAniParam );
			SAFE_DELETE( p->mpAniImage );
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mNpcHashMap.Clear();

	// party
	i = mPartyHashMap.Begin();
	end = mPartyHashMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mPartyHashMap.Clear();

	// portal
	i = mPortalHashMap.Begin();
	end = mPortalHashMap.End();
	for( ; i != end; ++i )
	{
		sPortalImageData* p = (sPortalImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mPortalHashMap.Clear();

	// partyUnion
	i = mPartyUnionHashMap.Begin();
	end = mPartyUnionHashMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mPartyUnionHashMap.Clear();

	SAFE_DELETE( mpMapImage );

	if( mpMapName )
		mpMapName->Clear();

	if( mpChannelNum )
		mpChannelNum->Clear();

	mPercent = MAX_PERCENT;
	mOldX = -1.f;
	mOldY = -1.f;

	mName.Clear();
	mJobName.Clear();
	mNamePos.mX = 0;
	mNamePos.mY = 0;
	mJobPos.mX = 0;
	mJobPos.mY = 0;

	mOldPvpLeaderIndex = 0;

	ClearFlag();
	ClearFlagPoint();
	ClearStone();
}

void cSmallMiniMapWindow::ClearFlag()
{
	// flag
	cDataHashMap::cIterator i = mFlagMap.Begin();
	cDataHashMap::cIterator end = mFlagMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mFlagMap.Clear();
}

void cSmallMiniMapWindow::ClearFlagPoint()
{
	// flag point
	cDataHashMap::cIterator i = mFlagPointMap.Begin();
	cDataHashMap::cIterator end = mFlagPointMap.End();
	for( ; i != end; ++i )
	{
		sAniImageData* p = (sAniImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mAniParam );
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mFlagPointMap.Clear();
}

void cSmallMiniMapWindow::ClearStone()
{
	// stone point
	cDataHashMap::cIterator i = mPvPStoneMap.Begin();
	cDataHashMap::cIterator end = mPvPStoneMap.End();
	for( ; i != end; ++i )
	{
		sBaseImageData* p = (sBaseImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mPvPStoneMap.Clear();
}

void cSmallMiniMapWindow::Open()
{
	cUIWindow::Open();

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

	unsigned short x = (unsigned short)GetAbsoluteRect().mLeft + MAP_X;
	unsigned short y = (unsigned short)GetAbsoluteRect().mTop + MAP_Y;

	mpMapImage->SetTextureRect( 0, 0, MAPTEX_W, MAPTEX_H );
	mpMapImage->SetScreenRect( cUIRect( x, y, x + MAP_W, y + MAP_H ) );

	/// 현재 윈도우에 적용된 텍스쳐 사이즈 구하기 ( 곱하기 100퍼센트 )
	mPerTexW = MAPTEX_W * mPercent * 0.01;
	mPerTexH = MAPTEX_H * mPercent * 0.01;
}

void cSmallMiniMapWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cSmallMiniMapWindow::OnHide()
{
	cUIWindow::OnHide();

	mName.Clear();
	mJobName.Clear();

	mpHoverData = 0;

	UIMAN->HideTip();
}

bool cSmallMiniMapWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMapName = (cLabel*)GetChild( eUIID_SMALL_MINIMAP_MAPNAME );
	mpChannelNum = (cLabel*)GetChild( eUIID_SMALL_MINIMAP_CHANNEL );

	if( mpMapName )
		mpMapName->SetTextColor( COLOR_WHITE );

	if( mpChannelNum )
		mpChannelNum->SetTextColor( COLOR_WHITE );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	// 좌표 배경 이미지
	cUINodeSkin* posSkin = pSkin->GetNodeSkin( "SmallMiniMap_PosImage" );
	if( posSkin && posSkin->mpTexture )
	{
		mpPosBackImage = new cUIImage( posSkin->mpTexture );

		/// 텍스처 영역 설정
		unsigned int tx = posSkin->mSkinInfo->mTexX;
		unsigned int ty = posSkin->mSkinInfo->mTexY;
		unsigned int tw = posSkin->mSkinInfo->mWidth;
		unsigned int th = posSkin->mSkinInfo->mHeight;
		mPos.mX = posSkin->mSkinInfo->mX;
		mPos.mY = posSkin->mSkinInfo->mY;

		mpPosBackImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpPosBackImage->SetScreenRect( cUIRect( mPos.mX, mPos.mY, mPos.mX + tw, mPos.mY + th ) );
	}

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
		int w = p->mSize.mWidth;
		int h = p->mSize.mHeight;

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

	mPartyInSkin = pSkin->GetNodeSkin( "SmallMiniMap_PartyIn" );
	if( mPartyInSkin == 0 )
		return false;

	mPartyOutSkin = pSkin->GetNodeSkin( "SmallMiniMap_PartyOut" );
	if( mPartyOutSkin == 0 )
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

void cSmallMiniMapWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpMapImage )
	{
		int x = GetAbsoluteRect().mLeft + MAP_X;
		int y = GetAbsoluteRect().mTop + MAP_Y;
		mpMapImage->SetPos( x, y );
	}

	if( mpPosBackImage )
	{
		int x = GetAbsoluteRect().mLeft + mPos.mX;
		int y = GetAbsoluteRect().mTop + mPos.mY;
		mpPosBackImage->SetPos( x, y );
	}
	
	mNeedUpdate = true;
}

void cSmallMiniMapWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	mNeedUpdate = true;
}

void cSmallMiniMapWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
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

		/// 윈도우에 적용할 텍스쳐 좌표 계산
		double texX = heroX - mPerTexW * 0.5f;
		double texY = heroY - mPerTexH * 0.5f;
		double texRight = heroX + mPerTexW * 0.5f;
		double texBottom = heroY + mPerTexH * 0.5f;

		/// 소수점 보정 
		short tl = (short)( texX + 0.5f );
		short tt = (short)( texY + 0.5f );
		unsigned short tr = (unsigned short)( texRight + 0.5f );
		unsigned short tb = (unsigned short)( texBottom + 0.5f );

		/// 미니맵의 스크롤 여부에 따른 보정값 계산 
		int fixX = 0;
		int fixY = 0;

		if( tl < 0 )
		{
			fixX = tl;
			tl = 0;
			tr = (short)mPerTexW;
		}
		else if( tr > MAPTEX_W )
		{
			fixX = tr - MAPTEX_W;
			tl = (short)(MAPTEX_W - mPerTexW);
			tr = MAPTEX_W;
		}

		if( tt < 0 )
		{
			fixY = tt;
			tt = 0;
			tb = (short)mPerTexH;
		}
		else if( tb > MAPTEX_H )
		{
			fixY = tb - MAPTEX_H;
			tt = (short)(MAPTEX_H - mPerTexH);
			tb = MAPTEX_H;
		}

		/// 보정값을 텍스쳐에 대한 비율로 변환
		fixX = (int)((float)fixX / mPerTexW * MAP_W);
		fixY = (int)((float)fixY / mPerTexH * MAP_H);

		//
		double xr = MAP_W * 0.5f + fixX + MAP_X;
		double yr = MAP_H * 0.5f + fixY + MAP_Y;

		/// 이미지 좌표 적용 
		mpMapImage->SetTextureRect( tl, tt, tr, tb );

		// hero 
		short heroScreenX = (short)xr - (short)(mHeroSize.mWidth * 0.5f );
		short heroScreenY = (short)yr - (short)(mHeroSize.mHeight * 0.5f );
		mpHeroImage->SetTranslate( (short)GetAbsoluteRect().mLeft + heroScreenX, (short)GetAbsoluteRect().mTop + heroScreenY );

		// camera
		short camScreenX = (short)xr - (short)(mCameraSize.mWidth * 0.5f );
		short camScreenY = (short)yr - (short)(mCameraSize.mHeight * 0.5f );
		mpCameraImage->SetTranslate( (short)GetAbsoluteRect().mLeft + camScreenX, (short)GetAbsoluteRect().mTop + camScreenY );

		// npc
		UpdateNpc( tl, tt, tr, tb );

		// party
		UpdateParty( tl, tt, tr, tb, xr, yr, heroScreenX, heroScreenY );

		// portal
		UpdatePortal( tl, tt, tr, tb );

		// party union
		UpdatePartyUnion( tl, tt, tr, tb );

		// find pos
		UpdateFindPos( tl, tt, tr, tb );

		// flag
		UpdateFlag( tl, tt, tr, tb );

		// flag point
		UpdateFlagPoint( tl, tt, tr, tb );

		// stone
		UpdateStone( tl, tt, tr, tb );

		//
		if( mpHoverData )
			UpdateNamePos();

		//
		mNeedUpdate = false;
	}

	// hero 회전
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

	// 카메라 회전
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

	/// 애니메이션 갱신
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

void cSmallMiniMapWindow::OnRender( cUIFontItemKeeper* pKeeper )
{

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	if( mpMapImage )
		mpMapImage->Draw();

	cDataHashMap::cIterator i, end;

	// npc
	i = mNpcHashMap.Begin();
	end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
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

	// find pos
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

	// 좌표 back
	if( mpPosBackImage )
		mpPosBackImage->Draw();

	//
	int x = (int)(HERO->GetXPos() * 0.01);
	int y = (int)(HERO->GetYPos() * 0.01);

	cStringT str;
	str.Format( _T("%d, %d"), x, y );
	pKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), GetAbsoluteRect().mLeft + 12, GetAbsoluteRect().mTop + 169, COLOR_WHITE, true );

	/// 이름 라벨 출력 
	if( mJobName )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mJobName.Cstr()), mJobPos.mX, mJobPos.mY, COLOR_WHITE, true );

	if( mName )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mName.Cstr()), mNamePos.mX, mNamePos.mY, COLOR_WHITE, true );

	pKeeper->DrawAll();
}

void cSmallMiniMapWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_SMALL_MINIMAP_LARGEOPEN:
		{
			/// 큰 미니맵 열기 버튼 
			GAMEUI->ToggleMiniMapWindow();
		}
		break;
	case eUIID_SMALL_MINIMAP_PLUS:
		{
			/// 미니맵 뷰 확대
			if( mPercent > MIN_PERCENT )
			{
				mPercent -= ADD_PERCENT;

				/// 현재 윈도우에 적용된 텍스쳐 사이즈 구하기 ( 곱하기 100퍼센트 )
				mPerTexW = MAPTEX_W * mPercent * 0.01;
				mPerTexH = MAPTEX_H * mPercent * 0.01;
				mNeedUpdate = true;
			}
		}
		break;
	case eUIID_SMALL_MINIMAP_MINUS:
		{
			/// 미니맵 뷰 축소 
			if( mPercent < MAX_PERCENT )
			{
				mPercent += ADD_PERCENT;

				/// 현재 윈도우에 적용된 텍스쳐 사이즈 구하기 ( 곱하기 100퍼센트 )
				mPerTexW= MAPTEX_W * mPercent * 0.01;
				mPerTexH = MAPTEX_H * mPercent * 0.01;
				mNeedUpdate = true;
			}			
		}
		break;
	}
}

///
void cSmallMiniMapWindow::OnMouseMove( const cUIPos& pos )
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
	}
}

void cSmallMiniMapWindow::OnMouseLeft( const cUIPos& pos )
{
	if( mpHoverData )
	{
		mName.Clear();
		mJobName.Clear();
		mpHoverData = 0;

		UIMAN->HideTip();
	}
}

/// 맵이름, 포탈정보 세팅
void cSmallMiniMapWindow::SetMapData()
{
	/// 채널번호 입력
	if( mpChannelNum )
		mpChannelNum->SetValue( HERO->GetChannelNum() );

	/// 맵이름
	LPCTSTR mapname = GAMERESOURCEMAN->GetMapName( HERO->GetMapNum() );
	if( mpMapName )
		mpMapName->SetText( mapname );

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

void cSmallMiniMapWindow::UpdateAni( unsigned long accumTime, sAniParam* p, cUIImage* image )
{
	if( !p || !image )
		return;

	p->mAccumTime = accumTime;
	unsigned long elasedTime = p->mAccumTime - p->mLastAccumTime;
	if( elasedTime > p->mTimeOut )
	{
		// 텍스쳐 좌표 갱신
		p->mCurFrame++;

		/// 프레임을 넘어가면 초기화
		if( p->mCurFrame >= p->mFrame )
			p->mCurFrame = 0;

		/// 텍스쳐 좌표 세팅
		unsigned int tx = p->mTexX + ( p->mCurFrame * p->mWidth );
		unsigned int ty = p->mTexY;

		image->SetTextureRect( tx, ty, tx + p->mWidth, ty + p->mHeight );

		// 갱신 시간 저장
		p->mLastAccumTime = p->mAccumTime;
	}
}

void cSmallMiniMapWindow::ClearNpc()
{
	cDataHashMap::cIterator i = mNpcHashMap.Begin();
	cDataHashMap::cIterator end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mAniParam );
			SAFE_DELETE( p->mpAniImage );
			SAFE_DELETE( p->mpImage );		
			SAFE_DELETE( p );
		}
	}
	mNpcHashMap.Clear();
}

void cSmallMiniMapWindow::AddNpc( sNpcData* data )
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

		image->SetTextureRect( tx, ty, tx + NPCICON_SIZE, ty + NPCICON_SIZE );
		image->SetScreenRect( cUIRect( npcData->mPos.mX, npcData->mPos.mY, 
									   npcData->mPos.mX + NPCICON_SIZE, npcData->mPos.mY + NPCICON_SIZE ) );

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
		image->SetScreenRect( cUIRect( npcData->mPos.mX + PIXNPCANI_X, npcData->mPos.mY - PIXNPCANI_Y, 
									   npcData->mPos.mX + tw + PIXNPCANI_X, npcData->mPos.mY + th - PIXNPCANI_Y ) );

		npcData->mAniParam->mWidth = tw;
		npcData->mAniParam->mHeight = th;

		npcData->mpAniImage = image;
	}

	if( mNpcHashMap.Insert( data->mNpcIdx, npcData ) == false )
	{
		assert(0);
	}

	mNeedUpdate = true;
}

void cSmallMiniMapWindow::DeleteNpc( unsigned long npcIdx )
{
	sNpcImageData* p = (sNpcImageData*)mNpcHashMap.GetAt( npcIdx );
	if( p )
	{
		SAFE_DELETE( p->mAniParam );
		SAFE_DELETE( p->mpAniImage );
		SAFE_DELETE( p->mpImage );		
		SAFE_DELETE( p );
		mNpcHashMap.Erase( npcIdx );
	}
}

void cSmallMiniMapWindow::UpdateNpcEffect( unsigned long npcIdx, eNpcQuestEffect type )
{
	sNpcImageData* p = (sNpcImageData*)mNpcHashMap.GetAt( npcIdx );
	if( p )
	{
		p->mQuestEffect = type;

		cUINodeSkin* skin = 0;
		switch( type )
		{
		case eNPCQUEST_EFFECT_NEW:			   skin = mpQuestNew;	   break;
		case eNPCQUEST_EFFECT_PLAYING:		   skin = mpQuestPlay;	   break;
		case eNPCQUEST_EFFECT_REWARD:		   skin = mpQuestComplete; break;
		case eNPCQUEST_EFFECT_COMPLETE:		   skin = mpQuestComplete; break;
		case eNPCQUEST_EFFECT_LOW_LEVEL_NEW:   skin = 0; return;
		case eNPCQUEST_EFFECT_NEXT_LEVEL_NEW:  skin = mpNexLevelQuestNew;    break;
		case eNPCQUEST_EFFECT_THEME_NEW:	   skin = mpThemeQuestNew;	     break;		
		case eNPCQUEST_EFFECT_THEME_COMPLETE:  skin = mpThemeQuestComplete;  break;
		case eNPCQUEST_EFFECT_THEME_REWARD:	   skin = mpThemeQuestComplete;  break;		
		case eNPCQUEST_EFFECT_REPEAT_NEW:	   skin = mpRepeatQuestNew;		 break;	
		case eNPCQUEST_EFFECT_REPEAT_COMPLETE: skin = mpRepeatQuestComplete; break;
		case eNPCQUEST_EFFECT_REPEAT_REWARD:   skin = mpRepeatQuestComplete; break;	 
		default: return;
		}

		if( skin && p->mpAniImage )
		{
			unsigned short tx = (unsigned short)skin->mSkinInfo->mTexX;
			unsigned short ty = (unsigned short)skin->mSkinInfo->mTexY;
			unsigned short tw = (unsigned short)skin->mSkinInfo->mTexWidth;
			unsigned short th = (unsigned short)skin->mSkinInfo->mTexHeight;

			p->mAniParam->mTexX = tx;
			p->mAniParam->mTexY = ty;
			p->mAniParam->mWidth = tw;
			p->mAniParam->mHeight = th;
			p->mpAniImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		}
	}
}

void cSmallMiniMapWindow::ClearParty()
{
	cDataHashMap::cIterator i = mPartyHashMap.Begin();
	cDataHashMap::cIterator end = mPartyHashMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p )
		{
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}
	}
	mPartyHashMap.Clear();
}

void cSmallMiniMapWindow::AddParty( sPartyUserData* data )
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

	if( !(mPartyInSkin && mPartyInSkin->mpTexture) )
	{
		assert(0);
		return;
	}

	sUserImageData* p = new sUserImageData;
	p->mMapNum = data->mapNum;
	p->mRoomNum = data->roomNum;
	p->mChannelNum = data->channelNum;
	p->mUserIndex = data->userIndex;
	p->mName = data->name;
	
	unsigned short heroMapNum = HERO->GetMapNum();
	unsigned short heroRoomIndex = HERO->GetPartyRoomIndex();
	DWORD		   heroChannelNum = HERO->GetChannelNum();

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

	p->mShow = isSameMap;

	p->mpImage = new cUIImage( mPartyInSkin->mpTexture );
	unsigned int tx = mPartyInSkin->mSkinInfo->mTexX;
	unsigned int ty = mPartyInSkin->mSkinInfo->mTexY;
	unsigned int w = mPartyInSkin->mSkinInfo->mTexWidth;
	unsigned int h = mPartyInSkin->mSkinInfo->mTexHeight;
	p->mPos.mX = (int)( data->posX / segLen * MAPTEX_W );
	p->mPos.mY = (int)( MAPTEX_H - ( data->posY / segLen * MAPTEX_H ) );

	p->mOldX = p->mPos.mX;
	p->mOldY = p->mPos.mY;
	p->mSize.mWidth = w;
	p->mSize.mHeight = h;

	p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );
	p->mpImage->SetScreenRect( cUIRect( p->mPos.mX, p->mPos.mY, p->mPos.mX + w, p->mPos.mY + h ) );

	if( mPartyHashMap.Insert( data->userIndex, p ) == false )
	{
		assert(0);
	}

	mNeedUpdate = true;
}

void cSmallMiniMapWindow::DeleteParty( unsigned long userIdx )
{
	sUserImageData* p = (sUserImageData*)mPartyHashMap.GetAt( userIdx );
	if( p )
	{
		SAFE_DELETE( p->mpImage );		
		SAFE_DELETE( p );
		mPartyHashMap.Erase( userIdx );
	}
}

void cSmallMiniMapWindow::UpdatePartyPos( unsigned long userIdx, float posX, float posY )
{
	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	float segLen = navi->GetSegmentLength();

	sUserImageData* p = (sUserImageData*)mPartyHashMap.GetAt( userIdx );
	if( p )
	{
		int newX = (int)( posX / segLen * MAPTEX_W );
		int newY = (int)( MAPTEX_H - ( posY / segLen * MAPTEX_H ) );

		if( newX != p->mOldX || newY != p->mOldY )
		{
			p->mPos.mX = newX;
			p->mPos.mY = newY;
			p->mOldX = newX;
			p->mOldY = newY;

			mNeedUpdate = true;
		}
	}
}

void cSmallMiniMapWindow::UpdatePartyMap( unsigned long userIdx, unsigned short mapnum, unsigned short roomnum, DWORD channelnum )
{
	unsigned short heroMapNum = HERO->GetMapNum();
	unsigned short heroRoomIndex = HERO->GetPartyRoomIndex();
	DWORD		   heroChannelNum = HERO->GetChannelNum();

	// 동일 맵 여부
	bool isSameMap = false;	

	// 일반 맵인 경우 
	if( mapnum >= MAP_MIN && mapnum <= MAP_MAX )
	{
		isSameMap = ( heroMapNum == mapnum && heroChannelNum == channelnum );
	}
	// 그외 맵 인 경우
	else
	{
		isSameMap = ( heroMapNum == mapnum && heroRoomIndex == roomnum );
	}

	sUserImageData* p = (sUserImageData*)mPartyHashMap.GetAt( userIdx );
	if( p )
	{
		p->mMapNum = mapnum;
		p->mRoomNum = roomnum;
		p->mChannelNum = channelnum;
		p->mShow = isSameMap;
		mNeedUpdate = true;
	}
}

void cSmallMiniMapWindow::AddUnion( sUnionUserData* data )
{
	if( data == 0 )
	{
		assert(0);
		return;
	}

	if( data->userIndex == HERO->GetObjectID() )
		return;

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

	cString unionSkinName;
	int groupIdx = PARTYUNIONMAN->GetGroup( data->userIndex );
	switch( groupIdx )
	{
	case eUNION_GROUP1:	unionSkinName = "SmallMiniMap_Union1"; break;
	case eUNION_GROUP2: unionSkinName = "SmallMiniMap_Union2"; break;
	case eUNION_GROUP3: unionSkinName = "SmallMiniMap_Union3"; break;
	default:			assert(0); return;
	}

	cUINodeSkin* unionSkin = pSkin->GetNodeSkin( unionSkinName );
	if( unionSkin && unionSkin->mpTexture )
	{
		sUserImageData* p = new sUserImageData;
		p->mMapNum = data->mapNum;
		p->mUserIndex = data->userIndex;
		p->mName = data->name;
		p->mShow = ( p->mMapNum == HERO->GetMapNum() );

		p->mpImage = new cUIImage( unionSkin->mpTexture );
		unsigned int tx = unionSkin->mSkinInfo->mTexX;
		unsigned int ty = unionSkin->mSkinInfo->mTexY;
		unsigned int w = unionSkin->mSkinInfo->mTexWidth;
		unsigned int h = unionSkin->mSkinInfo->mTexHeight;
		p->mPos.mX = (int)( data->posX / segLen * MAPTEX_W );
		p->mPos.mY = (int)( MAPTEX_H - ( data->posY / segLen * MAPTEX_H ) );

		p->mOldX = p->mPos.mX;
		p->mOldY = p->mPos.mY;
		p->mSize.mWidth = w;
		p->mSize.mHeight = h;

		p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );
		p->mpImage->SetScreenRect( cUIRect( p->mPos.mX, p->mPos.mY, p->mPos.mX + w, p->mPos.mY + h ) );

		if( mPartyUnionHashMap.Insert( data->userIndex, p ) == false )
		{
			assert(0);
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}

		mNeedUpdate = true;
	}
}

void cSmallMiniMapWindow::DeleteUnion( unsigned long userIdx )
{
	sUserImageData* p = (sUserImageData*)mPartyUnionHashMap.GetAt( userIdx );
	if( p )
	{
		SAFE_DELETE( p->mpImage );		
		SAFE_DELETE( p );
		mPartyUnionHashMap.Erase( userIdx );
	}
}

void cSmallMiniMapWindow::UpdateUnion( unsigned long userIdx, float posX, float posY )
{
	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	float segLen = navi->GetSegmentLength();

	sUserImageData* p = (sUserImageData*)mPartyUnionHashMap.GetAt( userIdx );
	if( p )
	{
		int newX = (int)( posX / segLen * MAPTEX_W );
		int newY = (int)( MAPTEX_H - ( posY / segLen * MAPTEX_H ) );

		if( newX != p->mOldX ||
			newY != p->mOldY )
		{
			p->mPos.mX = newX;
			p->mPos.mY = newY;
			p->mOldX = newX;
			p->mOldY = newY;

			mNeedUpdate = true;
		}
	}

	if( PVPMAN->GetPVPType() == MAP_FLAG )
	{
		cDataHashMap::cIterator i = mFlagMap.Begin();
		cDataHashMap::cIterator end = mFlagMap.End();
		for( ; i != end; ++i )
		{
			unsigned long objectIdx = (unsigned long)(i->mFirst);
			sUserImageData* p = (sUserImageData*)(i->mSecond);
			if( p && p->mUserIndex == userIdx )
			{
				AddFlag( objectIdx, userIdx, posX, posY );
				break;
			}
		}
	}
}


void cSmallMiniMapWindow::AddPortal( unsigned long index )
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

		p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );
		p->mpImage->SetScreenRect( cUIRect( p->mPos.mX, p->mPos.mY, p->mPos.mX + w, p->mPos.mY + h ) );

		if( mPortalHashMap.Insert( index, p ) == false )
		{
			assert(0);
		}

		mNeedUpdate = true;
	}
}

void cSmallMiniMapWindow::AddFindPosition( float xpos, float ypos )
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

		int x = mpPositionData->mPos.mX;
		int y = mpPositionData->mPos.mY;
		int w = mpPositionData->mSize.mWidth;
		int h = mpPositionData->mSize.mHeight;

		mpPositionData->mpImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		mNeedUpdate = true;
	}
}

void cSmallMiniMapWindow::DeleteFindPsotion()
{
	if( mpPositionData )
	{
		mpPositionData->mShow = false;
		mpPositionData->mEnableShow = false;
	}
}

void cSmallMiniMapWindow::AddFlag( unsigned long objectIdx, unsigned long userIdx, float xpos, float ypos )
{
	if( PVPMAN->IsEnableBattle() == false )
		return;

	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	float segLen = navi->GetSegmentLength();

	sUserImageData* data = (sUserImageData*)mFlagMap.GetAt( objectIdx );
	if( data )
	{
		// update
		data->mUserIndex = userIdx;

		data->mPos.mX = (int)( xpos / segLen * MAPTEX_W );
		data->mPos.mY = (int)( MAPTEX_H - ( ypos / segLen * MAPTEX_H ) );

		int x = data->mPos.mX;
		int y = data->mPos.mY;
		int w = data->mSize.mWidth;
		int h = data->mSize.mHeight;

		if( data->mpImage )
			data->mpImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}
	else
	{
		// add
		cUISkin* pSkin = UIMAN->GetSkin();
		if( !pSkin )
			return;

		// 깃발 이미지 로드
		cUINodeSkin* flagSkin = pSkin->GetNodeSkin( "SmallMiniMap_Flag" );
		if( flagSkin && flagSkin->mpTexture )
		{
			sUserImageData* p = new sUserImageData;
			p->mShow = true;
			p->mEnableShow = true;
			p->mUserIndex = userIdx;

			cUIImage* image = new cUIImage( flagSkin->mpTexture );
			unsigned int tx = flagSkin->mSkinInfo->mTexX;
			unsigned int ty = flagSkin->mSkinInfo->mTexY;
			unsigned int tw = flagSkin->mSkinInfo->mWidth;
			unsigned int th = flagSkin->mSkinInfo->mHeight;
			int x = (int)( xpos / segLen * MAPTEX_W );
			int y = (int)( MAPTEX_H - ( ypos / segLen * MAPTEX_H ) );

			image->SetTextureRect( tx, ty, tx + tw, ty + th );
			image->SetScreenRect( cUIRect( x, y, x + tw, y + th ) );

			p->mPos.mX = x;
			p->mPos.mY = y;
			p->mSize.mWidth = tw;
			p->mSize.mHeight = th;
			p->mpImage = image;

			if( mFlagMap.Insert( objectIdx, p ) == false )
			{
				assert(0);
				SAFE_DELETE( p->mpImage );
				SAFE_DELETE( p );
			}
		}
	}

	mNeedUpdate = true;
}

void cSmallMiniMapWindow::DeleteFlag( unsigned long objectIdx )
{
	sUserImageData* data = (sUserImageData*)mFlagMap.GetAt( objectIdx );
	if( data )
	{
		SAFE_DELETE( data->mpImage );
		SAFE_DELETE( data );
		mFlagMap.Erase( objectIdx );
	}
}

void cSmallMiniMapWindow::AddFlagPoint( unsigned long objectIdx, float xpos, float ypos )
{
	if( PVPMAN->IsEnableBattle() == false )
		return;

	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return;

	float segLen = navi->GetSegmentLength();

	cUINodeSkin* flagPointSkin = pSkin->GetNodeSkin( "SmallMiniMap_FlagPoint" );
	if( flagPointSkin && flagPointSkin->mpTexture )
	{
		sAniImageData* p = new sAniImageData;
		p->mShow = true;
		p->mEnableShow = true;

		/// 애니메이션세팅
		sAniParam* param = new sAniParam;
		param->mTexX = flagPointSkin->mSkinInfo->mTexX;
		param->mTexY = flagPointSkin->mSkinInfo->mTexY;
		param->mWidth = flagPointSkin->mSkinInfo->mWidth;
		param->mHeight = flagPointSkin->mSkinInfo->mHeight;
		param->mTimeOut = 500;
		param->mLastAccumTime = 0;
		param->mFrame = 2;
		param->mAniOn = false;
		param->mCurFrame = 0;
		param->mReturn = false;
		p->mAniParam = param;

		p->mpImage = new cUIImage( flagPointSkin->mpTexture );
		p->mPos.mX = (int)( xpos / segLen * MAPTEX_W );
		p->mPos.mY = (int)( MAPTEX_H - ( ypos / segLen * MAPTEX_H ) );
		p->mSize.mWidth = param->mWidth;
		p->mSize.mHeight = param->mHeight;

		unsigned int tx = param->mTexX;
		unsigned int ty = param->mTexY;
		int w = p->mSize.mWidth;
		int h = p->mSize.mHeight;

		p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );
		p->mpImage->SetScreenRect( cUIRect( p->mPos.mX, p->mPos.mY, p->mPos.mX + w, p->mPos.mY + h ) );

		if( mFlagPointMap.Insert( objectIdx, p ) == false )
		{
			assert(0);
			SAFE_DELETE( p->mAniParam );
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}

		mNeedUpdate = true;
	}
}

void cSmallMiniMapWindow::DeleteFlagPoint( unsigned long objectIdx )
{
	sAniImageData* p = (sAniImageData*)mFlagPointMap.GetAt( objectIdx );
	if( p )
	{
		SAFE_DELETE( p->mAniParam );
		SAFE_DELETE( p->mpImage );
		SAFE_DELETE( p );
		mFlagPointMap.Erase( objectIdx );
	}
}

void cSmallMiniMapWindow::AddStone( float posX, float posY, unsigned char teamType )
{
	if( PVPMAN->GetPVPType() != MAP_DEATHMATCH )
	{
		ClearStone();
		return;
	}

	/// 이미지 보이기/숨기기
	cNaviMesh* navi = WORLDMAN->GetNaviMesh();
	if( !navi )
	{
		assert(0);
		return;
	}

	float segLen = navi->GetSegmentLength();

	cUINodeSkin* stoneSkin = mPvpTeamAStone;
	if( teamType == ePVPDM_TEAMTYPE_B )
		stoneSkin = mPvpTeamBStone;
		
	if( stoneSkin && stoneSkin->mpTexture )
	{
		sBaseImageData* p = new sBaseImageData;
		p->mShow = true;
		p->mEnableShow = true;

		cUIImage* image = new cUIImage( stoneSkin->mpTexture );
		unsigned int tx = stoneSkin->mSkinInfo->mTexX;
		unsigned int ty = stoneSkin->mSkinInfo->mTexY;
		unsigned int tw = stoneSkin->mSkinInfo->mWidth;
		unsigned int th = stoneSkin->mSkinInfo->mHeight;
		int x = (int)( posX / segLen * MAPTEX_W );
		int y = (int)( MAPTEX_H - ( posY / segLen * MAPTEX_H ) );

		image->SetTextureRect( tx, ty, tx + tw, ty + th );
		image->SetScreenRect( cUIRect( x, y, x + tw, y + th ) );

		p->mPos.mX = x;
		p->mPos.mY = y;
		p->mSize.mWidth = tw;
		p->mSize.mHeight = th;
		p->mpImage = image;

		if( mPvPStoneMap.Insert( teamType, p ) == false )
		{
			assert(0);
			SAFE_DELETE( p->mpImage );
			SAFE_DELETE( p );
		}

		mNeedUpdate = true;
	}
}

void cSmallMiniMapWindow::DeleteStone( unsigned char teamType )
{
	sBaseImageData* data = (sBaseImageData*)mPvPStoneMap.GetAt( teamType );
	if( data )
	{
		SAFE_DELETE( data->mpImage );
		SAFE_DELETE( data );
		mPvPStoneMap.Erase( teamType );
	}
}

//
void cSmallMiniMapWindow::UpdateNpc( short tl, short tt, unsigned short tr, unsigned short tb )
{
	cDataHashMap::cIterator i = mNpcHashMap.Begin();
	cDataHashMap::cIterator end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mpImage && p->mpAniImage )
		{
			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			p->mShow = false;

			/// 출력 보정 ( 미니맵 범위안에 들어가야만 출력 )
			if( p->mPos.mX - cx >= tl && p->mPos.mX + cx <= tr && 
				p->mPos.mY - cy >= tt && p->mPos.mY + cy <= tb )
			{
				/// 윈도우사이즈에 맞게 변환 
				int x = (p->mPos.mX - tl) * MAP_W;
				int y = (p->mPos.mY - tt) * MAP_H;

				/// 텍스쳐상 의서의 위치 재계산
				p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) - cx + GetAbsoluteRect().mLeft; 
				p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) - cy + GetAbsoluteRect().mTop;

				p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				p->mpAniImage->SetPos( p->mScreenPos.mX + PIXNPCANI_X, p->mScreenPos.mY - PIXNPCANI_Y );

				p->mShow = true;
			}
		}
	}
}

void cSmallMiniMapWindow::UpdateParty( short tl, short tt, unsigned short tr, unsigned short tb, double xr, double yr, short heroScreenX, short heroScreenY )
{
	if( HERO->GetPartyIndex() == 0 )
		return;

	unsigned short heroMapNum = HERO->GetMapNum();
	unsigned short heroRoomIndex = HERO->GetPartyRoomIndex();
	DWORD		   heroChannelNum = HERO->GetChannelNum();

	cDataHashMap::cIterator i = mPartyHashMap.Begin();
	cDataHashMap::cIterator end = mNpcHashMap.End();
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

			int userX = p->mPos.mX;
			int userY = p->mPos.mY;

			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			// 미니맵 내부
			if( userX - cx >= tl && userX + cx <= tr && 
				userY - cy >= tt && userY + cy <= tb )
			{
				p->mpImage->SetTexture( mPartyInSkin->mpTexture );
				unsigned int tx = mPartyInSkin->mSkinInfo->mTexX;
				unsigned int ty = mPartyInSkin->mSkinInfo->mTexY;
				unsigned int w = mPartyInSkin->mSkinInfo->mTexWidth;
				unsigned int h = mPartyInSkin->mSkinInfo->mTexHeight;
				p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );

				/// 윈도우사이즈에 맞게 변환 
				int x = (userX - tl) * MAP_W;
				int y = (userY - tt) * MAP_H;

				/// 텍스쳐상 의서의 위치 재계산
				p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) - cx + GetAbsoluteRect().mLeft; 
				p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) - cy + GetAbsoluteRect().mTop;
				p->mpImage->SetScreenRect( cUIRect( p->mScreenPos.mX, p->mScreenPos.mY, p->mScreenPos.mX + w, p->mScreenPos.mY + h ) );
			}
			// 화살표 표시
			else
			{
				p->mpImage->SetTexture( mPartyOutSkin->mpTexture );
				unsigned int tx = mPartyOutSkin->mSkinInfo->mTexX;
				unsigned int ty = mPartyOutSkin->mSkinInfo->mTexY;
				unsigned int w = mPartyOutSkin->mSkinInfo->mTexWidth;
				unsigned int h = mPartyOutSkin->mSkinInfo->mTexHeight;
				p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );

				if( userX < tl )
					userX = tl;

				if( userX > tr )
					userX = tr;

				if( userY < tt )
					userY = tt;

				if( userY > tb )
					userY = tb;

				/// 윈도우사이즈에 맞게 변환 
				int x = (userX - tl) * MAP_W;
				int y = (userY - tt) * MAP_H;

				double nxr = ( x / mPerTexW ) + MAP_X; 
				double nyr = ( y / mPerTexH ) + MAP_Y;

				// 유저 위치 얻어오기
				float hx = (float)(xr - (short)(mHeroSize.mWidth * 0.5f ));
				float hy = (float)(yr - (short)(mHeroSize.mHeight * 0.5f ));
				NiPoint3 hero( (float)hx, (float)hy, 0 );
				NiPoint3 user( (float)(nxr - w), (float)(nyr - h), 0 );

				/// 유저 방향 얻어내기
				NiPoint3 userDir = user - hero;
				userDir.Unitize(); 

				/// 방향에 따른 각도 구하기
				float dot = userDir.Dot( -NiPoint3::UNIT_Y ); 
				float ang = NiACos( dot ); 					  

				if( userDir.x > 0.f )
					ang = -ang;

				/// 회전
				p->mpImage->SetRotate( ang );

				/// 반지름 이동
				short r = (short)(GetAbsoluteRect().GetWidth() * 0.5f);
				int screenPosX = (short)heroScreenX + (short)(userDir.x * r);
				int screenPosY = (short)heroScreenY + (short)(userDir.y * r);

				/// 사각형에 붙여놓기
				if( screenPosX < MAP_X )
					screenPosX = MAP_X;

				if( screenPosX + w > MAP_X + MAP_W )
					screenPosX = MAP_X + MAP_W - w;

				if( screenPosY < MAP_Y )
					screenPosY = MAP_Y;

				if( screenPosY + h > MAP_Y + MAP_H )
					screenPosY = MAP_Y + MAP_H - h;

				p->mScreenPos.mX = screenPosX + GetAbsoluteRect().mLeft;
				p->mScreenPos.mY = screenPosY + GetAbsoluteRect().mTop;
				p->mpImage->SetScreenRect( cUIRect( p->mScreenPos.mX, p->mScreenPos.mY, p->mScreenPos.mX + w, p->mScreenPos.mY + h ) );
			}
		}
	}
}

void cSmallMiniMapWindow::UpdatePortal( short tl, short tt, unsigned short tr, unsigned short tb )
{
	cDataHashMap::cIterator i = mPortalHashMap.Begin();
	cDataHashMap::cIterator end = mPortalHashMap.End();
	for( ; i != end; ++i )
	{
		sPortalImageData* p = (sPortalImageData*)(i->mSecond);
		if( p && p->mpImage )
		{
			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			p->mShow = false;

			/// 출력 보정 ( 미니맵 범위안에 들어가야만 출력 )
			if( p->mPos.mX - cx >= tl && p->mPos.mX + cx <= tr && 
				p->mPos.mY - cy >= tt && p->mPos.mY + cy <= tb )
			{
				/// 윈도우사이즈에 맞게 변환 
				int x = (p->mPos.mX - tl) * MAP_W;
				int y = (p->mPos.mY - tt) * MAP_H;

				/// 텍스쳐상 의서의 위치 재계산
				p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) - cx + GetAbsoluteRect().mLeft; 
				p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) - cy + GetAbsoluteRect().mTop;
				p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				p->mShow = true;
			}
		}
	}
}

void cSmallMiniMapWindow::UpdatePartyUnion( short tl, short tt, unsigned short tr, unsigned short tb )
{
	if( HERO->GetPartyUnionIndex() == 0 )
		return;

	cDataHashMap::cIterator i = mPartyUnionHashMap.Begin();
	cDataHashMap::cIterator end = mPartyUnionHashMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p && p->mpImage )
		{
			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			p->mShow = false;

			/// 출력 보정 ( 미니맵 범위안에 들어가야만 출력 )
			if( p->mPos.mX - cx >= tl && p->mPos.mX + cx <= tr && 
				p->mPos.mY - cy >= tt && p->mPos.mY + cy <= tb )
			{
				/// 윈도우사이즈에 맞게 변환 
				int x = (p->mPos.mX - tl) * MAP_W;
				int y = (p->mPos.mY - tt) * MAP_H;

				/// 텍스쳐상 의서의 위치 재계산
				p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) - cx + GetAbsoluteRect().mLeft; 
				p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) - cy + GetAbsoluteRect().mTop;
				p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
				p->mShow = true;
			}
		}
	}
}

void cSmallMiniMapWindow::UpdateFindPos( short tl, short tt, unsigned short tr, unsigned short tb )
{
	if( HERO->IsActiveFindPositon() == false )
		return;

	if( mpPositionData && mpPositionData->mpImage )
	{
		sAniImageData* p = mpPositionData;

		if( p->mEnableShow )
			p->mShow = false;

		int cx = (int)(p->mSize.mWidth * 0.5f);
		int cy = (int)(p->mSize.mHeight * 0.5f);

		/// 출력 보정 ( 미니맵 범위안에 들어가야만 출력 )
		if( p->mPos.mX - cx >= tl && p->mPos.mX + cx <= tr && 
			p->mPos.mY - cy >= tt && p->mPos.mY + cy <= tb )
		{
			/// 윈도우사이즈에 맞게 변환 
			int x = (p->mPos.mX - tl) * MAP_W;
			int y = (p->mPos.mY - tt) * MAP_H;

			/// 텍스쳐상 의서의 위치 재계산
			p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) - cx + GetAbsoluteRect().mLeft; 
			p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) - cy + GetAbsoluteRect().mTop;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );

			if( p->mEnableShow )
				p->mShow = true;
		}
	}
}

void cSmallMiniMapWindow::UpdateFlag( short tl, short tt, unsigned short tr, unsigned short tb )
{
	if( PVPMAN->IsEnableBattle() == false )
		return;

	cDataHashMap::cIterator i = mFlagMap.Begin();
	cDataHashMap::cIterator end = mFlagMap.End();
	for( ; i != end; ++i )
	{
		sUserImageData* p = (sUserImageData*)(i->mSecond);
		if( p->mEnableShow )
			p->mShow = false;

		int cx = (int)(p->mSize.mWidth * 0.5f);
		int cy = (int)(p->mSize.mHeight * 0.5f);

		/// 출력 보정 ( 미니맵 범위안에 들어가야만 출력 )
		if( p->mPos.mX - cx >= tl && p->mPos.mX + cx <= tr && 
			p->mPos.mY - cy >= tt && p->mPos.mY + cy <= tb )
		{
			/// 윈도우사이즈에 맞게 변환 
			int x = (p->mPos.mX - tl) * MAP_W;
			int y = (p->mPos.mY - tt) * MAP_H;

			/// 텍스쳐상 의서의 위치 재계산
			// 깃발 아랫부분을 기준으로 맞춘다
			p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) + GetAbsoluteRect().mLeft; 
			p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) + GetAbsoluteRect().mTop - p->mSize.mHeight;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );

			if( p->mEnableShow )
				p->mShow = true;
		}
	}
}

void cSmallMiniMapWindow::UpdateFlagPoint( short tl, short tt, unsigned short tr, unsigned short tb )
{
	if( PVPMAN->IsEnableBattle() == false )
		return;

	cDataHashMap::cIterator i = mFlagPointMap.Begin();
	cDataHashMap::cIterator end = mFlagPointMap.End();
	for( ; i != end; ++i )
	{
		sAniImageData* p = (sAniImageData*)(i->mSecond);
		if( p )
		{
			if( p->mEnableShow )
				p->mShow = false;

			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			/// 출력 보정 ( 미니맵 범위안에 들어가야만 출력 )
			if( p->mPos.mX - cx >= tl && p->mPos.mX + cx <= tr && 
				p->mPos.mY - cy >= tt && p->mPos.mY + cy <= tb )
			{
				/// 윈도우사이즈에 맞게 변환 
				int x = (p->mPos.mX - tl) * MAP_W;
				int y = (p->mPos.mY - tt) * MAP_H;

				/// 텍스쳐상 의서의 위치 재계산
				p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) - cx + GetAbsoluteRect().mLeft; 
				p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) - cy + GetAbsoluteRect().mTop;
				p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );

				if( p->mEnableShow )
					p->mShow = true;
			}
		}
	}
}

void cSmallMiniMapWindow::UpdateStone( short tl, short tt, unsigned short tr, unsigned short tb )
{
	if( PVPMAN->IsEnableBattle() == false )
		return;

	cDataHashMap::cIterator i = mPvPStoneMap.Begin();
	cDataHashMap::cIterator end = mPvPStoneMap.End();
	for( ; i != end; ++i )
	{
		sBaseImageData* p = (sBaseImageData*)(i->mSecond);
		if( p->mEnableShow )
			p->mShow = false;

		int cx = (int)(p->mSize.mWidth * 0.5f);
		int cy = (int)(p->mSize.mHeight * 0.5f);

		/// 출력 보정 ( 미니맵 범위안에 들어가야만 출력 )
		if( p->mPos.mX - cx >= tl && p->mPos.mX + cx <= tr && 
			p->mPos.mY - cy >= tt && p->mPos.mY + cy <= tb )
		{
			/// 윈도우사이즈에 맞게 변환 
			int x = (p->mPos.mX - tl) * MAP_W;
			int y = (p->mPos.mY - tt) * MAP_H;

			/// 텍스쳐상 의서의 위치 재계산
			// 깃발 아랫부분을 기준으로 맞춘다
			p->mScreenPos.mX = (int)(( x / mPerTexW ) + MAP_X ) - cx + GetAbsoluteRect().mLeft; 
			p->mScreenPos.mY = (int)(( y / mPerTexH ) + MAP_Y ) - cy + GetAbsoluteRect().mTop;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );

			if( p->mEnableShow )
				p->mShow = true;
		}
	}
}


/// 이름 정보 위치 업데이트
void cSmallMiniMapWindow::UpdateNamePos()
{
	if( mName.IsEmpty() )
		return;

	if( !(mpHoverData && mpHoverData->mpImage && mpHoverData->mShow ) )
		return;

	cUIRect rc;
	rc.mLeft = mpHoverData->mScreenPos.mX;
	rc.mTop = mpHoverData->mScreenPos.mY;
	rc.mRight = rc.mLeft + mpHoverData->mSize.mWidth;
	rc.mBottom = rc.mTop + mpHoverData->mSize.mHeight;

	/// 맵 이미지 좌표 비교후 보정
	int mapLeft = GetAbsoluteRect().mLeft + MAP_X;
	int mapTop = GetAbsoluteRect().mTop + MAP_Y;

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
		else if( mJobPos.mX + textWidth >= mapLeft + (int)MAP_W )
			mJobPos.mX = mapLeft + MAP_W - textWidth;

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
	else if( mNamePos.mX + textWidth >= mapLeft + (int)MAP_W )
		mNamePos.mX = mapLeft + MAP_W - textWidth;

	/// 너무 위에 찍히는 것을 방지
	if( mJobName.IsEmpty() == false )
		mNamePos.mY = mJobPos.mY + 2 + textHeight;
}

///팀원 수장 표시
void cSmallMiniMapWindow::SetPvpLeader( unsigned long userIdx )
{

	/// 예전 수장 이미지 파티 이미지로 변경
	sUserImageData* pOldLeader = (sUserImageData*)mPartyUnionHashMap.GetAt( mOldPvpLeaderIndex );
	if( pOldLeader )
	{
		cUISkin* pSkin = UIMAN->GetSkin();
		if( !pSkin )
			return;

		cString unionSkinName;
		int groupIdx = PARTYUNIONMAN->GetGroup( pOldLeader->mUserIndex );
		switch( groupIdx )
		{
		case eUNION_GROUP1:	unionSkinName = "SmallMiniMap_Union1"; break;
		case eUNION_GROUP2: unionSkinName = "SmallMiniMap_Union2"; break;
		case eUNION_GROUP3: unionSkinName = "SmallMiniMap_Union3"; break;
		default:			assert(0); return;
		}

		cUINodeSkin* unionSkin = pSkin->GetNodeSkin( unionSkinName );
		if( unionSkin && unionSkin->mpTexture )
		{
			pOldLeader->mpImage->SetTexture( unionSkin->mpTexture );
			unsigned short tx = (unsigned short)unionSkin->mSkinInfo->mTexX;
			unsigned short ty = (unsigned short)unionSkin->mSkinInfo->mTexY;
			unsigned short w = (unsigned short)unionSkin->mSkinInfo->mTexWidth;
			unsigned short h = (unsigned short)unionSkin->mSkinInfo->mTexHeight;
			pOldLeader->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );

			mNeedUpdate = true;
		}
	}

	mOldPvpLeaderIndex = userIdx;

	/// 새로운 수장 이미지 적용
	sUserImageData* pNewLeader = (sUserImageData*)mPartyUnionHashMap.GetAt( userIdx );
	if( pNewLeader )
	{
		if( mPvpPartyLeader && mPvpPartyLeader->mpTexture )
		{
			pNewLeader->mpImage->SetTexture( mPvpPartyLeader->mpTexture );
			unsigned short tx = (unsigned short)mPvpPartyLeader->mSkinInfo->mTexX;
			unsigned short ty = (unsigned short)mPvpPartyLeader->mSkinInfo->mTexY;
			unsigned short w = (unsigned short)mPvpPartyLeader->mSkinInfo->mTexWidth;
			unsigned short h = (unsigned short)mPvpPartyLeader->mSkinInfo->mTexHeight;
			pNewLeader->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );

			mNeedUpdate = true;
		}
	}
}

bool cSmallMiniMapWindow::ContainPoint( const cUIPos& pos ) const
{
	/// 알파 체크 하는 부분을 생략한다.
	if( cUINode::ContainPoint( pos ) == false )
		return false;

	return true;
}
