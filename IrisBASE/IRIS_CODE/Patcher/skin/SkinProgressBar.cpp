#include "stdafx.h"
#include "SkinProgressBar.h"
#include "SkinResource.h"

BEGIN_MESSAGE_MAP(cSkinProgressBar, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

cSkinProgressBar::cSkinProgressBar()
: mResource( 0 )
, mWidth( 0 )
, mHeight( 0 )
, mColor( 0 )
, mBackColor( 0 )
, mValue( 0.0f )
{
}

cSkinProgressBar::~cSkinProgressBar()
{
}

BOOL cSkinProgressBar::Create( cSkinComponentResource* resource, CWnd* parent, UINT id )
{
	/// 리소스 설정
	ASSERT( resource );
	mResource = (cSkinProgressBarResource*)resource;

	mWidth = mResource->mWidth;
	mHeight = mResource->mHeight;
	mColor = mResource->mColor;
	mBackColor = mResource->mBackColor;

	DWORD style = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
	return CStatic::Create( 0, style, mResource->mRect, parent, id );
}

void cSkinProgressBar::OnPaint() 
{
	CPaintDC dc( this );

	CRect rc;
	GetClientRect( &rc );

	CBrush brush, *poldBrush;
	CBrush back;
	CPen pen, *poldPen;
	CPen pen2;

	brush.CreatePatternBrush( &mResource->mBitmap );
	poldBrush = dc.SelectObject( &brush );
	
	pen.CreatePen( PS_SOLID, 0, mColor );
	poldPen = dc.SelectObject( &pen );
	dc.Rectangle( rc.left, rc.top, rc.left + mWidth * mValue, rc.bottom );

	pen2.CreatePen( PS_SOLID, 0, mBackColor );
	dc.SelectObject( &pen2 );
	
	back.CreateSolidBrush( mBackColor );
	dc.SelectObject( &back );
	dc.Rectangle( rc.left + mWidth * mValue, rc.top, rc.right, rc.bottom );


	dc.SelectObject( poldBrush );
	dc.SelectObject( poldPen );

	// Do not call CStatic::OnPaint() for painting messages
}
