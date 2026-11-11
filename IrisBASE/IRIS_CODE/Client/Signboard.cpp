#include "StdAfx.h"
#include ".\signboard.h"

#include "Application.h"
#include "RenderSystem.h"
#include "ResourceManager.h"
#include "FontAgent.h"


static unsigned int TITLE_TEX_LEN = 13;

//////////////////////////////////////////////////////////////////////////
// cSignboard
cSignboard::cSignboard()
: mpBoxElement(0)
, mActive(false)
{
	mSignHeight = 0;
	mSignWidth = 0;

	mAllowTextWidth = 0;

	mScreenPosX = 0;
	mScreenPosY = 0;

	mType = eBOARD_TYPE_NONE;
}

cSignboard::~cSignboard()
{
	if( mpBoxElement )
	{
		assert( mpBoxElement->GetRefCount() == 1 );
		mpBoxElement = 0;
	}

//	SAFE_NIDELETE(mpBoxElement);
}

void cSignboard::Draw()
{
	if( IsActive() == false )
		return;

	if( mpBoxElement )
	{
		NiRenderer* pRenderer = NiRenderer::GetRenderer();
		mpBoxElement->RenderImmediate(pRenderer);
	}
}

bool cSignboard::PickPoint( float x, float y )
{
	if( IsActive() == false )
		return false;

	float px = x/(float)RENDERSYS->GetScreenWidth();
	float py = y/(float)RENDERSYS->GetScreenHeight();

	if( mpBoxElement )
	{
		float fLeft, fTop, fWidth, fHeight;
		if( mpBoxElement->GetRectangle( 0, fLeft, fTop, fWidth, fHeight ) == true )
		{
			if( fLeft <= px && fTop <= py && fLeft+fWidth >= px && fTop+fHeight >= py )
			{
				return true;
			}
		}
	}
	return false;
}

/*
void cSignboard::Update( NiPoint3& pos, unsigned long time )
{
	if( IsActive() == false )
		return;

	/// 좌표 계산 및 Update
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
	int screenX, screenY;
	WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), pos, x, y );

	screenX = (int)( x * RENDERSYS->GetScreenWidth() );
	screenY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );

	/// 시작위치 계산 및 보정
	screenX = screenX - mSignWidth/2;
	screenY = screenY - mSignHeight;

	///
	if( mScreenPosX == screenX && mScreenPosY == screenY )
		return;

	mScreenPosX = screenX;
	mScreenPosY = screenY;

	x = (float)screenX / (float)RENDERSYS->GetScreenWidth();
	y = (float)screenY / (float)RENDERSYS->GetScreenHeight();
	w = (float)mSignWidth / (float)RENDERSYS->GetScreenWidth();
	h = (float)mSignHeight / (float)RENDERSYS->GetScreenHeight();

	mpBoxElement->SetRectangle( 0, x, y, w, h );
}
*/

void cSignboard::SetPos( int screenX, int screenY )
{
	/// 좌표 계산 및 Update
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;

	/// 시작위치 계산 및 보정
	screenX = screenX - mSignWidth/2;
	screenY = screenY - mSignHeight;

	///
	if( mScreenPosX == screenX && mScreenPosY == screenY )
		return;

	mScreenPosX = screenX;
	mScreenPosY = screenY;

	x = (float)screenX / (float)RENDERSYS->GetScreenWidth();
	y = (float)screenY / (float)RENDERSYS->GetScreenHeight();
	w = (float)mSignWidth / (float)RENDERSYS->GetScreenWidth();
	h = (float)mSignHeight / (float)RENDERSYS->GetScreenHeight();

	mpBoxElement->SetRectangle( 0, x, y, w, h );
}


//////////////////////////////////////////////////////////////////////////
// cTarotSignboard
cTarotSignboard::cTarotSignboard()
{
	mType = eBOARD_TYPE_TAROT; 

	NiTexture* pTex = RESOURCEMAN->LoadTexture( "./Data/2DData/depthplane.tga", false );
	if( pTex )
	{
		float texWid = (float)pTex->GetWidth();
		float texHei = (float)pTex->GetHeight();

		mpBoxElement = NiNew NiMeshScreenElements( false, true, 1 );

		/// Property정보를 생성한다.
		NiTexturingProperty* pTextureProp = NiNew NiTexturingProperty;
		pTextureProp->SetBaseTexture( pTex );
		pTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
		mpBoxElement->AttachProperty( pTextureProp );

		NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
		pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
		pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
		mpBoxElement->AttachProperty( pVertex );

		NiAlphaProperty* pAlphaProp = NiNew NiAlphaProperty;
		pAlphaProp->SetAlphaBlending( true );
		mpBoxElement->AttachProperty( pAlphaProp );

		/// 적용
		mpBoxElement->UpdateProperties();

		NiMeshUpdateProcess kUpdateProcess;
		mpBoxElement->Update( kUpdateProcess );

		mpBoxElement->Insert( 4 );

		// 0	156 190	77
		float localTexL,localTexT,localTexR,localTexB;
		localTexL = 0.0f / texWid;
		localTexR = localTexL + 190.0f/texWid;
		localTexT = 156.0f/texHei;
		localTexB = localTexT + 77.0f/texHei;

		mpBoxElement->SetRectangle( 0, 0.0f, 0.0f, 0.0f, 0.0f );
		mpBoxElement->SetTextures( 0, 0, localTexL, localTexT, localTexR, localTexB );
		mpBoxElement->SetColors( 0, NiColorA(1.0f, 1.0f, 1.0f, 1.0f) );

		mSignWidth = 190;
		mSignHeight = 77;
		mAllowTextWidth = 143;
	}
}

cTarotSignboard::~cTarotSignboard()
{
}

void cTarotSignboard::Active( LPTSTR name, LPTSTR title, unsigned long color, bool loop )
{
	DeActive();

	int len = ::_tcslen( title );
	if( len <= 0 )
	{
		assert( 0 );
		return;
	}

	/// 두줄 찍기
	unsigned int extW = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, title, len );
	for( unsigned int i=0;i<2; i++ )
	{
		unsigned int curlen = len;
		while( extW > mAllowTextWidth )
		{
			curlen--;
			extW = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, title, curlen );
		}

		assert( curlen < 128 );
		Sstrncpy( mTitle[i].mString, 128, title, curlen );
		mTitle[i].mStrLen = curlen;
		mTitle[i].mColor = 0xFFFFFFFF;

		len -= curlen;
		title += curlen;

		if( *title == _T(' ') ) 
		{
			/// 다음 문자 시작이 스페이스면 스킵
			++title;
			len--;
		}

		if( len <= 0)
			break;
	}

	len = ::_tcslen( name );
	if( len )
	{
		assert(len < 128);
		Sstrncpy( mName.mString, 128, name, len );
		mName.mStrLen = len;
		mName.mColor = color;
	}

	mActive = true;
}


void cTarotSignboard::DeActive()
{
	cSignboard::DeActive();

	_tcscpy_s( mName.mString, _T("") );
	mName.mStrLen = 0;

	_tcscpy_s( mTitle[0].mString, _T("") );
	mTitle[0].mStrLen = 0;

	_tcscpy_s( mTitle[1].mString, _T("") );
	mTitle[1].mStrLen = 0;
}

void cTarotSignboard::Draw()
{
	if( IsActive() == false )
		return;

	cSignboard::Draw();

	POINT pos;

	/// name
	pos.x = mScreenPosX + 33;
	pos.y = mScreenPosY + 15;
	FONTAGENT->DrawText( cFontAgent::eFont_Chat, mName.mString, mName.mStrLen, pos.x, pos.y, mName.mColor );

	/// title
	if( mTitle[0].mStrLen )
	{
		pos.x = mScreenPosX + 27;
		pos.y = mScreenPosY + 35;
		FONTAGENT->DrawText( cFontAgent::eFont_Chat, mTitle[0].mString, mTitle[0].mStrLen, pos.x, pos.y, mTitle[0].mColor );

		if( mTitle[1].mStrLen )
		{
			pos.y += FONTAGENT->GetTextHeight( cFontAgent::eFont_Chat );
			FONTAGENT->DrawText( cFontAgent::eFont_Chat, mTitle[1].mString, mTitle[1].mStrLen, pos.x, pos.y, mTitle[1].mColor );
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// cUseSellSignboard
cUseSellSignboard::cUseSellSignboard()
{
	mType = eBOARD_TYPE_USER_SELL;

	NiTexture* pTex = RESOURCEMAN->LoadTexture( "./Data/2DData/depthplane.tga", false );
	if( pTex )
	{
		float texWid = (float)pTex->GetWidth();
		float texHei = (float)pTex->GetHeight();

		mpBoxElement = NiNew NiMeshScreenElements( false, true, 1 );

		/// Property정보를 생성한다.
		NiTexturingProperty* pTextureProp = NiNew NiTexturingProperty;
		pTextureProp->SetBaseTexture( pTex );
		pTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
		mpBoxElement->AttachProperty( pTextureProp );

		NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
		pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
		pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
		mpBoxElement->AttachProperty( pVertex );

		NiAlphaProperty* pAlphaProp = NiNew NiAlphaProperty;
		pAlphaProp->SetAlphaBlending( true );
		mpBoxElement->AttachProperty( pAlphaProp );

		/// 적용
		mpBoxElement->UpdateProperties();

		NiMeshUpdateProcess kUpdateProcess;
		mpBoxElement->Update( kUpdateProcess );

		mpBoxElement->Insert( 4 );

		// 판매노점 - 0 0 190 77
		// 구입노점 - 0 78 190 77
		float localTexL,localTexT,localTexR,localTexB;
		localTexL = 0.0f / texWid;
		localTexR = localTexL + 190.0f/texWid;
		localTexT = 78.0f/texHei;
		localTexB = localTexT + 77.0f/texHei;

		mpBoxElement->SetRectangle( 0, 0.0f, 0.0f, 0.0f, 0.0f );
		mpBoxElement->SetTextures( 0, 0, localTexL, localTexT, localTexR, localTexB );
		mpBoxElement->SetColors( 0, NiColorA(1.0f, 1.0f, 1.0f, 1.0f) );

		mSignWidth = 190;
		mSignHeight = 77;

		mAllowTextWidth = 143;
	}
}

cUseSellSignboard::~cUseSellSignboard()
{
}

void cUseSellSignboard::Active( LPTSTR name, LPTSTR title, unsigned long color, bool loop )
{
	DeActive();

	int len = ::_tcslen( title );
	if( len <= 0 )
	{
		assert( 0 );
		return;
	}

	/// 두줄 찍기
	unsigned int extW = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, title, len );
	for( unsigned int i=0;i<2; i++ )
	{
		unsigned int curlen = len;
		while( extW > mAllowTextWidth )
		{
			curlen--;
			extW = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, title, curlen );
		}

		assert(curlen < 128);
		Sstrncpy( mTitle[i].mString, 128, title, curlen );
		mTitle[i].mStrLen = curlen;
		mTitle[i].mColor = 0xFFFFFFFF;

		len -= curlen;
		title += curlen;

		if( *title == _T(' ') ) 
		{
			/// 다음 문자 시작이 스페이스면 스킵
			++title;
			len--;
		}

		if( len <= 0)
			break;
	}

	len = ::_tcslen( name );
	if( len )
	{
		assert(len < 128);
		Sstrncpy( mName.mString, 128, name, len );
		mName.mStrLen = len;
		mName.mColor = color;
	}

	mActive = true;
}

void cUseSellSignboard::DeActive()
{
	cSignboard::DeActive();

	_tcscpy_s( mName.mString, _T("") );
	mName.mStrLen = 0;

	_tcscpy_s( mTitle[0].mString, _T("") );
	mTitle[0].mStrLen = 0;

	_tcscpy_s( mTitle[1].mString, _T("") );
	mTitle[1].mStrLen = 0;
}


void cUseSellSignboard::Draw()
{
	if( IsActive() == false )
		return;

	cSignboard::Draw();

	POINT pos;

	/// name
	pos.x = mScreenPosX + 33;
	pos.y = mScreenPosY + 15;
	FONTAGENT->DrawText( cFontAgent::eFont_Chat, mName.mString, mName.mStrLen, pos.x, pos.y, mName.mColor );

	/// title
	if( mTitle[0].mStrLen )
	{
		pos.x = mScreenPosX + 27;
		pos.y = mScreenPosY + 35;
		FONTAGENT->DrawText( cFontAgent::eFont_Chat, mTitle[0].mString, mTitle[0].mStrLen, pos.x, pos.y, mTitle[0].mColor );

		if( mTitle[1].mStrLen )
		{
			pos.y += FONTAGENT->GetTextHeight( cFontAgent::eFont_Chat );
			FONTAGENT->DrawText( cFontAgent::eFont_Chat, mTitle[1].mString, mTitle[1].mStrLen, pos.x, pos.y, mTitle[1].mColor );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#include "Npc_Common.h"
#include "NpcScript.h"
#include "UIManager.h"
#include "icon.h"

cNpcSignboard::cNpcSignboard( unsigned long npcIndex )
{
	mType = eBOARD_TYPE_NPC_SIGN;

	mNpcIndex = npcIndex;

	sNPCList* info = NPCSCRIPT->GetNPCList( npcIndex );
	if( info )
	{
		cIconParam* pParam = UIMAN->GetIconParam( info->mJobIconIndexTarget );
		if( pParam == 0 )
		{
			assert(0);
			return;
		}

		NiTexture* pTex = pParam->mpTexture;
		if( pTex )
		{
			float texWid = (float)pTex->GetWidth();
			float texHei = (float)pTex->GetHeight();

			mpBoxElement = NiNew NiMeshScreenElements( false, true, 1 );

			/// Property정보를 생성한다.
			NiTexturingProperty* pTextureProp = NiNew NiTexturingProperty;
			pTextureProp->SetBaseTexture( pTex );
			pTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
			mpBoxElement->AttachProperty( pTextureProp );

			NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
			pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
			pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
			mpBoxElement->AttachProperty( pVertex );

			NiAlphaProperty* pAlphaProp = NiNew NiAlphaProperty;
			pAlphaProp->SetAlphaBlending( true );
			mpBoxElement->AttachProperty( pAlphaProp );

			/// 적용
			mpBoxElement->UpdateProperties();

			NiMeshUpdateProcess kUpdateProcess;
			mpBoxElement->Update( kUpdateProcess );

			mpBoxElement->Insert( 4 );

			float localTexL,localTexT,localTexR,localTexB;
			localTexL = pParam->mTexPos.mX / texWid;
			localTexR = localTexL + 32.0f/texWid;
			localTexT = pParam->mTexPos.mY /texHei;
			localTexB = localTexT + 32.0f/texHei;

			mpBoxElement->SetRectangle( 0, 0.0f, 0.0f, 32.0f, 32.0f );
			mpBoxElement->SetTextures( 0, 0, localTexL, localTexT, localTexR, localTexB );
			mpBoxElement->SetColors( 0, NiColorA(1.0f, 1.0f, 1.0f, 1.0f) );

			mSignWidth = 32;
			mSignHeight = 32;
		}
	}
}

void cNpcSignboard::SetPos( int screenX, int screenY )
{
	cSignboard::SetPos( screenX, screenY );

	mActive = true;
}

//////////////////////////////////////////////////////////////////////////
#include "TutorialManager.h"

cTutorialSignboard::cTutorialSignboard( unsigned char type, unsigned long classIndex, unsigned long objectIndex )
: mObjectClassIndex(0)
, mObjectType(0)
, mObjectIndex(0)
, mpTextureProp(0)
, mCurFrame(0)
, mBeginAnim(false)
, mLastAccumTime(0)
, mpImageData(0)
, mPosX(0)
, mPosY(0)
{
	mType = eBOARD_TYPE_TUTORIAL_SIGN;

	mObjectClassIndex = classIndex;
	mObjectType = type;
	mObjectIndex = objectIndex;

	UpdateImage();
}

cTutorialSignboard::~cTutorialSignboard()
{
	mpTextureProp = 0;
	mpImageData = 0;
}

void cTutorialSignboard::ClearImage()
{
	mpImageData = 0;
	DeActive();
}

void cTutorialSignboard::UpdateImage()
{
	mpImageData = TUTORIALMAN->GetObjectImage( mObjectType, mObjectClassIndex, mObjectIndex );
	if( mpImageData == 0 )
	{
		DeActive();
		return;
	}

	if( mpBoxElement == 0 )
	{
		mpBoxElement = NiNew NiMeshScreenElements( false, true, 1 );

		/// Property정보를 생성한다.
		mpTextureProp = NiNew NiTexturingProperty;
		mpTextureProp->SetBaseTexture( mpImageData->mpTexture );
		mpTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
		mpBoxElement->AttachProperty( mpTextureProp );

		NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
		pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
		pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
		mpBoxElement->AttachProperty( pVertex );

		NiAlphaProperty* pAlphaProp = NiNew NiAlphaProperty;
		pAlphaProp->SetAlphaBlending( true );
		mpBoxElement->AttachProperty( pAlphaProp );

		/// 적용
		mpBoxElement->UpdateProperties();

		NiMeshUpdateProcess kUpdateProcess;
		mpBoxElement->Update( kUpdateProcess );

		mpBoxElement->Insert( 4 );
	}
	else
	{
		mpTextureProp->SetBaseTexture( mpImageData->mpTexture );

		/// 적용
		mpBoxElement->UpdateProperties();

		NiMeshUpdateProcess kUpdateProcess;
		mpBoxElement->Update( kUpdateProcess );
	}

	//
	float texWid = (float)mpImageData->mpTexture->GetWidth();
	float texHei = (float)mpImageData->mpTexture->GetHeight();

	float localTexL,localTexT,localTexR,localTexB;
	localTexL = mpImageData->mTexPosX / texWid;
	localTexR = localTexL + mpImageData->mWidth/texWid;
	localTexT = mpImageData->mTexPosY /texHei;
	localTexB = localTexT + mpImageData->mHeight/texHei;

	mpBoxElement->SetRectangle( 0, 0.0f, 0.0f, mpImageData->mWidth, mpImageData->mHeight );
	mpBoxElement->SetTextures( 0, 0, localTexL, localTexT, localTexR, localTexB );
	mpBoxElement->SetColors( 0, NiColorA(1.0f, 1.0f, 1.0f, 1.0f) );

	mSignWidth = mpImageData->mWidth;
	mSignHeight = mpImageData->mHeight;

	mPosX = mpImageData->mPosX;
	mPosY = mpImageData->mPosY;

	//
	mCurFrame = 0;

	if( mpImageData->mFrameCount > 1 )
	{
		mBeginAnim = true;
		mLastAccumTime = THEAPP->GetWorldAccumTime();
	}

	mActive = true;
}

void cTutorialSignboard::Update( unsigned long accumTime )
{
	if( mBeginAnim == false )
		return;

	if( mpImageData == 0 )
		return;

	if( accumTime - mLastAccumTime >= mpImageData->mOneFrameTime )
	{
		/// frame change
		if( mCurFrame >= mpImageData->mFrameCount )
		{
			if( mpImageData->mRepeat )
				mCurFrame = 0;
			else
			{
				mBeginAnim = false;
				mCurFrame = 0;
			}
		}
		mLastAccumTime = accumTime;

		unsigned int tx = mpImageData->mTexPosX + ( mCurFrame * mpImageData->mWidth );
		unsigned int ty = mpImageData->mTexPosY;

		if( mpTextureProp )
		{
			NiTexture* tex = mpTextureProp->GetBaseTexture();
			if( tex )
			{
				float texWid = (float)tex->GetWidth();
				float texHei = (float)tex->GetHeight();

				float localTexL,localTexT,localTexR,localTexB;
				localTexL = tx / texWid;
				localTexR = localTexL + mpImageData->mWidth/texWid;
				localTexT = ty /texHei;
				localTexB = localTexT + mpImageData->mHeight/texHei;

				if( mpBoxElement )
					mpBoxElement->SetTextures( 0, 0, localTexL, localTexT, localTexR, localTexB );
			}
			else
				assert(0);
		}
		else
			assert(0);
		
		mCurFrame++;
	}
}
