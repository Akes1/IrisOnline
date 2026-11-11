#include "stdafx.h"
#include "TabButton.h"

#include "TabButtonGroup.h"

BEGIN_MESSAGE_MAP( cTabButton, CButton )
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

cTabButton::cTabButton()
: mGroup( 0 )
, mWindow( 0 )
, mChecked( false )
, mIndex( 0 )
{
}

cTabButton::~cTabButton()
{
	if( mGroup )
		mGroup->Remove( mIndex );
}

void cTabButton::DrawItem( LPDRAWITEMSTRUCT ds ) 
{
	assert( ds );

	CDC* dc = CDC::FromHandle( ds->hDC );
	assert( dc != 0 );

	DrawFrame( dc, ds );
	DrawBorder( dc, ds );
	DrawCaption( dc, ds );
}

BOOL cTabButton::OnEraseBkgnd( CDC* /*dc*/ ) 
{
	return 1;
}

void cTabButton::OnLButtonDown( UINT flags, CPoint point ) 
{
	SetChecked( !mChecked );

	CButton::OnLButtonDown( flags, point );
}

void cTabButton::SetGroup( cTabButtonGroup* group )
{
	if( mGroup == group )
		return;

	if( mGroup )
		mGroup->Remove( mIndex );
	
	mGroup = group;
	if( mGroup )
		mGroup->Add( this );
}

void cTabButton::SetChecked( bool checked )
{
	if( mChecked == checked )
		return;

	if( checked )
	{
		mChecked = true;

		if( mWindow )
		{
			mWindow->ShowWindow( SW_SHOW );
			mWindow->SetFocus();
		}
		if( mGroup )
		{
			mGroup->SetCheckedButton( this );
		}
	}
	else
	{
		mChecked = false;

		if( mWindow )
		{
			mWindow->ShowWindow( SW_HIDE );
		}
		if( mGroup )
		{
			mGroup->SetCheckedButton( 0 );
		}
	}

	Invalidate( FALSE );
	UpdateWindow();
}

void cTabButton::DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0 );
	assert( ds != 0 );

	COLORREF color;
	if( mChecked )
		color = RGB(255,255,0);
	else
		color = ::GetSysColor(COLOR_BTNFACE);
	CBrush brush( color );
	dc->FillRect( &(ds->rcItem), &brush);
}

void cTabButton::DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0);
	assert( ds != 0);

	CRect rect( ds->rcItem );

	if( mChecked || ds->itemState & ODS_SELECTED )
		dc->Draw3dRect (rect, ::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNHIGHLIGHT));
	else
		dc->Draw3dRect (rect, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
}

void cTabButton::DrawCaption( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	CRect rect( ds->rcItem );
	CString caption;
	GetWindowText( caption );

	if( ODS_DISABLED & ds->itemState )
	{
		dc->SetBkMode( TRANSPARENT );
		dc->SetBkColor( GetSysColor(COLOR_BTNFACE) );
		rect.OffsetRect( 1, 1 );
		dc->SetTextColor( GetSysColor(COLOR_WINDOW) );
		dc->DrawText( caption, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER );

		rect.OffsetRect( -1, -1 );
		dc->SetTextColor( GetSysColor(COLOR_GRAYTEXT) );
		dc->DrawText( caption, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
	}
	else
	{
		if( ds->itemState & ODS_SELECTED )
			rect.OffsetRect( 1, 1 );

		dc->SetBkMode( TRANSPARENT );
		dc->SetBkColor( GetSysColor(COLOR_BTNFACE) );
		dc->SetTextColor( GetSysColor(COLOR_BTNTEXT) );
		dc->DrawText( caption, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
	}
}
