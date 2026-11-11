#include "stdafx.h"
#include "CashFreeMoveWindow.h"

#include "Application.h"
#include "Stage_Common.h"
#include "NpcScript.h"
#include "StageScript.h"

#include "FontAgent.h"
#include "ComboBox.h"
#include "UIImage.h"
#include "Icon.h"

#include "NaviMesh.h"
#include "NaviField.h"
#include "UICursor.h"

#include "WorldManager.h"
#include "CameraManager.h"
#include "ResourceManager.h"
#include "GameResourceManager.h"
#include "ItemManager.h"
#include "ChatManager.h"
#include "ObjectManager.h"
#include "Hero.h"

#include "SmallMiniMapWindow.h"


cCashFreeMoveWindow::cCashFreeMoveWindow()
: mpMapList(0)
, mpMapImage(0)
, mpMapSkin(0)
, mOldX(-1.f)
, mOldY(-1.f)
, mNeedUpdate( false )
, mpHoverData(0)
, mInventorySlotIndex(USHRT_MAX)
{
	mIsSetCursorDefault = false;

	mHeroDir = -NiPoint3::UNIT_Y;
	mCameraDir = -NiPoint3::UNIT_Y;

	SetBackupVisible( false );
}

cCashFreeMoveWindow::~cCashFreeMoveWindow()
{
	Clear();

	SAFE_DELETE( mpHeroImage );
	SAFE_DELETE( mpCameraImage );
	SAFE_DELETE( mpMapImage );

}

void cCashFreeMoveWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cCashFreeMoveWindow::Clear()
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

	mName.Clear();
	mJobName.Clear();
	mNamePos.mX = 0;
	mNamePos.mY = 0;
	mJobPos.mX = 0;
	mJobPos.mY = 0;

	// 자유 이동 주문서 락 풀기
	if( mInventorySlotIndex != USHRT_MAX )
	{
		if( mInventorySlotIndex <= MAX_INVENTORY )
		{
			if( ITEMMAN )
			{
				cItem* Item = ITEMMAN->GetItem( mInventorySlotIndex );
				if( Item )
					Item->SetLock( false );
			}
		}

		mInventorySlotIndex = USHRT_MAX; 
	}

}

bool cCashFreeMoveWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMapList = (cComboBox*)GetChild( eUIID_GAME_CASH_FREEMOVE_COMBO );
	if( !mpMapList )
	{
		assert(0);
		return false;
	}

	if( NPCSCRIPT ) 
		NPCSCRIPT->StageInit( MAP_NONE, DEFAULT_MAPMODE );

	for( int index = MAP_MIN; index <= MAP_MAX; ++index )
	{
		if( GAMERESOURCEMAN->IsValidMap( index ) == false )
			continue;

		mpMapList->AddItem( GAMERESOURCEMAN->GetMapName( index ), (void*)index );
	}

	mpMapList->SetSelect( 0, false );


	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	// MAP 이미지
	mpMapSkin = pSkin->GetNodeSkin( "MapLoading_Image" );
	if( !mpMapSkin )
	{
		assert(0);
		return false;
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

	return true;
}

void cCashFreeMoveWindow::OnCommand( cUINode* , unsigned int id )
{
	switch ( id )
	{
	case eUIID_GAME_CASH_FREEMOVE_CLOSE:
		Hide();
		break;
	default:
		assert(0);
		break;
	}
}

void cCashFreeMoveWindow::OnShow()
{
	cUIWindow::OnShow();
	if( mpMapList && mpMapSkin )
	{
		/// 자동으로 현재 맵에 선택하도록 설정
		unsigned short heroMapIdx = HERO->GetMapNum();
		if( heroMapIdx >= MAP_MIN && heroMapIdx <= MAP_MAX ) 
		{
			unsigned int mapListSize = mpMapList->GetSize();
			for( unsigned int cnt = 0; cnt < mapListSize; ++cnt )
			{
				int mapIdx = (int)mpMapList->GetExtraData( cnt );
				if( mapIdx == heroMapIdx )
				{
					mpMapList->SetSelect( cnt, false );
					break;
				}
			}
		}
		else
			mpMapList->SetSelect( 0, false );

		int mapIndex = (int)mpMapList->GetSelectData();
		if( mapIndex < MAP_MIN || mapIndex > MAP_MAX )
		{
			assert(0);
			return;
		}

		unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIndex, DEFAULT_MAPMODE );

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
			SAFE_DELETE( mpMapImage );

		int x = GetAbsoluteRect().mLeft + mpMapSkin->mSkinInfo->mX;
		int y = GetAbsoluteRect().mTop  + mpMapSkin->mSkinInfo->mY;
		int width = x + mpMapSkin->mSkinInfo->mWidth;
		int height = y + mpMapSkin->mSkinInfo->mHeight;

		/// 텍스쳐 설정
		mpMapImage = new cUIImage( ptex );
		mpMapImage->SetTextureRect( 0, 0, MAPTEX_W, MAPTEX_H );
		mpMapImage->SetScreenRect( cUIRect( x, y, width, height ) );
		mpMapImage->SetAlpha( 0.7f );

		/// npc리스트 설정.
		ClearNpc();
		typedef tList<void*> cNpcList;
		cNpcList npcList;
		NPCSCRIPT->GetNPCRegen( MAP_NONE, mapIndex, &npcList );
		if( npcList.IsEmpty() == false )
		{
			cNpcList::cIterator pos = npcList.Begin();
			cNpcList::cIterator end = npcList.End();
			for( int index = 0; pos != end; ++pos, ++index )
			{
				sNPCRegen* pNPCRegen = (sNPCRegen*)(*pos);
				if( pNPCRegen )
				{
					sNpcData tempNpcData;
					tempNpcData.mNpcIdx		 = index;
					tempNpcData.mNpcClassIdx = pNPCRegen->mNpcClassIdx;
					tempNpcData.mMapNumber   = pNPCRegen->mMapNumber;
					tempNpcData.mPosX		 = pNPCRegen->mPosX;
					tempNpcData.mPosY		 = pNPCRegen->mPosY;
					tempNpcData.mDirection   = pNPCRegen->mDirection;
					
					AddNpc( &tempNpcData );
				}
			}
		}

		/// 맵포탈 정보 세팅
		ClearPotal();
		tPointerArray<void*>* pMapArray = STAGESCRIPT->GetMapChangeStarterArr( (unsigned short)mapIndex );
		if( pMapArray->IsEmpty() == false )
		{
			for( unsigned int i = 0; i < pMapArray->GetSize(); ++i )
			{
				sStageChangePos* p = (sStageChangePos*)(*pMapArray)[i];
				AddPortal( p->mPosIdx );
			}
		}

		mPosStr.Clear();
	}
}

void cCashFreeMoveWindow::Show( unsigned short slotIndex )
{
	// 자유 이동 주문서 아이템 락 풀기
	if( mInventorySlotIndex != USHRT_MAX )
	{
		if( mInventorySlotIndex <= MAX_INVENTORY )
		{
			cItem* Item = ITEMMAN->GetItem( mInventorySlotIndex );
			if( Item )
				Item->SetLock( false );
		}

		mInventorySlotIndex = USHRT_MAX; 
	}

	// 셋팅
	mInventorySlotIndex = slotIndex;

	// 자유 이동 주문서 아이템 락 걸기
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
		item->SetLock( true );

	cUIWindow::Show();
}

void cCashFreeMoveWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();
}

void cCashFreeMoveWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpMapImage && mpMapSkin )
	{
		int x = GetAbsoluteRect().mLeft + mpMapSkin->mSkinInfo->mX;
		int y = GetAbsoluteRect().mTop  + mpMapSkin->mSkinInfo->mY;
		int width  = x + mpMapSkin->mSkinInfo->mWidth;
		int height = y + mpMapSkin->mSkinInfo->mHeight;
		
		mpMapImage->SetScreenRect( cUIRect( x, y, width, height ) );
	}

	cUIRect mapRect;
	if( mpMapImage )
		mapRect = mpMapImage->GetScreenRect();
	
	cDataHashMap::cIterator i, end;
	i = mNpcHashMap.Begin();
	end = mNpcHashMap.End();
	for( ; i != end; ++i )
	{
		sNpcImageData* p = (sNpcImageData*)(i->mSecond);
		if( p && p->mpImage )
		{
			int cx = (int)(p->mSize.mWidth * 0.5f);
			int cy = (int)(p->mSize.mHeight * 0.5f);

			p->mScreenPos.mX = (int)p->mPos.mX - cx + mapRect.mLeft; 
			p->mScreenPos.mY = (int)p->mPos.mY - cy + mapRect.mTop;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
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

			p->mScreenPos.mX = (int)p->mPos.mX - cx + mapRect.mLeft; 
			p->mScreenPos.mY = (int)p->mPos.mY - cy + mapRect.mTop;
			p->mpImage->SetPos( p->mScreenPos.mX, p->mScreenPos.mY );
		}
	}

	mNeedUpdate = true;
}

void cCashFreeMoveWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	mNeedUpdate = true;
}

void cCashFreeMoveWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 지도 렌더링
	if( mpMapImage )
		mpMapImage->Draw();

	
	// npc 이미지 출력
	cDataHashMap::cIterator npcPos = mNpcHashMap.Begin();
	cDataHashMap::cIterator npcEnd = mNpcHashMap.End();
	for( ; npcPos != npcEnd; ++npcPos )
	{
		sNpcImageData* p = (sNpcImageData*)(npcPos->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
	}

	// portal 이미지 출력
	cDataHashMap::cIterator potalPos = mPortalHashMap.Begin();
	cDataHashMap::cIterator potalEnd = mPortalHashMap.End();

	for( ; potalPos != potalEnd; ++potalPos )
	{
		sPortalImageData* p = (sPortalImageData*)(potalPos->mSecond);
		if( p && p->mShow && p->mpImage )
			p->mpImage->Draw();
	}

	if( HERO->GetMapNum() == (unsigned short)mpMapList->GetSelectData() )
	{
		// 카메라
		if( mpCameraImage )
			mpCameraImage->Draw();

		// hero
		if( mpHeroImage )
			mpHeroImage->Draw();
	}

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}

	/// 이름 라벨 출력 
	if( mJobName )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mJobName.Cstr()), mJobPos.mX, mJobPos.mY, COLOR_WHITE, true );

	if( mName )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mName.Cstr()), mNamePos.mX, mNamePos.mY, COLOR_WHITE, true );

	/// 좌표 출력
	if( !mPosStr.IsEmpty() )
	{
		cUIRect mapRect;
		if( mpMapImage )
			mapRect = mpMapImage->GetScreenRect();
		else
			mapRect = GetAbsoluteRect();

		int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)mPosStr.Cstr(), mPosStr.GetLength() );	
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mPosStr.Cstr()), mapRect.mRight - width - 30, mapRect.mTop + 50, COLOR_WHITE );
	}


	pKeeper->DrawAll();
}

void cCashFreeMoveWindow::OnComboSelected( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_CASH_FREEMOVE_COMBO:
		if( mpMapList && mpMapSkin )
		{
			int mapIndex = (int)mpMapList->GetSelectData();
			if( mapIndex < MAP_MIN || mapIndex > MAP_MAX )
			{
				assert(0);
				return;
			}

			unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIndex, 0 );

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
				SAFE_DELETE( mpMapImage );

			int x = GetAbsoluteRect().mLeft + mpMapSkin->mSkinInfo->mX;
			int y = GetAbsoluteRect().mTop  + mpMapSkin->mSkinInfo->mY;
			int width = x + mpMapSkin->mSkinInfo->mWidth;
			int height = y + mpMapSkin->mSkinInfo->mHeight;

			/// 텍스쳐 설정
			mpMapImage = new cUIImage( ptex );
			mpMapImage->SetTextureRect( 0, 0, MAPTEX_W, MAPTEX_H );
			mpMapImage->SetScreenRect( cUIRect( x, y, width, height ) );
			mpMapImage->SetAlpha( 0.7f );

			/// npc 이미지 설정.
			ClearNpc();
			typedef tList<void*> cNpcList;
			cNpcList npcList;
			NPCSCRIPT->GetNPCRegen( MAP_NONE, mapIndex, &npcList );
			if( npcList.IsEmpty() == false )
			{
				cNpcList::cIterator pos = npcList.Begin();
				cNpcList::cIterator end = npcList.End();
				for( int index = 0; pos != end; ++pos, ++index )
				{
					sNPCRegen* pNPCRegen = (sNPCRegen*)(*pos);
					if( pNPCRegen )
					{
						sNpcData tempNpcData;
						tempNpcData.mNpcIdx		 = index;
						tempNpcData.mNpcClassIdx = pNPCRegen->mNpcClassIdx;
						tempNpcData.mMapNumber   = pNPCRegen->mMapNumber;
						tempNpcData.mPosX		 = pNPCRegen->mPosX;
						tempNpcData.mPosY		 = pNPCRegen->mPosY;
						tempNpcData.mDirection   = pNPCRegen->mDirection;

						AddNpc( &tempNpcData );
					}
				}
			}

			/// 맵포탈 정보 세팅
			ClearPotal();
			tPointerArray<void*>* pMapArray = STAGESCRIPT->GetMapChangeStarterArr( (unsigned short)mapIndex );
			if( pMapArray->IsEmpty() == false )
			{
				for( unsigned int i = 0; i < pMapArray->GetSize(); ++i )
				{
					sStageChangePos* p = (sStageChangePos*)(*pMapArray)[i];
					AddPortal( p->mPosIdx );
				}
			}

		}
		break;
	}
}

void cCashFreeMoveWindow::OnMouseMove( const cUIPos& pos )
{
	cUIWindow::OnMouseMove( pos );

	cUIRect rect;
	cDataHashMap::cIterator i, end;
	bool hoverd = false;

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

					mName = p->mName;
					mJobName.Format( _T("<%s>"), (LPCTSTR)p->mJobName.Cstr() );
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

	cUIRect mapRect;
	if( mpMapImage )
		mapRect = mpMapImage->GetScreenRect();

	/// 마우스 커서 바꾸기.
	if( mapRect.ContainPoint( pos ) == true )
	{
		int mapIndex = (int)mpMapList->GetSelectData();

		/// 일반맵은 모드가 0 이다.
 		cNaviField* field = WORLDMAN->GetNaviField( mapIndex, DEFAULT_MAPMODE );
 		if( !field )
		{
			assert(0);
 			return;
		}

 		/// 이동 가능영역 체크
 		int mouseX = pos.mX - mapRect.mLeft;
 		int mouseY = pos.mY - mapRect.mTop;
 
 		float segLen = field->GetSegmentLength();
 
 		unsigned int x = (unsigned int)((mouseX * segLen) / MAPTEX_W * 0.01f );
 		unsigned int y = (unsigned int)((MAPTEX_H - mouseY) * (segLen / MAPTEX_H) * 0.01f );

 		/// navifield check
 		if( field->GetValue( x, y ) != 0 )
		{
			CURSOR->SetCursor( eCURSOR_MOVESTOP );
			mPosStr.Format( _T("%d, %d"), (int)x, (int)y );
		}
 		else
 		{	
			CURSOR->SetCursor( eCURSOR_DEFAULT );
			mPosStr.Format( _T("%d, %d"), (int)x, (int)y );
		}
	}
	else
	{
		CURSOR->SetCursor( eCURSOR_DEFAULT );
		mPosStr.Clear();
	}
}

void cCashFreeMoveWindow::OnMouseLeft(const cUIPos& pos )
{
	cUIWindow::OnMouseLeft( pos );

	mPosStr.Clear();
}

bool cCashFreeMoveWindow::ContainPoint( const cUIPos& pos ) const
{
	/// 알파 체크 하는 부분을 생략한다.
	if( cUINode::ContainPoint( pos ) == false )
		return false;

	return true;
}

void cCashFreeMoveWindow::ClearNpc()
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

void cCashFreeMoveWindow::ClearPotal()
{
	// portal
	cDataHashMap::cIterator i = mPortalHashMap.Begin();
	cDataHashMap::cIterator end = mPortalHashMap.End();
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
}


void cCashFreeMoveWindow::AddNpc( sNpcData* data )
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

	cUIRect mapRect;
	if( mpMapImage )
		mapRect = mpMapImage->GetScreenRect();

	sNpcImageData* npcData = new sNpcImageData;
	npcData->mClassIndex	= data->mNpcClassIdx;
	npcData->mQuestEffect	= eNPCQUEST_EFFECT_NONE;
	npcData->mPos.mX		= (int)( data->mPosX / segLen * MAPTEX_W );
	npcData->mPos.mY		= (int)( MAPTEX_H - ( data->mPosY / segLen * MAPTEX_H ) );
	npcData->mName			= pNpcList->mName;
	npcData->mJobName		= pNpcList->mJobName;

	
	cIconParam* param = UIMAN->GetIconParam( pNpcList->mJobIconIndexLarge );
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
		npcData->mScreenPos.mX = (int)npcData->mPos.mX - cx + mapRect.mLeft; 
		npcData->mScreenPos.mY = (int)npcData->mPos.mY - cy + mapRect.mTop;

		image->SetTextureRect( tx, ty, tx + NPCICON_SIZE, ty + NPCICON_SIZE );
		image->SetScreenRect( cUIRect( npcData->mScreenPos.mX, npcData->mScreenPos.mY, 
			npcData->mScreenPos.mX + NPCICON_SIZE, npcData->mScreenPos.mY + NPCICON_SIZE ) );

		npcData->mpImage = image;
	}
	
	if( mNpcHashMap.Insert( data->mNpcIdx, npcData ) == false )
	{
		assert(0);
	}

	mNeedUpdate = true;
}

void cCashFreeMoveWindow::AddPortal( unsigned long index )
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

	cUIRect mapRect;
	if( mpMapImage )
		mapRect = mpMapImage->GetScreenRect();

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
		p->mScreenPos.mX = (int)p->mPos.mX - cx + mapRect.mLeft; 
		p->mScreenPos.mY = (int)p->mPos.mY - cy + mapRect.mTop;

		p->mpImage->SetTextureRect( tx, ty, tx + w, ty + h );
		p->mpImage->SetScreenRect( cUIRect( p->mScreenPos.mX, p->mScreenPos.mY, p->mScreenPos.mX + w, p->mScreenPos.mY + h ) );

		if( mPortalHashMap.Insert( index, p ) == false )
		{
			assert(0);
		}
	}
}

void cCashFreeMoveWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	cHero* hero = OBJECTMANAGER->GetHero();
	cCamera* camera = CAMERAMAN->GetCurrent();
	cNaviMesh* navi = WORLDMAN->GetNaviMesh();

	if( !hero || !camera || !navi )
		return;

	if( !mpHeroImage || !mpCameraImage || !mpMapImage )
		return;

	float segLen = navi->GetSegmentLength();

	float x = hero->GetXPos();
	float y = hero->GetYPos();

	if( mNeedUpdate || x != mOldX || y != mOldY )
	{
		mOldX = x;
		mOldY = y;

		cUIRect mapRect = mpMapImage->GetScreenRect();

		/// 텍스쳐 상의 캐릭터 위치 GetSegmentLength : 맵의 변 길이 
		int heroX = (int)(x / segLen * MAPTEX_W);
		int heroY = (int)(MAPTEX_H - (y / segLen * MAPTEX_H ));

		// hero 
		short heroScreenX = (short)heroX - (short)(mHeroSize.mWidth * 0.5f );
		short heroScreenY = (short)heroY - (short)(mHeroSize.mHeight * 0.5f );
		mpHeroImage->SetTranslate( (short)mapRect.mLeft + heroScreenX, (short)mapRect.mTop + heroScreenY );

		// camera
		short camScreenX = (short)heroX - (short)(mCameraSize.mWidth * 0.5f );
		short camScreenY = (short)heroY - (short)(mCameraSize.mHeight * 0.5f );
		mpCameraImage->SetTranslate( (short)mapRect.mLeft + camScreenX, (short)mapRect.mTop + camScreenY );

		//
		if( mpHoverData )
			UpdateNamePos();

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
}

/// 이름 정보 위치 업데이트
void cCashFreeMoveWindow::UpdateNamePos()
{
	if( !mpMapImage )
	{
		assert(0);
		return;
	}

	/// 맵 이미지 좌표 비교후 보정
	cUIRect mapRect = mpMapImage->GetScreenRect();

	if( mName.IsEmpty() )
		return;

	if( !(mpHoverData && mpHoverData->mpImage && mpHoverData->mShow) )
		return;

	cUIRect rc;
	rc.mLeft = mpHoverData->mScreenPos.mX;
	rc.mTop = mpHoverData->mScreenPos.mY;
	rc.mRight = rc.mLeft + mpHoverData->mSize.mWidth;
	rc.mBottom = rc.mTop + mpHoverData->mSize.mHeight;

	int textHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_System );

	/// 직업이름이 있으면 좌표 세팅
	if( mJobName.IsEmpty() == false )
	{
		/// 직업좌표 세팅
		int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, mJobName.Cstr(), mJobName.GetLength() );

		mJobPos.mX = (int)(rc.mLeft + ( rc.GetWidth() - textWidth ) * 0.5);
		mJobPos.mY = rc.mTop - 10 - textHeight;

		/// 이름이 튀어나가지 않도록 보정
		if( mJobPos.mX <= mapRect.mLeft )
			mJobPos.mX = mapRect.mLeft;
		else if( mJobPos.mX + textWidth >= mapRect.mRight )
			mJobPos.mX = mapRect.mRight - textWidth;

		/// 너무 위에 찍히는것을 방지
		if( mJobPos.mY <= mapRect.mTop )
			mJobPos.mY = rc.mBottom + 2;
	}

	/// 이름좌표 세팅
	int textWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, mName.Cstr(), mName.GetLength() );

	mNamePos.mX = (int)(rc.mLeft + ( rc.GetWidth() - textWidth ) * 0.5);
	mNamePos.mY = rc.mTop - 10;

	/// 이름이 튀어나가지 않도록 보정
	if( mNamePos.mX <= mapRect.mLeft )
		mNamePos.mX = mapRect.mLeft;
	else if( mNamePos.mX + textWidth >= mapRect.mRight )
		mNamePos.mX = mapRect.mRight - textWidth;

	/// 너무 위에 찍히는 것을 방지
	if( mJobName.IsEmpty() == false )
		mNamePos.mY = mJobPos.mY + 2 + textHeight;
}

void cCashFreeMoveWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	cUIWindow::OnLButtonDoubleClick( pos );

	cUIRect mapRect;
	if( mpMapImage )
		mapRect = mpMapImage->GetScreenRect();

	cHero* hero = HERO;

	/// 마우스 커서 바꾸기.
	if( mapRect.ContainPoint( pos ) == true )
	{
		// 사용가능한 맵인지 체크
		unsigned short heroMapIndex = hero->GetMapNum();
		if( ( heroMapIndex < MAP_MIN || heroMapIndex > MAP_MAX ) && ( heroMapIndex < MAP_THEME_MIN || heroMapIndex > MAP_THEME_MAX ) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12201 ) );
			return;
		}

		// 맵 이동 가능 좌표인지 체크
		int mapIndex = (int)mpMapList->GetSelectData();
		if( mapIndex < MAP_MIN || mapIndex > MAP_MAX )
		{
			assert(0);
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12203 ) );
			return;
		}

		/// 일반맵은 모드가 0 이다.
		cNaviField* field = WORLDMAN->GetNaviField( mapIndex, DEFAULT_MAPMODE );
		if( !field )
		{
			assert(0);
			return;
		}

		/// 자유 이동 주문서 아이템이 있나 체크
		if( mInventorySlotIndex == USHRT_MAX )
		{
			assert(0);
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12204 ) );
			return;
		}
	
		/// 자유 이동 주문서가 락이 걸렸는지 체크.
		cItem* item = ITEMMAN->GetItem( mInventorySlotIndex );
		if( item == 0 )
		{
			assert(0);
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12205 ) );
			return;
		}

		if( item->GetIndex() == 0 || item->IsLock() == false )
		{
			assert(0);
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12205 ) );
			return;
		}

		/// 이동 가능영역 체크
		int mouseX = pos.mX - mapRect.mLeft;
		int mouseY = pos.mY - mapRect.mTop;

		float segLen = field->GetSegmentLength();

		unsigned int x = (unsigned int)((mouseX * segLen) / MAPTEX_W * 0.01f );
		unsigned int y = (unsigned int)((MAPTEX_H - mouseY) * (segLen / MAPTEX_H) * 0.01f );

		cItemAbility* abil = item->GetDefine()->GetAbililty();
		if( abil == 0 )
		{
			assert(0);
			return;
		}

		unsigned long skillIndex = abil->GetInfluenceIndex();

		/// navifield check
		if( field->GetValue( x, y ) == 0 )
		{
			HERO->UseCashMapChangeSkill( skillIndex, mInventorySlotIndex, (unsigned short)mapIndex, (float)(x * 100), (float)(y * 100) );
		}
		else
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12202 ) );
			return;
		}

		Hide();
	}
}
