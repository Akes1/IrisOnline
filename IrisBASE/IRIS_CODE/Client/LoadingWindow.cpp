#include "stdafx.h"
#include "LoadingWindow.h"
#include "Gauge.h"
#include "UIImage.h"
#include "TipManager.h"
#include "ResourceManager.h"
#include "Label.h"
#include "Application.h"

cLoadingWindow::cLoadingWindow()
: mpLoadGauge(0)
, mpTipImage(0)
, mpLogo(0)
{
}

cLoadingWindow::~cLoadingWindow()
{
	SAFE_DELETE( mpTipImage );
}

void cLoadingWindow::Open()
{
	cUIWindow::Open();

	InitGauge( 100.0f );

	/// 이미지 갱신
	sLoadTipData* data = TIPMAN->GetLoadTipData();
	if( data )
	{
		cString path;
		path.Format("./language/%s/%s", cApplication::mLangaugeFolder.Cstr(), data->mTexName.Cstr() );
		NiTexture* tex = RESOURCEMAN->LoadMapTexture( path, false );
		if( tex )
		{
			mpTipImage = new cUIImage( tex );
			unsigned int tx = data->mTexX;
			unsigned int ty = data->mTexY;
			unsigned int x = (unsigned int)((GetScreenWidth() - TIPIMG_W) * 0.5f);
			unsigned int y = mAbsoluteRect.mTop + 34;

			mpTipImage->SetTextureRect( tx, ty, tx + TIPIMG_W, ty + TIPIMG_H );
			mpTipImage->SetScreenRect( cUIRect( x, y, x + TIPIMG_W, y + TIPIMG_H ) );
		}
	}
}

void cLoadingWindow::Close()
{
	cUIWindow::Close();

	SAFE_DELETE( mpTipImage );
}

bool cLoadingWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpLoadGauge = new cGauge;
	mpLoadGauge->EnableRenderTarget();

	// 게이지
	if( ((cUINode*)mpLoadGauge)->CreateBySkinName( "Loading_GaugeBar", this, 100 ) == false )
	{
		assert(0);
		return false;
	}

	//// 등급
	//mpGrade = new cLabel;
	//if( ((cUINode*)mpGrade)->CreateBySkinName( "Loding_Grade", this, 102 ) == false )
	//{
	//	assert(0);
	//	return false;
	//}

	//// copyright
	//mpCopyRight = new cLabel;
	//if( ((cUINode*)mpCopyRight)->CreateBySkinName( "Loding_COPYRIGHT", this, 103 ) == false )
	//{
	//	assert(0);
	//	return false;
	//}

	//// 주소
	//mpAddress = new cLabel;
	//if( ((cUINode*)mpAddress)->CreateBySkinName( "Loding_Address", this, 104 ) == false )
	//{
	//	assert(0);
	//	return false;
	//}

	// 로고
	mpLogo = new cLabel;
	if( mpLogo->CreateBySkinName( "Loding_Logo", this, 101 ) == false )
	{
		assert(0);
		return false;
	}

	/// 사이즈 변경
	SetRelativeSize( cUISize(GetScreenWidth(), mAbsoluteRect.GetHeight() ) );

	/// 게이지 위
	if( mpLoadGauge )
	{
		int x = (int)(( GetScreenWidth() - mpLoadGauge->GetAbsoluteRect().GetWidth() ) * 0.5f);
		int y = (int)( mpLoadGauge->GetRelativeRect().mTop );
		mpLoadGauge->SetRelativePos( cUIPos( x, y) );
	}

	if( mpLogo )
	{
		int x = (int)(( GetScreenWidth() - mpLogo->GetAbsoluteRect().GetWidth() ) * 0.5f);
		int y = (int)( mpLogo->GetRelativeRect().mTop );
		mpLogo->SetRelativePos( cUIPos( x, y) );
	}

	//if( mpGrade )
	//{
	//	// 우측상단
	//	cUINodeSkin* nodeSkin = mpGrade->GetNodeSkin();
	//	if( nodeSkin )
	//	{
	//		int x = (int)( GetScreenWidth() - nodeSkin->mSkinInfo->mWidth - nodeSkin->mSkinInfo->mX );
	//		int y = (int)( mpLogo->GetRelativeRect().mTop );
	//		mpGrade->SetRelativePos( cUIPos( x, y) );
	//	}
	//}
	//
	//if( mpCopyRight )
	//{
	//	// 하단 중앙정렬
	//	cUINodeSkin* nodeSkin = mpCopyRight->GetNodeSkin();
	//	if( nodeSkin )
	//	{
	//		int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
	//		int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
	//		mpCopyRight->SetRelativePos( cUIPos( x, y) );
	//	}
	//}

	//if( mpAddress )
	//{
	//	// 하단 중앙정렬
	//	cUINodeSkin* nodeSkin = mpAddress->GetNodeSkin();
	//	if( nodeSkin )
	//	{
	//		int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
	//		int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
	//		mpAddress->SetRelativePos( cUIPos( x, y) );
	//	}

	//	//
	//	mpAddress->SetTextColor( 0xFFD7C832 );
	//}
	return true;
}

void cLoadingWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 사이즈 변경
	SetRelativeSize( cUISize(GetScreenWidth(), mAbsoluteRect.GetHeight() ) );

	/// 게이지 위
	if( mpLoadGauge )
	{
		int x = (int)(( GetScreenWidth() - mpLoadGauge->GetAbsoluteRect().GetWidth() ) * 0.5f);
		int y = (int)( mpLoadGauge->GetRelativeRect().mTop );
		mpLoadGauge->SetRelativePos( cUIPos( x, y) );
	}

	if( mpLogo )
	{
		int x = (int)(( GetScreenWidth() - mpLogo->GetAbsoluteRect().GetWidth() ) * 0.5f);
		int y = (int)( mpLogo->GetRelativeRect().mTop );
		mpLogo->SetRelativePos( cUIPos( x, y) );
	}

	//if( mpGrade )
	//{
	//	// 우측상단
	//	cUINodeSkin* nodeSkin = mpGrade->GetNodeSkin();
	//	if( nodeSkin )
	//	{
	//		int x = (int)( GetScreenWidth() - nodeSkin->mSkinInfo->mWidth - nodeSkin->mSkinInfo->mX );
	//		int y = (int)( mpLogo->GetRelativeRect().mTop );
	//		mpGrade->SetRelativePos( cUIPos( x, y) );
	//	}
	//}

	//if( mpCopyRight )
	//{
	//	// 하단 중앙정렬
	//	cUINodeSkin* nodeSkin = mpCopyRight->GetNodeSkin();
	//	if( nodeSkin )
	//	{
	//		int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
	//		int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
	//		mpCopyRight->SetRelativePos( cUIPos( x, y) );
	//	}
	//}

	//if( mpAddress )
	//{
	//	// 하단 중앙정렬
	//	cUINodeSkin* nodeSkin = mpAddress->GetNodeSkin();
	//	if( nodeSkin )
	//	{
	//		int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
	//		int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
	//		mpAddress->SetRelativePos( cUIPos( x, y) );
	//	}

	//	//
	//	mpAddress->SetTextColor( 0xFFD7C832 );
	//}
}

void cLoadingWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mpTipImage )
		mpTipImage->Draw();

	cUIWindow::OnRender( pKeeper );
}

void cLoadingWindow::InitGauge( float maxValue )
{
	if( mpLoadGauge )
	{
		mpLoadGauge->ClearFront();
		mpLoadGauge->SetMaxValue( maxValue );
	}
}

double cLoadingWindow::GetCurrentValue()
{
	if( mpLoadGauge )
		return mpLoadGauge->GetValue();

	return 0.0f;
}

///
void cLoadingWindow::IncreaseGaugeValue( float destValue )
{
	if( mpLoadGauge )
	{
		mpLoadGauge->SetGaugeAni( destValue );
	}
}

bool cLoadingWindow::IsGaugeEnd()
{
	if( mpLoadGauge == 0 )
		return true;
	if( mpLoadGauge->GetValue() >= mpLoadGauge->GetMaxValue() )
		return true;

	return false;
}
