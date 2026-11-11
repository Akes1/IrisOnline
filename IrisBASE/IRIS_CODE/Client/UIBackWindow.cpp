#include "StdAfx.h"
#include "UIBackWindow.h"
#include "planeobject.h"
#include "Label.h"

cUIBackWindow::cUIBackWindow( eUINodeType type )
: cUIWindow( type )
, mpPlane(0)
{
}

cUIBackWindow::~cUIBackWindow()
{
	SAFE_DELETE(mpPlane);
}

void cUIBackWindow::Open()
{
	cUIWindow::Open();

	SetEnabled( false );
	SetExceptEvent( true );
}

void cUIBackWindow::Close()
{
	SetEnabled( false );
}

/// 주의사항 : 
bool cUIBackWindow::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_WINDOW ) == false )
	{
		assert( 0 && "not window type" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( pskin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( pskin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned short tx = (unsigned short)pskin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)pskin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)pskin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)pskin->mSkinInfo->mTexHeight;

		if( mpPlane == 0 )
		{
			mpPlane = new cPlaneObject;

			if( mpPlane->Create( pskin->mpTexture, 0, 0, 
								(unsigned short)GetScreenWidth(), (unsigned short)GetScreenHeight(),
								tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create back image");
				return false;
			}
		}
		else
		{
			mpPlane->SetTextureRect( tx, ty, tx + tw, ty + th );
		}
	}
	else
	{
		SAFE_DELETE( mpPlane );
	}

	/// 스크린상 위치 좌표 세팅 
	unsigned int x = pskin->mSkinInfo->mX;
	unsigned int y = pskin->mSkinInfo->mY;
	unsigned int w = pskin->mSkinInfo->mWidth;
	unsigned int h = pskin->mSkinInfo->mHeight;

	SetRelativeRect( cUIRect(x, y, x + w, y + h) );
	return true;
}

//void cUIBackWindow::SetAlphaEnabled( bool enabled, NiAlphaProperty::AlphaFunction srcBlend, NiAlphaProperty::AlphaFunction destBlend )
//{
//	/// 알파 옵션 조절
//	NiAlphaProperty* alphaProp = mpPlane->GetAlphaProp();
//	alphaProp->SetAlphaBlending( enabled );
//
//	if( enabled )
//	{
//		alphaProp->SetSrcBlendMode( srcBlend );
//		alphaProp->SetDestBlendMode( destBlend );
//	}
//}

/// 주의사항 : 
void cUIBackWindow::UpdateSkin()
{
}

/// 주의사항 : 
void cUIBackWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{
	/// 배경 이미지 렌더링
	if( mpPlane )
		mpPlane->Draw();

	cUIWindow::OnRender( fontKeeper );
}

/// 주의사항 : 
void cUIBackWindow::OnMouseMove( const cUIPos& )
{
}

/// 주의사항 : 
void cUIBackWindow::OnLButtonDown( const cUIPos&, bool , bool , bool )
{
}

void cUIBackWindow::SetBackTexture( NiTexture* tex )
{
	if( tex == 0 )
	{
		SAFE_DELETE(mpPlane);
		return;
	}

	mpPlane = new cPlaneObject;
	if( mpPlane->Create( tex, 0, 0, (unsigned short)GetScreenWidth(), (unsigned short)GetScreenHeight(),
			0, 0, tex->GetWidth(), tex->GetHeight() ) == false )
	{
		assert( 0 && "failed to create back image");
		NiMessageBox("failed to create back image","Error");
		return;
	}
}

void cUIBackWindow::SetAlpha( float alpha )
{
	if( mpPlane )
		mpPlane->SetColorA( 1.0f, 1.0f, 1.0f, alpha );
}

//////////////////////////////////////////////////////////////////
cLoadBackWindow::cLoadBackWindow( eUINodeType type )
: cUIBackWindow( type )
//, mpGrade(0)
, mpCopyRight(0)
, mpAddress(0)
{
}

cLoadBackWindow::~cLoadBackWindow()
{
}

void cLoadBackWindow::Open()
{
	cUIWindow::Open();

	SetEnabled( false );
}

void cLoadBackWindow::Close()
{
	cUIWindow::Close();

	SetEnabled( false );
	SetBackTexture( 0 );
}

bool cLoadBackWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIBackWindow::OnCreate( pproperty ) == false )
		return false;

	// 등급
//	mpGrade = new cLabel;
//	if( ((cUINode*)mpGrade)->CreateBySkinName( "Loding_Grade", this, 102 ) == false )
//	{
//		assert(0);
//		return false;
//	}

	// copyright
	mpCopyRight = new cLabel;
	if( ((cUINode*)mpCopyRight)->CreateBySkinName( "Loding_COPYRIGHT", this, 103 ) == false )
	{
		assert(0);
		return false;
	}

	// 주소
	mpAddress = new cLabel;
	if( ((cUINode*)mpAddress)->CreateBySkinName( "Loding_Address", this, 104 ) == false )
	{
		assert(0);
		return false;
	}

//	if( mpGrade )
//	{
		// 우측상단
//		cUINodeSkin* nodeSkin = mpGrade->GetNodeSkin();
//		if( nodeSkin )
//		{
//			int x = (int)( GetScreenWidth() - nodeSkin->mSkinInfo->mWidth - nodeSkin->mSkinInfo->mX );
//			int y = (int)( mpGrade->GetRelativeRect().mTop );
//			mpGrade->SetRelativePos( cUIPos( x, y) );
//		}
//	}

	if( mpCopyRight )
	{
		// 하단 중앙정렬
		cUINodeSkin* nodeSkin = mpCopyRight->GetNodeSkin();
		if( nodeSkin )
		{
			int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
			int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
			mpCopyRight->SetRelativePos( cUIPos( x, y) );
		}
	}

	if( mpAddress )
	{
		// 하단 중앙정렬
		cUINodeSkin* nodeSkin = mpAddress->GetNodeSkin();
		if( nodeSkin )
		{
			int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
			int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
			mpAddress->SetRelativePos( cUIPos( x, y) );
		}


		mpAddress->SetTextColor( 0xFFD7C832 );
	}
	return true;
}

void cLoadBackWindow::UpdateSkin()
{
	cUIBackWindow::UpdateSkin();
	
//	if( mpGrade )
//	{
		// 우측상단
//		cUINodeSkin* nodeSkin = mpGrade->GetNodeSkin();
//		if( nodeSkin )
//		{
//			int x = (int)( GetScreenWidth() - nodeSkin->mSkinInfo->mWidth - nodeSkin->mSkinInfo->mX );
//			int y = (int)( mpGrade->GetRelativeRect().mTop );
//			mpGrade->SetRelativePos( cUIPos( x, y) );
//		}
//	}

	if( mpCopyRight )
	{
		// 하단 중앙정렬
		cUINodeSkin* nodeSkin = mpCopyRight->GetNodeSkin();
		if( nodeSkin )
		{
			int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
			int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
			mpCopyRight->SetRelativePos( cUIPos( x, y) );
		}
	}

	if( mpAddress )
	{
		// 하단 중앙정렬
		cUINodeSkin* nodeSkin = mpAddress->GetNodeSkin();
		if( nodeSkin )
		{
			int x = (int)(( GetScreenWidth() - mpCopyRight->GetAbsoluteRect().GetWidth() ) * 0.5f);
			int y = (int)( GetScreenHeight() - nodeSkin->mSkinInfo->mHeight - nodeSkin->mSkinInfo->mY );
			mpAddress->SetRelativePos( cUIPos( x, y) );
		}

		mpAddress->SetTextColor( 0xFFD7C832 );
	}
}


//////////////////////////////////////////////////////////////////
cNpcBackWindow::cNpcBackWindow( eUINodeType type )
: cUIBackWindow( type )
{
}

cNpcBackWindow::~cNpcBackWindow()
{
}

void cNpcBackWindow::Open()
{
	SetEnabled( false );
}

void cNpcBackWindow::Close()
{
	cUIWindow::Close();

	SetEnabled( false );
}
