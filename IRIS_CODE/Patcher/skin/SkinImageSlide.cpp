#include "stdafx.h"
#include "SkinImageSlide.h"
#include "SkinResource.h"

BEGIN_MESSAGE_MAP( cSkinImageSlide, CStatic )
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

cSkinImageSlide::cSkinImageSlide()
: mResource( 0 )
, mWidth( 0 )
, mHeight( 0 )
, mNumImages( 0 )
, mImageIndex( 0 )
{
}

cSkinImageSlide::~cSkinImageSlide()
{
}

BOOL cSkinImageSlide::Create( cSkinComponentResource* resource, CWnd* parent, UINT id )
{
	/// 리소스 설정
	ASSERT( resource );
	mResource = (cSkinImageSlideResource*)resource;

	mWidth = mResource->mWidth;
	mHeight = mResource->mHeight;
	mNumImages = mResource->mBitmapArray.GetSize();

	/// 스타일 지정
	DWORD style = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
	return CStatic::Create( 0, style, mResource->mRect, parent, id );
}

BOOL cSkinImageSlide::OnEraseBkgnd( CDC* dc ) 
{
	return 1;
}

void cSkinImageSlide::OnPaint() 
{
	CPaintDC dc( this );

	/// 비트맵 전송
	CDC memdc;
	memdc.CreateCompatibleDC( &dc );

	CBitmap *poldBmp = memdc.SelectObject( &mResource->mBitmapArray[mImageIndex] );
	dc.TransparentBlt( 0, 0, mWidth, mHeight, &memdc, 0, 0, mWidth, mHeight, RGB(255,0,0) );
	memdc.SelectObject( poldBmp );
}
