#include "stdafx.h"
#include "ColorBox.h"

#include "ColorBoxGroup.h"

BEGIN_MESSAGE_MAP( cColorBox, CButton )
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

cColorBox::cColorBox()
: mGroup( 0 )
, mIndex( 0 )
, mChecked( false )
, mColor( RGB(0, 0, 0) )
{
}

cColorBox::~cColorBox()
{
	if( mGroup )
		mGroup->Remove( mIndex );
}

void cColorBox::DrawItem( LPDRAWITEMSTRUCT ds ) 
{
	assert( ds );

	CDC* dc = CDC::FromHandle( ds->hDC );
	assert( dc != 0 );

	DrawFrame( dc, ds );
	DrawBorder( dc, ds );
}

BOOL cColorBox::OnEraseBkgnd( CDC* /*dc*/ ) 
{
	return 1;
}

void cColorBox::OnLButtonDown( UINT flags, CPoint point ) 
{
	SetChecked( true );

	CButton::OnLButtonDown( flags, point );
}

void cColorBox::OnLButtonDblClk( UINT flags, CPoint point )
{
	CColorDialog dlg( mColor, CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mColor = dlg.GetColor();
		Invalidate( FALSE );
		UpdateWindow();
	}

	CButton::OnLButtonDblClk( flags, point );
}

void cColorBox::OnRButtonDown( UINT flags, CPoint point ) 
{
	CColorDialog dlg( mColor, CC_FULLOPEN );

	if( dlg.DoModal() == IDOK )
	{
		mColor = dlg.GetColor();
		Invalidate( FALSE );
		UpdateWindow();
	}

	CButton::OnRButtonDown( flags, point );
}

void cColorBox::SetGroup( cColorBoxGroup* group )
{
	if( mGroup == group )
		return;

	if( mGroup )
		mGroup->Remove( mIndex );

	mGroup = group;
	if( mGroup )
		mGroup->Add( this );
}

void cColorBox::SetChecked( bool checked )
{
	if( mChecked == checked )
		return;

	if( checked )
	{
		mChecked = true;

		if( mGroup )
			mGroup->SetCheckedBox( this );
	}
	else
	{
		mChecked = false;
	}

	Invalidate( FALSE );
	UpdateWindow();
}

void cColorBox::DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0 );
	assert( ds != 0 );

	CBrush brush( mColor );
	dc->FillRect( &(ds->rcItem), &brush);
}

void cColorBox::DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0);
	assert( ds != 0);

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
