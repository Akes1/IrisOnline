#include "stdafx.h"
#include "CheckButton.h"

#include "CheckButtonGroup.h"

BEGIN_MESSAGE_MAP( cCheckButton, CButton )
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

cCheckButton::cCheckButton()
: mGroup( 0 )
, mChecked( false )
, mIndex( 0 )
{
}

cCheckButton::~cCheckButton()
{
	if( mGroup )
		mGroup->Remove( mIndex );
}

void cCheckButton::DrawItem( LPDRAWITEMSTRUCT ds ) 
{
	assert( ds );

	CDC* dc = CDC::FromHandle( ds->hDC );
	assert( dc != 0 );

	DrawFrame( dc, ds );
	DrawBorder( dc, ds );
	DrawCaption( dc, ds );
}

BOOL cCheckButton::OnEraseBkgnd( CDC* /*dc*/ ) 
{
	return 1;
}

void cCheckButton::OnLButtonUp( UINT flags, CPoint point ) 
{
	SetChecked( !mChecked );

	CButton::OnLButtonUp( flags, point );
}

void cCheckButton::SetGroup( cCheckButtonGroup* group )
{
	if( mGroup == group )
		return;

	if( mGroup )
		mGroup->Remove( mIndex );
	
	mGroup = group;
	if( mGroup )
		mGroup->Add( this );
}

void cCheckButton::SetChecked( bool checked )
{
	if( mChecked == checked )
		return;

	if( checked )
	{
		mChecked = true;

		if( mGroup )
		{
			mGroup->SetCheckedButton( this );
		}
	}
	else
	{
		mChecked = false;

		if( mGroup )
		{
			mGroup->SetCheckedButton( 0 );
		}
	}
	Invalidate( FALSE );
}

void cCheckButton::DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds )
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

void cCheckButton::DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0);
	assert( ds != 0);

	CRect rect( ds->rcItem );

	if( mChecked || ds->itemState & ODS_SELECTED )
		dc->Draw3dRect (rect, ::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNHIGHLIGHT));
	else
		dc->Draw3dRect (rect, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
}

void cCheckButton::DrawCaption( CDC* dc, LPDRAWITEMSTRUCT ds )
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
