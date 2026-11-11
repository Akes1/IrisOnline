#include "stdafx.h"
#include "Button.h"

BEGIN_MESSAGE_MAP( cButton, CButton )
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

cButton::cButton()
{
}

cButton::~cButton()
{
}

void cButton::DrawItem( LPDRAWITEMSTRUCT ds ) 
{
	assert( ds );

	CDC* dc = CDC::FromHandle( ds->hDC );
	assert( dc != 0 );

	DrawFrame( dc, ds );
	DrawBorder( dc, ds );
	DrawCaption( dc, ds );
}

BOOL cButton::OnEraseBkgnd( CDC* /*dc*/ ) 
{
	return 1;
}

void cButton::DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0 );
	assert( ds != 0 );

	COLORREF color;
	color = ::GetSysColor(COLOR_BTNFACE);
	CBrush brush( color );
	dc->FillRect( &(ds->rcItem), &brush);
}

void cButton::DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds )
{
	assert( dc != 0);
	assert( ds != 0);

	CRect rect( ds->rcItem );

	if( ds->itemState & ODS_SELECTED )
		dc->Draw3dRect (rect, ::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNHIGHLIGHT));
	else
		dc->Draw3dRect (rect, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
}

void cButton::DrawCaption( CDC* dc, LPDRAWITEMSTRUCT ds )
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
