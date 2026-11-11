#include "stdafx.h"
#include "planeobject.h"
#include "RenderSystem.h"
#include "ResourceManager.h"


cPlaneObject::cPlaneObject()
{
	mpScreenElement = 0;

	mpAlphaProp = 0;
	mpTextureProp = 0;

	mTextureWidth = 1;
	mTextureHeight = 1;

	mTexLeft = 0;
	mTexTop = 0;
	mTexRight = 0;
	mTexBottom = 0;

	mScreenX = 0;
	mScreenY = 0;
	mScreenW = 1;
	mScreenH = 1;

	mAngle = 0.0f;
	mCenter = NiPoint2::ZERO;

	mScaleX = 0;
	mScaleY = 0;

	mRenderTargetWidth = 0;
	mRenderTargetHeight = 0;

	for( int i = 0; i < 4; ++i )
		mVertex[i] = NiPoint2::ZERO;
}

cPlaneObject::~cPlaneObject()
{
	mpTextureProp = 0;
	mpAlphaProp = 0;

	if( mpScreenElement )
	{
		assert( mpScreenElement->GetRefCount() == 1 );
	}
	mpScreenElement = 0;
//	SAFE_NIDELETE( mpScreenElement );
}

/// 주의 : 스크린상에서 사용하는 이미지 영역과 텍스쳐상의 영역의 크기가 다를 수 있다...(늘여찍기!!)
bool cPlaneObject::Create( const char* pathName, 
						   int screenX, int screenY, unsigned int screenW, unsigned int screenH,
						   unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom )
{
	NiTexture* tex = RESOURCEMAN->LoadTexture( pathName, false );
	if( tex == 0 ) 
	{
		assert(0);
		return false;
	}

	return Create( tex, screenX, screenY, screenW, screenH, texLeft, texTop, texRight, texBottom );
}

/// 주의 : 스크린상에서 사용하는 이미지 영역과 텍스쳐상의 영역의 크기가 다를 수 있다...(늘여찍기!!)
bool cPlaneObject::Create(  NiTexture* pTexture, 
							int screenX, int screenY, unsigned int screenW, unsigned int screenH,
							unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom )
{
	if( !pTexture ) 
		return false;

	mTextureWidth = pTexture->GetWidth();
	mTextureHeight = pTexture->GetHeight();

	mRenderTargetWidth = 0;
	mRenderTargetHeight = 0;

	/// 관리용 스크린 객체를 생성한다.
	mpScreenElement = NiNew NiMeshScreenElements( false, true, 1 );
	if( !mpScreenElement ) 
		return false;

	for( unsigned int i=0; i<4; i++ )
	{
		mVertex[i].x = i/2? 1.0f:-1.0f;
		mVertex[i].y = i%3? 1.0f:-1.0f;
	}

	/// 4점을 생성한다.
	mpScreenElement->Insert( 4 );

	/// 컬러 정보를 셋팅한다.
	mpScreenElement->SetColors( 0, NiColorA::WHITE );

	/// 텍스쳐 UV셋팅
	SetTextureRect( texLeft, texTop, texRight, texBottom );

	/// 위치 정보를 셋팅한다.
	SetScreenRect( screenX, screenY, screenW, screenH );

	/// 관리 Property정보를 생성한다.
	mpTextureProp = NiNew NiTexturingProperty();
	mpTextureProp->SetBaseTexture( pTexture );
	mpTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
	mpScreenElement->AttachProperty( mpTextureProp );

	/// use mVertex colors
	NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
	pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
	pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
	mpScreenElement->AttachProperty( pVertex );

	mpAlphaProp = NiNew NiAlphaProperty();
	mpAlphaProp->SetAlphaBlending( true );
	mpScreenElement->AttachProperty( mpAlphaProp );

	mpScreenElement->UpdateProperties();

	NiMeshUpdateProcess kUpdateProcess;
	mpScreenElement->Update( kUpdateProcess );

	return true;
}

void cPlaneObject::SetScreenXY( int screenX, int screenY )
{
	/// 위치 정보를 셋팅한다.
	/// 주의 : 스크린상의 상대 좌표를 사용한다.
	mScreenX = screenX;
	mScreenY = screenY;

	SetTranslate( mScreenX, mScreenY );
}

void cPlaneObject::SetScreenX( int screenX )
{
	SetScreenXY( screenX, mScreenY );
}

void cPlaneObject::SetScreenY( int screenY )
{
	SetScreenXY( mScreenX, screenY );
}

void cPlaneObject::SetScreenWH( unsigned int screenW, unsigned int screenH )
{
	/// 위치 정보를 셋팅한다.
	/// 주의 : 스크린상의 상대 좌표를 사용한다.
	mScreenW = screenW;
	mScreenH = screenH;

	mScaleX = (float)mScreenW * 0.5f;
	mScaleY = (float)mScreenH * 0.5f;

	SetTranslate( mScreenX, mScreenY );
}

void cPlaneObject::SetScreenWidth( unsigned int screenW )
{
	SetScreenWH( screenW, mScreenH );
}

void cPlaneObject::SetScreenHeight( unsigned int screenH )
{
	SetScreenWH( mScreenW, screenH );
}

void cPlaneObject::SetScreenRect( int screenX, int screenY, unsigned int screenW, unsigned int screenH )
{
	/// 위치 정보를 셋팅한다.
	/// 주의 : 스크린상의 상대 좌표를 사용한다.
	mScreenX = screenX;
	mScreenY = screenY;
	mScreenW = screenW;
	mScreenH = screenH;

	mScaleX = (float)mScreenW * 0.5f;
	mScaleY = (float)mScreenH * 0.5f;

	SetTranslate( mScreenX, mScreenY );
}

void cPlaneObject::SetRenderTargetWH( unsigned int width, unsigned int height ) 
{ 
	mRenderTargetWidth = width; 
	mRenderTargetHeight = height; 
	UpdateImage();
}

void cPlaneObject::SetTextureRect( unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom )
{
	mTexLeft = texLeft;
	mTexTop = texTop;
	mTexRight = texRight;
	mTexBottom = texBottom;

	/// 텍스쳐 UV셋팅
	//float localTexL = (float)(mTexLeft+0.5f) / (float)mTextureWidth;
	float localTexL = (float)mTexLeft / (float)mTextureWidth;
	float localTexT = (float)mTexTop / (float)mTextureHeight;
	float localTexR = (float)mTexRight / (float)mTextureWidth;
	float localTexB = (float)mTexBottom / (float)mTextureHeight;

	mpScreenElement->SetTextures( 0, 0, localTexL, localTexT, localTexR, localTexB );
}

void cPlaneObject::SetColorA( float r, float g, float b, float a )
{
	SetColorA( NiColorA(r, g, b, a) );
}

void cPlaneObject::SetColorA( NiColorA color )
{
	mpScreenElement->SetColors( 0, color );
}

void cPlaneObject::Draw()
{
	NiRenderer* pkRenderer = NiRenderer::GetRenderer();
//	pkRenderer->SetScreenSpaceCameraData();

	mpScreenElement->RenderImmediate( pkRenderer );
}

void cPlaneObject::UpdateImage()
{
	SetRotate( mAngle );
}

///
void cPlaneObject::SetTranslate( int x, int y )
{
	float cx = (float)x + mScaleX;
	float cy = (float)y + mScaleY;

	mScreenX = x;
	mScreenY = y;

	/// 중앙을 기준으로 한다.
	//if( mCenter.x != cx || mCenter.y != cy )
	{
		mCenter.x = cx;
		mCenter.y = cy;
		SetRotate( mAngle );
	}
}

void cPlaneObject::SetRotate(float angle )
{
	float width = (float)RENDERSYS->GetScreenWidth();
	float height = (float)RENDERSYS->GetScreenHeight();

	if( mRenderTargetWidth != 0 )
		width = (float)mRenderTargetWidth;
	if( mRenderTargetHeight != 0 )
		height = (float)mRenderTargetHeight;

	mAngle = angle;

	if( mAngle != 0.f )
	{
		float cos = NiCos( angle );
		float sin = NiSin( angle );

		NiPoint2 p;
		for( unsigned int i = 0; i < 4; ++i )
		{
			p.x = ( mVertex[i].x * cos * mScaleX + mVertex[i].y * sin * mScaleY );
			p.y = ( mVertex[i].y * cos * mScaleY - mVertex[i].x * sin * mScaleX );

			p += mCenter;

			p.x /= width;
			p.y /= height;

			mpScreenElement->SetVertex( 0, i, NiPoint2( p.x, p.y ) );
		}
	}
	else
	{
		NiPoint2 p;

		for( unsigned int i = 0; i < 4; ++i )
		{
			p.x = mVertex[i].x * mScaleX + mCenter.x;
			p.y = mVertex[i].y * mScaleY + mCenter.y;

			p.x /= width;
			p.y /= height;

			mpScreenElement->SetVertex( 0, i, p );
		}
	}
}