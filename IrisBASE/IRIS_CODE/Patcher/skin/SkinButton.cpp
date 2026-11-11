#include "stdafx.h"
#include "SkinButton.h"
#include "SkinResource.h"

#pragma warning( disable: 4100 )

BEGIN_MESSAGE_MAP( cSkinButton, CButton )
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

cSkinButton::cSkinButton()
: mResource( 0 )
, mWidth( 0 )
, mHeight( 0 )
, mStyle( 0 )
, mChecked( false )
{
	mRect.SetRectEmpty();
}

cSkinButton::~cSkinButton()
{
}

BOOL cSkinButton::Create( cSkinComponentResource* resource, CWnd* parent, UINT id )
{
	/// 리소스 설정
	ASSERT( resource );
	mResource = (cSkinButtonResource*)resource;

	mRect = mResource->mRect;
	mWidth = mResource->mWidth;
	mHeight = mResource->mHeight;
	mStyle = mResource->mStyle;

	/// 스타일 지정
	DWORD style = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;

	return CButton::Create( 0, style, mRect, parent, id );
}

void cSkinButton::DrawItem( LPDRAWITEMSTRUCT ds ) 
{
	ASSERT( ds );

	/// 버튼 상태에 따라 가져올 이미지 선택
	UINT itemState = ds->itemState;
	int x = 0;

	if( mStyle == 0)
	{
		/// 푸시 버튼
		if( itemState & ODS_SELECTED )
		{
			x = mWidth + mWidth;
		}
		else if( itemState & ODS_DISABLED )
		{
			x = 0;
		}
		else
		{
			if( GetCapture() != this )
				x = 0;
			else
				x = mWidth;
		}
	}
	else
	{
		/// 체크 박스 버튼
		if( mChecked )
		{
			x = mWidth;
		}
//		else if( itemState & ODS_DISABLED )
//		{
//			x = mWidth;
//		}
//		else
//		{
//			if( GetCapture() != this )
//				x = 0;
//			else
//				x = mWidth;
//		}
	}

	/// 비트맵 전송
	CDC *dc = CDC::FromHandle( ds->hDC );
	CDC memdc;
	memdc.CreateCompatibleDC( dc );

	CBitmap *oldBmp = memdc.SelectObject( &mResource->mBitmap );
	dc->TransparentBlt( 0, 0, mWidth, mHeight, &memdc, x, 0, mWidth, mHeight, RGB(255,0,0) );
	memdc.SelectObject( oldBmp );
//	ReleaseDC( dc );
}

void cSkinButton::OnDestroy() 
{
	CButton::OnDestroy();
}

BOOL cSkinButton::OnEraseBkgnd( CDC* dc ) 
{
	return 1;
}

void cSkinButton::OnMouseMove( UINT flags, CPoint point ) 
{
	if( GetCapture() != this )
	{
		SetCapture();
		Invalidate( 0 );
	}
	else
	{
		CRect rect;
		GetClientRect( &rect );

		if( rect.PtInRect( point ) == 0 )
		{
			ReleaseCapture();
			Invalidate( 0 );
		}
	}

	CButton::OnMouseMove( flags, point );
}

void cSkinButton::OnLButtonDown( UINT flags, CPoint point ) 
{
	if( mStyle )
	{
		mChecked = !mChecked;
	}

	CButton::OnLButtonDown( flags, point );
}

void cSkinButton::OnLButtonUp( UINT flags, CPoint point ) 
{

	CButton::OnLButtonUp( flags, point );
}
