#include "StdAfx.h"
#include "UIImage.h"


cUIImage::cUIImage( NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th )
{
	if( tex == 0 )
	{
		assert(0);
		return;
	}

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

	mpScreenElement->AddNewScreenRect( (short)y, (short)x, (unsigned short)tw, (unsigned short)th,
		(unsigned short)ty, (unsigned short)tx, NiColorA::WHITE);

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

	mpScreenElement->UpdateProperties();
	mpScreenElement->Update( 0.0f );
}

cUIImage::~cUIImage()
{
	mpTextureProp = 0;
	mpAlphaProp = 0;

	if( mpScreenElement )
	{
		NiDelete mpScreenElement;
		mpScreenElement = 0;
	}
}


void cUIImage::SetAlpha( float a )
{
	NiColorA color = NiColorA::WHITE;
	color.a = a;

	mpScreenElement->SetColors( 0, color );
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