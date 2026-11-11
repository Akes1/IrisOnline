#include "StdAfx.h"
#include "SkinComboBox.h"
#include "SkinResource.h"
#include "skinscrollwnd.h" 

IMPLEMENT_DYNCREATE( cSkinComboBox, CComboBox )

BEGIN_MESSAGE_MAP(cSkinComboBox, CComboBox) 
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCbnCloseup) 
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnCbnDropdown)

	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP() 

cSkinComboBox::cSkinComboBox()
{
	mhWndList = NULL;
	mhBmpScroll = NULL;

}

cSkinComboBox::~cSkinComboBox()
{
}

BOOL cSkinComboBox::Create( cSkinComponentResource* resource, CWnd* parent, UINT id )
{
	ASSERT( resource );
	mResource = (cSkinComboBoxResource*)resource;
	mhBmpScroll = mResource->mScrollBitmap;

	mRect = mResource->mRect;

	///
	mRectLeft = mResource->mRectLeft;
	mRectCenter = mResource->mRectCenter;
	mRectRight = mResource->mRectRight;

	DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_OWNERDRAWVARIABLE;

	return CComboBox::Create( style, mRect, parent, id );
}

void cSkinComboBox::OnCbnCloseup() 
{ 
	if(!mhWndList) 
		return; 

	::ShowWindow(::GetParent(::GetParent(mhWndList)),SW_HIDE); 
} 

void cSkinComboBox::OnCbnDropdown() 
{ 
	if(!mhWndList) 
		return;

	CWnd *pFrame = CWnd::FromHandle(::GetParent(::GetParent(mhWndList)));

	CRect rc;
	GetWindowRect(&rc);

	CRect rc2;
	GetDroppedControlRect(&rc2);

	int nHei;
	int nLineHei=GetItemHeight(-1);
	if( nLineHei*GetCount() < rc2.Height()-2 )
		nHei=nLineHei*GetCount();
	else
		nHei=rc2.Height()-2;

	pFrame->SetWindowPos( &wndTopMost, rc.left, rc.bottom, rc2.Width(), nHei, 0 );
	pFrame->ShowWindow(SW_SHOW);
}

void cSkinComboBox::InitShow()
{
	ShowDropDown( TRUE );
	ShowDropDown( FALSE );
}

HBRUSH cSkinComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{ 
	HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
	if( (nCtlColor == CTLCOLOR_LISTBOX) && !mhWndList && mhBmpScroll )
	{
		pWnd->ShowWindow(SW_HIDE);
		SkinWndScroll( pWnd, mhBmpScroll );
		mhWndList=pWnd->m_hWnd;
	}

	return hbr;
} 

void cSkinComboBox::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	RECT rc;
	CDC MemDC;

	GetClientRect(&rc);
	MemDC.CreateCompatibleDC(&dc);

	CBitmap *oldBmp = MemDC.SelectObject( &mResource->mBitmap );

	dc.BitBlt( rc.left,rc.top,mRectLeft.right,mRectLeft.bottom, &MemDC, 
		mRectLeft.left, mRectLeft.top, SRCCOPY );
	dc.BitBlt( rc.right-mRectRight.right, rc.top, mRectRight.right, mRectRight.bottom, &MemDC, 
		mRectRight.left, mRectRight.top, SRCCOPY );
	dc.StretchBlt( rc.left+mRectLeft.right, rc.top, rc.right-mRectLeft.right-mRectRight.right, mRectCenter.bottom, &MemDC, 
		mRectCenter.left, mRectCenter.top, mRectCenter.right, mRectCenter.bottom, SRCCOPY );

	MemDC.SelectObject( oldBmp );

	int curSel = GetCurSel();
	if( curSel > -1 )
	{
		CString sz;
		GetLBText( curSel, sz );
		rc.left += 3;//nListBoxTextLeft;

		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor( RGB(255,255,255) );
		dc.SelectObject( GetFont() );
		dc.DrawText( sz, &rc, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS  );
	}
}

void cSkinComboBox::DrawItem( LPDRAWITEMSTRUCT ds )
{
	// TODO: Add your code to draw the specified item
	ASSERT(ds->CtlType == ODT_COMBOBOX);
	if( ds->itemID == -1 )
		return;

	CDC			dc;	
	dc.Attach(ds->hDC);

	dc.SetTextColor( RGB(255, 255, 255) );

	if( (ds->itemAction | ODA_SELECT) && (ds->itemState & ODS_SELECTED) )
	{
		if( ds->itemState & ODS_COMBOBOXEDIT )
			dc.FillSolidRect(&ds->rcItem, RGB(41, 41, 43)); 
		else
			dc.FillSolidRect(&ds->rcItem, RGB(71, 71, 73));
	}
	else
		dc.FillSolidRect(&ds->rcItem, RGB(41, 41, 43)); 

	// Copy the text of the item to a string
	CString		sItem;
	GetLBText(ds->itemID, sItem);
	dc.SetBkMode(TRANSPARENT);

	// Draw the text after the images left postion
	dc.SelectObject(GetFont());
	dc.DrawText( sItem, &ds->rcItem, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS );

	dc.Detach();
}