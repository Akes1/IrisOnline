#include "stdafx.h"
#include "IntroWindow.h"
#include "FontAgent.h"
#include "PlaneObject.h"

cIntroWindow::cIntroWindow( eUINodeType type )
: cUIBackWindow( type )
{
	mPlaneGrade = 0;
}

cIntroWindow::~cIntroWindow()
{
	SAFE_DELETE(mPlaneGrade);
}

bool cIntroWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIBackWindow::OnCreate( pproperty ) == false )
		return false;
	return true;
}

void cIntroWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mpPlane )
		mpPlane->Draw();

	if( mPlaneGrade )
		mPlaneGrade->Draw();

	cUIWindow::OnRender( pKeeper );
}

bool cIntroWindow::SetSkin( const cUINodeSkin* pskin )
{
	if( cUIBackWindow::SetSkin( pskin ) == false )
		return false;

	if( pskin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned short tx = (unsigned short)pskin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)pskin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)pskin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)pskin->mSkinInfo->mTexHeight;

		if( mPlaneGrade == 0 )
		{
			mPlaneGrade = new cPlaneObject;

			if( mPlaneGrade->Create( pskin->mpTexture, 0, 0, 
				(unsigned short)GetScreenWidth(), (unsigned short)GetScreenHeight(),
				tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create grade image");
				return false;
			}
		}
		else
		{
			mPlaneGrade->SetTextureRect( tx, ty, tx + tw, ty + th );
		}
	}
	else
	{
		SAFE_DELETE( mPlaneGrade );
	}

	return true;
}

void cIntroWindow::SetAlphaGrade( float alpha )
{
	if( mPlaneGrade )
		mPlaneGrade->SetColorA( 1.0f, 1.0f, 1.0f, alpha );
}

void cIntroWindow::SetGradeTexture( NiTexture* tex )
{
	if( tex == 0 )
	{
		SAFE_DELETE(mPlaneGrade);
		return;
	}

	mPlaneGrade = new cPlaneObject;
	if( mPlaneGrade->Create( tex, 0, 0, (unsigned short)GetScreenWidth(), (unsigned short)GetScreenHeight(),
		0, 0, tex->GetWidth(), tex->GetHeight() ) == false )
	{
		assert( 0 && "failed to create Grade image");
		NiMessageBox("failed to create Grade image","Error");
		return;
	}
}
