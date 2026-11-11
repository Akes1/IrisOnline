#include "stdafx.h"
#include "ImageButton.h"

#include "ImageButtonGroup.h"

BEGIN_MESSAGE_MAP( cImageButton, CButton )
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

cImageButton::cImageButton()
: mGroup( 0 )
, mIndex( 0 )
, mCheckEnabled( true )
, mChecked( false )
, mBitmap( 0 )
, mBitmapWidth( 0 )
, mBitmapHeight( 0 )
{
}

cImageButton::~cImageButton()
{
	if( mGroup )
		mGroup->Remove( mIndex );
}

void cImageButton::SetImage( CBitmap* bitmap, unsigned int width, unsigned int height )
{
	mBitmap = bitmap;
	mBitmapWidth = (int)width;
	mBitmapHeight = (int)height;

	if( mBitmap == 0 && mChecked == true )
	{
		SetChecked( false );
	}

	Invalidate( FALSE );
	UpdateWindow();
}

void cImageButton::DrawItem( LPDRAWITEMSTRUCT ds ) 
{
	assert( ds );

	CDC* dc = CDC::FromHandle( ds->hDC );
	assert( dc != 0 );

	if( mBitmap )
		DrawImage( dc, ds );
	else
		DrawFrame( dc, ds );

	DrawBorder( dc, ds );
}

BOOL cImageButton::OnEraseBkgnd( CDC* /*dc*/ ) 
{
	return 1;
}

void cImageButton::OnLButtonDown( UINT flags, CPoint point ) 
{
	if( mBitmap )
		SetChecked( !mChecked );

	CButton::OnLButtonUp( flags, point );
}

void cImageButton::SetGroup( cImageButtonGroup* group )
{
	if( mGroup == group )
		return;

	if( mGroup )
		mGroup->Remove( mIndex );
	
	mGroup = group;
	if( mGroup )
		mGroup->Add( this );
}

void cImageButton::SetChecked( bool checked )
{
	if( mCheckEnabled == false )
		return;

	if( mChecked == checked )
		return;

	if( checked )
	{
		mChecked = true;

		if( mGroup )
			mGroup->SetCheckedButton( this );
	}
	else
	{
		mChecked = false;

		if( mGroup )
			mGroup->SetCheckedButton( 0 );
	}

	Invalidate( FALSE );
	UpdateWindow();
}

void cImageButton::DrawImage( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0 );
	assert( ds != 0 );

	RECT& rc = ds->rcItem;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	/// ºñÆ®¸Ê Àü¼Û
	CDC memdc;
	memdc.CreateCompatibleDC( dc );

	CBitmap *oldBmp = memdc.SelectObject( mBitmap );

	if( w > mBitmapWidth && h > mBitmapHeight )
	{
		int x = (w - mBitmapWidth) / 2;
		int y = (h - mBitmapHeight) / 2;
		dc->BitBlt( x, y, mBitmapWidth, mBitmapHeight, &memdc, 0, 0, SRCCOPY );
	}
	else
	{
		int x = (mBitmapWidth - w) / 2;
		int y = (mBitmapHeight - h) / 2;
		dc->BitBlt( 0, 0, w, h, &memdc, x, y, SRCCOPY );
	}
	memdc.SelectObject( oldBmp );
}

void cImageButton::DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0 );
	assert( ds != 0 );

	CBrush brush( ::GetSysColor(COLOR_BTNFACE) );
	dc->FillRect( &(ds->rcItem), &brush);
}

void cImageButton::DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0 );
	assert( ds != 0 );

	CRect rect( ds->rcItem );

	if( mChecked )
	{
		dc->Draw3dRect( rect, RGB(0,0,0), RGB(0,0,0) );
		rect.DeflateRect( 1, 1, 1, 1 );
		dc->Draw3dRect( rect, RGB(255,255,0), RGB(255,255,0) );
		rect.DeflateRect( 1, 1, 1, 1 );
		dc->Draw3dRect( rect, RGB(0,0,0), RGB(0,0,0) );
	}
	else
		dc->Draw3dRect( rect, ::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNSHADOW) );
}
