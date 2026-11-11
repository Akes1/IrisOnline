#include "StdAfx.h"
#include "UIImage.h"

//#include "RenderSystem.h"

cUIImage::cUIImage()
: mpScreenElement( 0 )
, mpTextureProp( 0 )
, mpAlphaProp( 0 )
{
	mTextureWidth = 1;
	mTextureHeight = 1;

	mTexRect.Set( 0, 0, 1, 1);

	mScreenX = 0;
	mScreenY = 0;

	mAngle = 0.0f;
	mCenter = NiPoint2::ZERO;

// 	mpScreenElement = NiNew NiMeshScreenElements( false, true, 1 );
// 	if( !mpScreenElement ) 
// 	{
// 		assert(0);
// 		return;
// 	}
//
// 	for( unsigned int i=0; i<4; i++ )
// 	{
// 		mVertex[i].x = i/2? 1.0f:-1.0f;
// 		mVertex[i].y = i%3? 1.0f:-1.0f;
// 	}
// 
// 	/// 4점을 생성한다.
// 	mpScreenElement->Insert( 4 );
// 
// 	/// 컬러 정보를 셋팅한다.
// 	mpScreenElement->SetColors( 0, NiColorA::WHITE );
// 	mColor = NiColorA::WHITE;
// 
// 	/// use vertex colors
// 	NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
// 	pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
// 	pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
// 	mpScreenElement->AttachProperty( pVertex );
// 
// 	NiZBufferProperty* pZ = NiNew NiZBufferProperty;
// 	pZ->SetZBufferTest( false );
// 	pZ->SetZBufferWrite( false );
// 	mpScreenElement->AttachProperty( pZ );
// 
// 	mpAlphaProp = NiNew NiAlphaProperty();
// 	mpAlphaProp->SetAlphaBlending( true );
// 	mpScreenElement->AttachProperty( mpAlphaProp );
// 
// 	mpScreenElement->UpdateProperties();
// 	NiMeshUpdateProcess kUpdateProcess;
// 	mpScreenElement->Update( kUpdateProcess );
}

cUIImage::cUIImage( NiTexture* pTex )
{
	if( pTex == 0 )
	{
		assert(0);
		return;
	}

// 	mTextureWidth = pTex->GetWidth();
// 	mTextureHeight = pTex->GetHeight();
	mTextureWidth = 0;
	mTextureHeight = 0;

	mTexRect.Set( 0, 0, 1, 1);

	mScreenX = 0;
	mScreenY = 0;

	mAngle = 0.0f;
	mCenter = NiPoint2::ZERO;

//	/// 관리용 스크린 객체를 생성한다.
//	mpScreenElement = NiNew NiMeshScreenElements( false, true, 1 );
// 	if( !mpScreenElement ) 
// 	{
// 		assert(0);
// 		return;
// 	}
//
// 	for( unsigned int i=0; i<4; i++ )
// 	{
// 		mVertex[i].x = i/2? 1.0f:-1.0f;
// 		mVertex[i].y = i%3? 1.0f:-1.0f;
// 	}
// 
// 	/// 4점을 생성한다.
// 	mpScreenElement->Insert( 4 );
// 
// 	/// 컬러 정보를 셋팅한다.
// 	mpScreenElement->SetColors( 0, NiColorA::WHITE );
// 	mColor = NiColorA::WHITE;
//
//	/// 관리 Property정보를 생성한다.
// 	mpTextureProp = NiNew NiTexturingProperty();
// 	mpTextureProp->SetBaseTexture( pTex );
// 	mpTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
// 	mpScreenElement->AttachProperty( mpTextureProp );
//
//	/// use vertex colors
// 	NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
// 	pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
// 	pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
// 	mpScreenElement->AttachProperty( pVertex );
// 
// 	NiZBufferProperty* pZ = NiNew NiZBufferProperty;
// 	pZ->SetZBufferTest( false );
// 	pZ->SetZBufferWrite( false );
// 	mpScreenElement->AttachProperty( pZ );
// 
// 	mpAlphaProp = NiNew NiAlphaProperty();
// 	mpAlphaProp->SetAlphaBlending( true );
// 	mpScreenElement->AttachProperty( mpAlphaProp );
// 
// 	mpScreenElement->UpdateProperties();
// 
// 	NiMeshUpdateProcess kUpdateProcess;
// 	mpScreenElement->Update( kUpdateProcess );
}

cUIImage::cUIImage( NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th )
{
	if( tex == 0 )
	{
		assert(0);
		return;
	}

	mTextureWidth = tex->GetWidth();
	mTextureHeight = tex->GetHeight();

	mTexRect.Set( tx, ty, tw, th );

	mScreenX = 0;
	mScreenY = 0;

	mAngle = 0.0f;
	mCenter = NiPoint2::ZERO;

	mpScreenElement = NiNew NiMeshScreenElements( false, true, 1 );
	if( !mpScreenElement ) 
	{
		assert(0);
		return;
	}

	for( unsigned int i=0; i<4; i++ )
	{
		mVertex[i].x = i/2? 1.0f:-1.0f;
		mVertex[i].y = i%3? 1.0f:-1.0f;
	}

	/// 관리 Property정보를 생성한다.
	mpTextureProp = NiNew NiTexturingProperty();
	mpTextureProp->SetBaseTexture( tex );
	mpTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
	mpScreenElement->AttachProperty( mpTextureProp );

	/// use vertex colors
	NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
	pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
	pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
	mpScreenElement->AttachProperty( pVertex );

	NiZBufferProperty* pZ = NiNew NiZBufferProperty;
	pZ->SetZBufferTest( false );
	pZ->SetZBufferWrite( false );
	mpScreenElement->AttachProperty( pZ );

	mpAlphaProp = NiNew NiAlphaProperty();
	mpAlphaProp->SetAlphaBlending( true );
	mpScreenElement->AttachProperty( mpAlphaProp );

	mpScreenElement->AddNewScreenRect( (short)y, (short)x, (unsigned short)tw, (unsigned short)th,
		(unsigned short)ty, (unsigned short)tx, NiColorA::WHITE);

	mColor = NiColorA::WHITE;

	mpScreenElement->UpdateProperties();

	NiMeshUpdateProcess kUpdateProcess;
	mpScreenElement->Update( kUpdateProcess );
}



cUIImage::~cUIImage()
{
	mpTextureProp = 0;
	mpAlphaProp = 0;

	if( mpScreenElement )
	{
		assert( mpScreenElement->GetRefCount() == 1 );
		mpScreenElement = 0;
	}
//	SAFE_NIDELETE(mpScreenElement);
}

/// 주의사항 : 
void cUIImage::SetTexture( NiTexture* ptex )
{
	if( ptex == 0 || mpScreenElement == 0 )
	{
		assert(0);
		return;
	}

	mTextureWidth = ptex->GetWidth();
	mTextureHeight = ptex->GetHeight();

	if( mpTextureProp )
	{
		mpTextureProp->SetBaseTexture( ptex );
	}
	else
	{
		mpTextureProp = NiNew NiTexturingProperty();
		mpTextureProp->SetBaseTexture( ptex );
		mpTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
		mpScreenElement->AttachProperty( mpTextureProp );
	}

	mpScreenElement->UpdateProperties();

	NiMeshUpdateProcess kUpdateProcess;
	mpScreenElement->Update( kUpdateProcess );
}

void cUIImage::SetPos( int x, int y )
{
	/// 위치 정보를 셋팅한다.
	/// 주의 : 스크린상의 상대 좌표를 사용한다.
	mScreenX = (short)x;
	mScreenY = (short)y;

	SetTranslate( mScreenX, mScreenY );
}

void cUIImage::SetScreenWH( unsigned int screenW, unsigned int screenH )
{
	/// 위치 정보를 셋팅한다.
	/// 주의 : 스크린상의 상대 좌표를 사용한다.
	mWidthOnScreen = screenW;
	mHeightOnScreen = screenH;

	mScaleX = (float)mWidthOnScreen * 0.5f;
	mScaleY = (float)mHeightOnScreen * 0.5f;

	SetTranslate( mScreenX, mScreenY );
}

void cUIImage::SetScreenRect( const cUIRect& rect )
{
	/// 위치 정보를 셋팅한다.
	/// 주의 : 스크린상의 상대 좌표를 사용한다.
	mScreenX = rect.mLeft;
	mScreenY = rect.mTop;
	mWidthOnScreen = rect.GetWidth();
	mHeightOnScreen = rect.GetHeight();

	mScaleX = (float)mWidthOnScreen * 0.5f;
	mScaleY = (float)mHeightOnScreen * 0.5f;

	SetTranslate( mScreenX, mScreenY );

}

void cUIImage::SetTextureRect( const cUIRect& rect )
{
	if( mpScreenElement == 0 )
	{
		assert(0);
		return;
	}
	SetTextureRect( rect.mLeft, rect.mTop, rect.mRight, rect.mBottom );
}


void cUIImage::SetTextureRect( unsigned int tl, unsigned int tt, unsigned int tr, unsigned int tb )
{
	if( mpScreenElement == 0 )
	{
		assert(0);
		return;
	}

	mTexRect.Set( tl, tt, tr, tb );

	/// 텍스쳐 UV셋팅
	//	float localTexL = (float)(tl+0.5f) / (float)mTextureWidth;
	float localTexL = (float)tl / (float)mTextureWidth;
	float localTexT = (float)tt / (float)mTextureHeight;
	float localTexR = (float)tr / (float)mTextureWidth;
	float localTexB = (float)tb / (float)mTextureHeight;

	mpScreenElement->SetTextures( 0, 0, localTexL, localTexT, localTexR, localTexB );
}

void cUIImage::SetColor( float r, float g, float b, float a )
{
	SetColor( NiColorA(r, g, b, a) );
}

void cUIImage::SetColor( const NiColorA& color )
{
	if( mpScreenElement == 0 )
	{
		assert(0);
		return;
	}

	mpScreenElement->SetColors( 0, color );
	mColor = color;
}

void cUIImage::SetAlpha( float a )
{
	SetColor( mColor.r, mColor.g, mColor.b, a );
}

void cUIImage::Draw()
{
	if( mpScreenElement == 0 )
	{
		assert(0);
		return;
	}

	mpScreenElement->RenderImmediate( NiRenderer::GetRenderer() );
}

void cUIImage::UpdateImage()
{
	SetRotate( mAngle );
}

///
void cUIImage::SetTranslate( int x, int y )
{
	float cx = (float)x + mScaleX;
	float cy = (float)y + mScaleY;

	mScreenX = x;
	mScreenY = y;

	/// 중앙을 기준으로 한다.
	//	if( mCenter.x != cx || mCenter.y != cy )
	{
		mCenter.x = cx;
		mCenter.y = cy;
		SetRotate( mAngle );
	}
}

void cUIImage::SetRotate(float angle )
{
	if( mpScreenElement == 0 )
	{
		assert(0);
		return;
	}

// 	float width = (float)RENDERSYS->GetScreenWidth();
// 	float height = (float)RENDERSYS->GetScreenHeight();
	float width = 1024;
	float height = 768;


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
