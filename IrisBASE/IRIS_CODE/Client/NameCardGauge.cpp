#include "StdAfx.h"
#include "namecardgauge.h"

#include "PlaneObject.h"

#include "RenderSystem.h"

cNameCardGauge::cNameCardGauge()
: mpGaugeTexture(0)
, mpFrameTexture(0)
, mpFrameBackTexture(0)
{
	mpGaugeTarget = 0;

	mpRenderTargetGroup = 0;
	mpRenderedTexture = 0;
}

cNameCardGauge::~cNameCardGauge()
{
	SAFE_DELETE(mpGaugeTarget);
	SAFE_DELETE(mpFrameBackTexture);
	SAFE_DELETE(mpFrameTexture);
	SAFE_DELETE(mpGaugeTexture);

	mpRenderTargetGroup = 0;
	mpRenderedTexture = 0;
}

bool cNameCardGauge::Init( NiTexture* pGaugeFile, RECT* GaugeUV, unsigned short GaugeTexW, unsigned short GaugeTexH, NiTexture* pFrameFile, RECT* FrameUV, RECT* FrameBack )
{
	if( pGaugeFile == 0 )
		return false;

	if( pFrameFile )
	{
		//////////////////////////////////////////////////////////////////////////
		/// rendered texture
		cRenderer* r = RENDERSYS->GetRenderer();
		if( r == 0 )
			return false;

		NiTexture::FormatPrefs prefs;
		prefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
		prefs.m_eAlphaFmt = NiTexture::FormatPrefs::BINARY;
		prefs.m_eMipMapped = NiTexture::FormatPrefs::NO;

		mpRenderedTexture = NiRenderedTexture::Create( FrameUV->right, FrameUV->bottom, r, prefs );
		if( mpRenderedTexture == 0 )
			return false;

		mpRenderTargetGroup = NiRenderTargetGroup::Create( mpRenderedTexture->GetBuffer(), r, true, false );

		mpGaugeTarget = new cPlaneObject;
		mpGaugeTarget->Create( mpRenderedTexture, 0, 0, (unsigned short)FrameUV->right, (unsigned short)FrameUV->bottom, 0, 0, (unsigned short)FrameUV->right, (unsigned short)FrameUV->bottom );

		//////////////////////////////////////////////////////////////////////////

		mpFrameTexture = new cPlaneObject;
		mpFrameTexture->Create( pFrameFile, 0, 0, (unsigned short)FrameUV->right, (unsigned short)FrameUV->bottom,
			(unsigned short)FrameUV->left, (unsigned short)FrameUV->top,
			(unsigned short)(FrameUV->left+FrameUV->right), (unsigned short)(FrameUV->top+FrameUV->bottom) );

		mpFrameBackTexture = new cPlaneObject;
		mpFrameBackTexture->Create( pFrameFile, 0, 0, (unsigned short)FrameBack->right, (unsigned short)FrameBack->bottom,
			(unsigned short)FrameBack->left, (unsigned short)FrameBack->top,
			(unsigned short)(FrameBack->left+FrameBack->right), (unsigned short)(FrameBack->top+FrameBack->bottom) );
	}

	mpGaugeTexture = new cPlaneObject;
	if( mpGaugeTexture->Create( pGaugeFile, 1, 1, (unsigned short)GaugeUV->right, (unsigned short)GaugeUV->bottom,
		(unsigned short)GaugeUV->left, (unsigned short)GaugeUV->top, (unsigned short)GaugeUV->left+GaugeTexW, (unsigned short)GaugeUV->top+GaugeTexH ) == false )
	{
		assert(0);
		return false;
	}

	if( mpGaugeTarget )
	{
		if( mpFrameTexture )
			mpFrameTexture->SetRenderTargetWH( mpGaugeTarget->GetScreenW(), mpGaugeTarget->GetScreenH() );

		if( mpFrameBackTexture )
			mpFrameBackTexture->SetRenderTargetWH( mpGaugeTarget->GetScreenW(), mpGaugeTarget->GetScreenH() );

		if( mpGaugeTexture )
			mpGaugeTexture->SetRenderTargetWH( mpGaugeTarget->GetScreenW(), mpGaugeTarget->GetScreenH() );
	}

	return true;
}

void cNameCardGauge::SetGaugePos( short posx, short posy )
{
	if( mpGaugeTarget )
	{
		mpGaugeTarget->SetTranslate( posx, posy );
	}
	else
	{
		if( mpGaugeTexture )
			mpGaugeTexture->SetTranslate( posx, posy );
	}
}

void cNameCardGauge::SetGaugeWidth( unsigned short width )
{
	if( mpGaugeTexture )
		mpGaugeTexture->SetScreenWidth( width );
}

void cNameCardGauge::PreDraw()
{
	if( mpGaugeTarget )
	{
		cRenderer* r = RENDERSYS->GetRenderer();
		if( r == 0 )
			return;

		if( r->BeginUsingRenderTargetGroup( mpRenderTargetGroup, NiRenderer::CLEAR_ALL ) == true )
		{
			r->SetScreenSpaceCameraData();

			if( mpFrameBackTexture )
				mpFrameBackTexture->Draw();
			if( mpGaugeTexture )
				mpGaugeTexture->Draw();
			if( mpFrameTexture )
				mpFrameTexture->Draw();

			r->EndUsingRenderTargetGroup();
		}
	}
}

void cNameCardGauge::Draw()
{
	if( mpGaugeTarget )
	{
		mpGaugeTarget->Draw();
	}
	else
	{
		if( mpGaugeTexture )
			mpGaugeTexture->Draw();
	}
}
