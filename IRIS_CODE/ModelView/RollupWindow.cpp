//////////////////////////////////////////////////////////////////////////////
//
// RollupCtrl.cpp
// 
//
// Code Johann Nadalutti
// Mail: jnadalutti@hotmail.com
//
//////////////////////////////////////////////////////////////////////////////
//
// This code is free for personal and commercial use, providing this 
// notice remains intact in the source files and all eventual changes are
// clearly marked with comments.
//
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
//////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RollupWindow.h"

#pragma warning( disable : 4312 4311 ) 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl Message Map

BEGIN_MESSAGE_MAP(cRollupWindow, CWnd)
	//{{AFX_MSG_MAP(CRollupCtrl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl Implementation

IMPLEMENT_DYNCREATE(cRollupWindow, CWnd)

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
cRollupWindow::cRollupWindow()
{
	m_strMyClass = AfxRegisterWndClass(
		CS_VREDRAW | CS_HREDRAW,
		(HCURSOR)::LoadCursor(0, IDC_ARROW),
		(HBRUSH)(COLOR_BTNSHADOW),
		0);

	m_StartYPos = m_PageHeight = 0;

	m_nColumnWidth=200;
	m_bEnabledAutoColumns=FALSE;
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
cRollupWindow::~cRollupWindow()
{
	//Remove all pages allocations
	for (int i=0; i<m_PageList.GetSize(); i++)
	{
		RC_PAGEINFO* pi = m_PageList[i];
	
		if( pi->pwndButton)		delete pi->pwndButton;
		if( pi->pwndGroupBox)	delete pi->pwndGroupBox;

		if( pi->pwndTemplate)
		{
			if( pi->bAutoDestroyTpl) {
				pi->pwndTemplate->DestroyWindow();
				delete pi->pwndTemplate;
			} else {
				//pi->pwndTemplate->ShowWindow(SW_HIDE);
				::SetWindowLong(pi->pwndTemplate->m_hWnd, DWL_DLGPROC, (LONG)pi->pOldDlgProc);
			}
		}

		delete pi;
	}

}

//---------------------------------------------------------------------------
// Create
//---------------------------------------------------------------------------
BOOL cRollupWindow::Create(DWORD style, const RECT& rect, CWnd* pParentWnd, UINT id)
{
	BOOL bRet = CWnd::Create(m_strMyClass, _T("RollupCtrl"), style, rect, pParentWnd, id);

	return bRet;
}


//---------------------------------------------------------------------------
// Function name	: InsertPage
// Description	 : return -1 if an error occurs
//---------------------------------------------------------------------------
int	cRollupWindow::InsertPage(LPCTSTR caption, UINT nIDTemplate, int idx)
{
	if( idx>0 && idx>=m_PageList.GetSize())		idx=-1;

	//Create Template
	CDialog* wndtemplate = new CDialog(nIDTemplate, this);
	BOOL b = wndtemplate->Create(nIDTemplate, this);
	if( !b)	{ delete wndtemplate; return -1; }

	//Insert Page
	return _InsertPage(caption, wndtemplate, idx, TRUE);
}

//---------------------------------------------------------------------------
// Function name	: InsertPage
// Description	 : return -1 if an error occurs
//---------------------------------------------------------------------------
int cRollupWindow::InsertPage(LPCTSTR caption, UINT nIDTemplate, CRuntimeClass* rtc, int idx)
{
	if( idx>0 && idx>=m_PageList.GetSize())		idx=-1;

	//Create Template
	assert(rtc!=0);
	CDialog* wndtemplate = (CDialog*)rtc->CreateObject();
	BOOL b = wndtemplate->Create(nIDTemplate, this);
	if( !b)	{ delete wndtemplate; return -1; }

	//Insert Page
	return _InsertPage(caption, wndtemplate, idx, TRUE);
}

//---------------------------------------------------------------------------
// Function name	: InsertPage
// Description		: return -1 if an error occurs
//					 Make sure template had WS_CHILD style
//---------------------------------------------------------------------------
int cRollupWindow::InsertPage(LPCTSTR caption, CDialog* pwndTemplate, BOOL bAutoDestroyTpl, int idx)
{
	if( !pwndTemplate)		return -1;

	if( idx>0 && idx>=m_PageList.GetSize())		idx=-1;

	//Insert Page
	return _InsertPage(caption, pwndTemplate, idx, bAutoDestroyTpl);
}

//---------------------------------------------------------------------------
// Function name	: _InsertPage
// Description	 : Called by InsertPage(...) methods
//									Return -1 if an error occurs
//------------------------------------------------	---------------------------
int cRollupWindow::_InsertPage(LPCTSTR caption, CDialog* pwndTemplate, int idx, BOOL bAutoDestroyTpl)
{
	assert(pwndTemplate!=0);
	assert(pwndTemplate->m_hWnd!=0);

 	//Get client rect
	CRect r; GetClientRect(r);

	//Create GroupBox
	CButton* groupbox = new CButton;
	groupbox->Create(_T(""), WS_CHILD|BS_GROUPBOX, r, this, 0 );

	//Create Button
	CButton* but = new CButton;
	but->Create(caption, WS_CHILD|BS_AUTOCHECKBOX|BS_PUSHLIKE|BS_FLAT, r, this, 0 );

	//Change Button's font
	HFONT hfont= (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	CFont* font = CFont::FromHandle(hfont);
	but->SetFont(font);

	//Add page at pagelist
	RC_PAGEINFO* pi			= new RC_PAGEINFO;
	pi->cstrCaption			= caption;
	pi->bExpanded				= FALSE;
	pi->bEnable					= TRUE;
	pi->pwndTemplate		= pwndTemplate;
	pi->pwndButton			= but;
	pi->pwndGroupBox		= groupbox;
	pi->pOldDlgProc			= (WNDPROC)::GetWindowLong(pwndTemplate->m_hWnd, DWL_DLGPROC);
	pi->pOldButProc			= (WNDPROC)::GetWindowLong(but->m_hWnd, GWL_WNDPROC);
	pi->bAutoDestroyTpl	= bAutoDestroyTpl;

	int newidx;
	if( idx<0)	newidx = (int)m_PageList.Add(pi);
	else	{	m_PageList.InsertAt(idx, pi); newidx=idx; }

	//Set Dlg Window datas
	::SetWindowLong(pwndTemplate->m_hWnd, GWL_USERDATA,	(LONG)m_PageList[newidx]);
	::SetWindowLong(pwndTemplate->m_hWnd, DWL_USER,		(LONG)this);

	//Set But Window data
	::SetWindowLong(but->m_hWnd, GWL_USERDATA,	(LONG)m_PageList[newidx]);

	//SubClass Template window proc
	::SetWindowLong(pwndTemplate->m_hWnd, DWL_DLGPROC, (LONG)cRollupWindow::DlgWindowProc);

	//SubClass Button window proc
	::SetWindowLong(but->m_hWnd, GWL_WNDPROC, (LONG)cRollupWindow::ButWindowProc);

	//Update
	m_PageHeight+=RC_PGBUTTONHEIGHT+(RC_GRPBOXINDENT/2);
	RecalLayout();

	return newidx;
}


//---------------------------------------------------------------------------
// Function name	: RemovePage
// Description		: 
//---------------------------------------------------------------------------
void cRollupWindow::RemovePage(int idx)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return;

	//Remove
	_RemovePage(idx);

	//Update
	RecalLayout();
}

//---------------------------------------------------------------------------
// Function name	: RemoveAllPages
// Description		:
//---------------------------------------------------------------------------
void cRollupWindow::RemoveAllPages()
{
	//Remove all
	while (m_PageList.GetSize())	_RemovePage(0);

	//Update
	RecalLayout();
}

//---------------------------------------------------------------------------
// Function name	: _RemovePage
// Description		: Called by RemovePage or RemoveAllPages methods
//---------------------------------------------------------------------------
void cRollupWindow::_RemovePage(int idx)
{
	RC_PAGEINFO* pi = m_PageList[idx];

	//Get Page Rect
	CRect tr; pi->pwndTemplate->GetWindowRect(&tr);

	//Update PageHeight
	m_PageHeight-=RC_PGBUTTONHEIGHT+(RC_GRPBOXINDENT/2);
	if( pi->bExpanded)		m_PageHeight-=tr.Height();

	//Remove wnds
	if( pi->pwndButton)				delete pi->pwndButton;
	if( pi->pwndGroupBox)			delete pi->pwndGroupBox;

	if( pi->pwndTemplate)
	{
		if( pi->bAutoDestroyTpl) {
			pi->pwndTemplate->DestroyWindow();
			delete pi->pwndTemplate;
		} else {
			pi->pwndTemplate->ShowWindow(SW_HIDE);
			::SetWindowLong(pi->pwndTemplate->m_hWnd, DWL_DLGPROC, (LONG)pi->pOldDlgProc);
		}
	}

	//Remove page from array
	m_PageList.RemoveAt(idx);

	//Delete pageinfo
	delete pi;
}


//---------------------------------------------------------------------------
// Function name	: ExpandPage
// Description	 : 
//---------------------------------------------------------------------------
void cRollupWindow::ExpandPage(int idx, BOOL bExpand, BOOL bScrollToPage)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return;

	//Expand-collapse
	_ExpandPage(m_PageList[idx], bExpand);

	//Update
	RecalLayout();

	//Scroll to this page (Automatic page visibility)
	if( bScrollToPage&&bExpand)		ScrollToPage(idx, FALSE);

}

//---------------------------------------------------------------------------
// Function name	: ExpandAllPages
// Description	 : 
//---------------------------------------------------------------------------
void cRollupWindow::ExpandAllPages(BOOL bExpand)
{
	//Expand-collapse All
	for (int i=0; i<m_PageList.GetSize(); i++)
		_ExpandPage(m_PageList[i], bExpand);

	//Update
	RecalLayout();
}

//---------------------------------------------------------------------------
// Function name	: _ExpandPage
// Description	 : Called by ExpandPage or ExpandAllPages methods
//---------------------------------------------------------------------------
void	cRollupWindow::_ExpandPage(RC_PAGEINFO* pi, BOOL bExpand)
{
	//Check if we need to change state
	if( pi->bExpanded==bExpand)					return;
	if( !pi->bEnable)										return;

	//Get Page Rect
	CRect tr; pi->pwndTemplate->GetWindowRect(&tr);

	//Expand-collapse
	pi->bExpanded = bExpand;

	if( bExpand)	m_PageHeight+=tr.Height();
	else					m_PageHeight-=tr.Height();
}


//---------------------------------------------------------------------------
// Function name	: EnablePage
// Description	 : 
//---------------------------------------------------------------------------
void cRollupWindow::EnablePage(int idx, BOOL bEnable)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return;

	//Enable-Disable
	_EnablePage(m_PageList[idx], bEnable);

	//Update
	RecalLayout();
}


//---------------------------------------------------------------------------
// Function name	: EnableAllPages
// Description	 : 
//---------------------------------------------------------------------------
void cRollupWindow::EnableAllPages(BOOL bEnable)
{
	//Enable-disable All
	for (int i=0; i<m_PageList.GetSize(); i++)
		_EnablePage(m_PageList[i], bEnable);

	//Update
	RecalLayout();
}

//---------------------------------------------------------------------------
// Function name	: _EnablePage
// Description	 : Called by EnablePage or EnableAllPages methods
//---------------------------------------------------------------------------
void cRollupWindow::_EnablePage(RC_PAGEINFO* pi, BOOL bEnable)
{
	//Check if we need to change state
	if( pi->bEnable==bEnable)		return;

	//Get Page Rect
	CRect tr; pi->pwndTemplate->GetWindowRect(&tr);

	//Change state
	pi->bEnable = bEnable;

	if( pi->bExpanded)		{ m_PageHeight-=tr.Height(); pi->bExpanded=FALSE; }
}


//---------------------------------------------------------------------------
// Function name	: ScrollToPage
// Description		: Scroll a page at the top of the RollupCtrl if bAtTheTop=TRUE
//					 or just ensure page visibility into view if bAtTheTop=FALSE
//---------------------------------------------------------------------------
void cRollupWindow::ScrollToPage(int idx, BOOL bAtTheTop)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return;

	//Get page infos
	RC_PAGEINFO* pi = m_PageList[idx];

	//Get windows rect
	CRect r; GetWindowRect(&r);
	CRect tr; pi->pwndTemplate->GetWindowRect(&tr);

	//Check page visibility
	if( bAtTheTop || ((tr.bottom>r.bottom) || (tr.top<r.top)))
	{
		//Compute new m_nStartYPos
		pi->pwndButton->GetWindowRect(&tr);
		m_StartYPos-= (tr.top-r.top);

		//Update
		RecalLayout();
	}

}


//---------------------------------------------------------------------------
// Function name	: MovePageAt
// Description		: newidx can be equal to -1 (move at end)
//					 Return -1 if an error occurs
//---------------------------------------------------------------------------
int cRollupWindow::MovePageAt(int idx, int newidx)
{
	if( idx==newidx)									return -1;
	if( idx>=m_PageList.GetSize() || idx<0)				return -1;

	if( newidx>0 && newidx>=m_PageList.GetSize())		newidx=-1;

	//Remove page from its old position
	RC_PAGEINFO* pi = m_PageList[idx];
	m_PageList.RemoveAt(idx);

	//Insert at its new position
	int retidx;
	if( newidx<0)	retidx = (int)m_PageList.Add(pi);
	else	{ m_PageList.InsertAt(newidx, pi); retidx=newidx; }


	//Update
	RecalLayout();
	
	return retidx;
}


//---------------------------------------------------------------------------
// Function name	: IsPageExpanded
// Description		: 
//---------------------------------------------------------------------------
BOOL cRollupWindow::IsPageExpanded(int idx)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return FALSE;
	return m_PageList[idx]->bExpanded;
}

//---------------------------------------------------------------------------
// Function name	: IsPageEnabled
// Description		: 
//---------------------------------------------------------------------------
BOOL cRollupWindow::IsPageEnabled(int idx)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return FALSE;
	return m_PageList[idx]->bEnable;
}




//---------------------------------------------------------------------------
// Function name	: RecalLayout
// Description	 : 
//---------------------------------------------------------------------------
void cRollupWindow::RecalLayout()
{
	//Check StartPosY
	CRect r; GetClientRect(&r);
	int BottomPagePos = m_StartYPos+m_PageHeight;

	int nWidth	= r.Width();
	int nHeight = r.Height();
	if( BottomPagePos<nHeight)		m_StartYPos = nHeight-m_PageHeight;
	if( m_StartYPos>0)						m_StartYPos = 0;

	////////////////////////////////////////////
	//Calc new pages's positions 
	// used column sub-divisions if necessary
	int nPageWidth = nWidth-RC_SCROLLBARWIDTH;

	if( m_bEnabledAutoColumns)
	{
		nPageWidth=m_nColumnWidth;
		if( nPageWidth>nWidth-RC_SCROLLBARWIDTH)
			nPageWidth=nWidth-RC_SCROLLBARWIDTH;
	}

	int posx=0;
	int posy=0;
	int nMaxHeight=-1;

	CArray<CPoint, CPoint&> carrayPos;

	for (int i=0; i<m_PageList.GetSize(); i++)
	{
		RC_PAGEINFO* pi = m_PageList[i];

		//Page Height
		int nCurPageHeight=RC_PGBUTTONHEIGHT+(RC_GRPBOXINDENT/2);
		if( pi->bExpanded && pi->bEnable)
		{
			CRect tr; pi->pwndTemplate->GetWindowRect(&tr);
			nCurPageHeight+=tr.Height();
		}

		//Split to a new column ?
		if( m_bEnabledAutoColumns && ((nWidth-posx-m_nColumnWidth-RC_SCROLLBARWIDTH )>m_nColumnWidth) && i!=0 && (posy+(nCurPageHeight/2))>nHeight)
		{
			posx+=m_nColumnWidth;	//New column
			posy=0;
		}

		CPoint cpos(posx, posy);
		carrayPos.Add(cpos);
		
		posy+=nCurPageHeight;
		if( posy>nMaxHeight)	nMaxHeight=posy;
	}

	if( nMaxHeight!=-1)
	{
		m_PageHeight=nMaxHeight;

		BottomPagePos = m_StartYPos+m_PageHeight;
		if( BottomPagePos<nHeight)		m_StartYPos = nHeight-m_PageHeight;
		if( m_StartYPos>0)						m_StartYPos = 0;
	}

	////////////////////////////////////////////
	//Update children windows position
	HDWP hdwp = BeginDeferWindowPos((int)m_PageList.GetSize()*3);	//*3 for pwndButton+pwndTemplate+pwndGroupBox
	if( hdwp)
	{

		int posx=0;
		int posy=m_StartYPos;

		for (int i=0; i<m_PageList.GetSize(); i++)
		{
			RC_PAGEINFO* pi = m_PageList[i];

			posx=carrayPos[i].x;
			posy=carrayPos[i].y+m_StartYPos;

			//Enable-Disable Button
			pi->pwndButton->SetCheck(pi->bEnable&pi->bExpanded);
			pi->pwndButton->EnableWindow(pi->bEnable);

			//Update Button's position and size

			//Expanded
			if( pi->bExpanded && pi->bEnable)	
			{
				CRect tr; pi->pwndTemplate->GetWindowRect(&tr);

				//Update GroupBox position and size
				DeferWindowPos(hdwp, pi->pwndGroupBox->m_hWnd, 0, posx+2, posy, nPageWidth-3, tr.Height()+RC_PGBUTTONHEIGHT+RC_GRPBOXINDENT-4, SWP_NOZORDER|SWP_SHOWWINDOW);

				//Update Template position and size
				DeferWindowPos(hdwp, pi->pwndTemplate->m_hWnd, 0, posx+RC_GRPBOXINDENT, posy+RC_PGBUTTONHEIGHT, nPageWidth-(RC_GRPBOXINDENT*2), tr.Height(), SWP_NOZORDER);

				//Update Button's position and size
				DeferWindowPos(hdwp, pi->pwndButton->m_hWnd, 0, posx+RC_GRPBOXINDENT, posy, nPageWidth-(RC_GRPBOXINDENT*2), RC_PGBUTTONHEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW);

			//Collapsed
			} else {	

				//Update GroupBox position and size
				DeferWindowPos(hdwp, pi->pwndGroupBox->m_hWnd, 0, posx+2, posy, nPageWidth-3, 16, SWP_NOZORDER|SWP_SHOWWINDOW);

				//Update Template position and size
				DeferWindowPos(hdwp, pi->pwndTemplate->m_hWnd, 0, posx+RC_GRPBOXINDENT, 0, 0, 0,SWP_NOZORDER|SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE);

				//Update Button's position and size
				DeferWindowPos(hdwp, pi->pwndButton->m_hWnd, 0, posx+RC_GRPBOXINDENT, posy, nPageWidth-(RC_GRPBOXINDENT*2), RC_PGBUTTONHEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW);

			}

		}
		EndDeferWindowPos(hdwp);
	}

	////////////////////////////////////////////
	//Update children windows visibility
	hdwp = BeginDeferWindowPos((int)m_PageList.GetSize());
	if( hdwp)
	{

		for (int i=0; i<m_PageList.GetSize(); i++){
			RC_PAGEINFO* pi = m_PageList[i];

			//Expanded
			if( pi->bExpanded && pi->bEnable) {
				DeferWindowPos(hdwp, pi->pwndTemplate->m_hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
			//Collapsed
			} else {
				DeferWindowPos(hdwp, pi->pwndTemplate->m_hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE);
			}

		}
		EndDeferWindowPos(hdwp);
	}


	//////////////////////////////////////////
	//Update Scroll Bar
	CRect br = CRect(r.right-RC_SCROLLBARWIDTH,r.top, r.right, r.bottom);
	InvalidateRect(&br, FALSE);
	UpdateWindow();
}

void cRollupWindow::Resize( UINT type, int cx, int cy )
{
	cx -= 12;
	cy -= 12;
	CWnd::MoveWindow(4, 4, cx, cy);
	OnSize( type, cx, cy );
}

//---------------------------------------------------------------------------
// Function name	: GetPageIdxFromButtonHWND
// Description	 : Return -1 if matching hwnd not found
//---------------------------------------------------------------------------
int cRollupWindow::GetPageIdxFromButtonHWND(HWND hwnd)
{
	//Search matching button's hwnd
	for (int i=0; i<m_PageList.GetSize(); i++)
		if( hwnd==m_PageList[i]->pwndButton->m_hWnd)		return i;

	return -1;
}

//---------------------------------------------------------------------------
// Function name	: GetPageInfo
// Description	 : Return -1 if an error occurs
//---------------------------------------------------------------------------
const RC_PAGEINFO* cRollupWindow::GetPageInfo(int idx)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return (RC_PAGEINFO*)-1;

	return m_PageList[idx];
}


//---------------------------------------------------------------------------
// Function name	: EnableAutoColumns
// Description	 : ...
//---------------------------------------------------------------------------
void cRollupWindow::EnableAutoColumns(BOOL bEnable)
{
	if( m_bEnabledAutoColumns!=bEnable)
	{
		m_bEnabledAutoColumns=bEnable;
		RecalLayout();
	}

}


//---------------------------------------------------------------------------
// Function name	: SetColumnWidth
// Description	 : ...
//---------------------------------------------------------------------------
BOOL cRollupWindow::SetColumnWidth(int nWidth)
{
	if( nWidth>RC_MINCOLUMNWIDTH)
	{
		m_nColumnWidth=nWidth;
		RecalLayout();
		return TRUE;
	}

	return FALSE;
}

//---------------------------------------------------------------------------
// Function name	: SetPageCaption
// Description	 : ...
//---------------------------------------------------------------------------
BOOL cRollupWindow::SetPageCaption(int idx, LPCTSTR caption)
{
	if( idx>=m_PageList.GetSize() || idx<0)				return FALSE;
	
	m_PageList[idx]->pwndButton->SetWindowText(caption);
	m_PageList[idx]->cstrCaption = caption;

	return TRUE;	
}


//---------------------------------------------------------------------------
// SubClasser
//---------------------------------------------------------------------------
LRESULT CALLBACK cRollupWindow::DlgWindowProc(HWND hWnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	RC_PAGEINFO* pi		= (RC_PAGEINFO*)GetWindowLong(hWnd, GWL_USERDATA);
	cRollupWindow*	_this = (cRollupWindow*)GetWindowLong(hWnd, DWL_USER);

	CRect r; _this->GetClientRect(&r);
	if( _this->m_PageHeight>r.Height())	//Can Scroll ?
	{

		switch (uMsg) {

			case WM_MBUTTONDOWN:
			case WM_LBUTTONDOWN: {
				CPoint pos; GetCursorPos(&pos);
				_this->m_OldMouseYPos = pos.y;
				::SetCapture(hWnd);
				::SetFocus(_this->m_hWnd);
			break; }

			case WM_MBUTTONUP:
			case WM_LBUTTONUP:
				if( ::GetCapture()==hWnd) 	::ReleaseCapture();
			break;

			case WM_MOUSEMOVE: {

					if( (wparam==MK_LBUTTON||wparam==MK_MBUTTON) && ::GetCapture()==hWnd) {
						CPoint pos; GetCursorPos(&pos);
						_this->m_StartYPos+=(pos.y-_this->m_OldMouseYPos);
						_this->RecalLayout();
						_this->m_OldMouseYPos = pos.y;
						//return 0;
					}

			break;}

			case WM_SETCURSOR:
			{
				if( (HWND)wparam==hWnd)	{ ::SetCursor(::LoadCursor(0, RC_CURSOR)); return TRUE; }
			break;
			}

		}//switch(uMsg)

	}

	return ::CallWindowProc(pi->pOldDlgProc, hWnd, uMsg, wparam, lparam);
}

//---------------------------------------------------------------------------
// Button SubClasser
//---------------------------------------------------------------------------
LRESULT CALLBACK cRollupWindow::ButWindowProc(HWND hWnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	if( uMsg==WM_SETFOCUS)		return FALSE;

	RC_PAGEINFO* pi	= (RC_PAGEINFO*)GetWindowLong(hWnd, GWL_USERDATA);
	return ::CallWindowProc(pi->pOldButProc, hWnd, uMsg, wparam, lparam);
}


/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl message handlers

//---------------------------------------------------------------------------
// OnCommand
//---------------------------------------------------------------------------
BOOL cRollupWindow::OnCommand(WPARAM wparam, LPARAM lparam) 
{
	if( LOWORD(wparam)==RC_MID_COLLAPSEALL) {
		ExpandAllPages(FALSE);
		return TRUE;

	} else if( LOWORD(wparam)==RC_MID_EXPANDALL) {
		ExpandAllPages(TRUE);
		return TRUE;

	}	else if( LOWORD(wparam)>=RC_MID_STARTPAGES
		&&	 LOWORD(wparam)<RC_MID_STARTPAGES+GetPagesCount())
	{
		int idx = LOWORD(wparam)-RC_MID_STARTPAGES;
		ExpandPage(idx, !IsPageExpanded(idx) );

	} else if( HIWORD(wparam)==BN_CLICKED) {
		int idx = GetPageIdxFromButtonHWND((HWND)lparam);
		if( idx!=-1) {
			RC_PAGEINFO*	pi = m_PageList[idx];
			ExpandPage(idx, !pi->bExpanded);
			SetFocus();
			return TRUE;
		}
	}

	return CWnd::OnCommand(wparam, lparam);
}

//---------------------------------------------------------------------------
// OnPaint
//---------------------------------------------------------------------------
void cRollupWindow::OnPaint() 
{
	CPaintDC dc(this);

	//Draw ScrollBar
	CRect r; GetClientRect(&r);
	CRect br = CRect(r.right-RC_SCROLLBARWIDTH,r.top, r.right, r.bottom);
	dc.DrawEdge(&br, EDGE_RAISED, BF_RECT );

	int SB_Pos	= 0;
	int SB_Size = 0;
	int ClientHeight = r.Height()-4;

	if( m_PageHeight>r.Height()) {
		SB_Size = ClientHeight-(((m_PageHeight-r.Height())*ClientHeight)/m_PageHeight);
		SB_Pos	= -(m_StartYPos*ClientHeight)/m_PageHeight;
	} else {
		SB_Size = ClientHeight;
	}

	br.left		+=2;
	br.right	-=1;
	br.top		= SB_Pos+2;
	br.bottom	= br.top+SB_Size;

	dc.FillSolidRect(&br, RC_SCROLLBARCOLOR);
	dc.FillSolidRect(CRect(br.left,2,br.right,br.top), RGB(0,0,0));
	dc.FillSolidRect(CRect(br.left,br.bottom,br.right,2+ClientHeight), RGB(0,0,0));

	// Do not call CWnd::OnPaint() for painting messages
}

//---------------------------------------------------------------------------
// OnSize
//---------------------------------------------------------------------------
void cRollupWindow::OnSize(UINT type, int cx, int cy) 
{
	CWnd::OnSize(type, cx, cy);

	RecalLayout();
}

//---------------------------------------------------------------------------
// OnLButtonDown
//---------------------------------------------------------------------------
void cRollupWindow::OnLButtonDown(UINT flags, CPoint point) 
{
	CRect r; GetClientRect(&r);
	if( m_PageHeight>r.Height())
	{

		//Click on scroll bar client rect
		CRect br = CRect(r.right-RC_SCROLLBARWIDTH,r.top, r.right, r.bottom);
		if( (flags&MK_LBUTTON) && br.PtInRect(point)) {

			SetCapture();

			int ClientHeight	= r.Height()-4;

			int SB_Size = ClientHeight-(((m_PageHeight-r.Height())*ClientHeight)/m_PageHeight);
			int	SB_Pos	= -(m_StartYPos*ClientHeight)/m_PageHeight;

			//Click inside scrollbar cursor
			if( (point.y<(SB_Pos+SB_Size)) && (point.y>SB_Pos)) {

				m_SBOffset = SB_Pos-point.y+1;

			//Click outside scrollbar cursor (2 cases => above or below cursor)
			} else {
				int distup		= point.y-SB_Pos;	
				int distdown	= (SB_Pos+SB_Size)-point.y;
				if( distup<distdown)	m_SBOffset = 0;					//above
				else									m_SBOffset = -SB_Size;	//below
			}

			//Calc new m_StartYPos from mouse pos
			int TargetPos	= point.y + m_SBOffset;
			m_StartYPos=-(TargetPos*m_PageHeight)/(ClientHeight);

			//Update
			RecalLayout();
		}


		//Click on scroll bar up button
		br = CRect(r.right-RC_SCROLLBARWIDTH,r.top, r.right, r.top);
		if( (flags&MK_LBUTTON) && br.PtInRect(point)) {
			m_StartYPos+=32;
			RecalLayout();
		}

		//Click on scroll bar down button
		br = CRect(r.right-RC_SCROLLBARWIDTH,r.bottom, r.right, r.bottom);
		if( (flags&MK_LBUTTON) && br.PtInRect(point)) {
			m_StartYPos-=32;
			RecalLayout();
		}
	}

	CWnd::OnLButtonDown(flags, point);
}

//---------------------------------------------------------------------------
// OnLButtonUp
//---------------------------------------------------------------------------
void cRollupWindow::OnLButtonUp(UINT flags, CPoint point) 
{
	ReleaseCapture();
	CWnd::OnLButtonUp(flags, point);
}


//---------------------------------------------------------------------------
// OnMouseMove
//---------------------------------------------------------------------------
void cRollupWindow::OnMouseMove(UINT flags, CPoint point) 
{
	CRect r; GetClientRect(&r);
	if( m_PageHeight>r.Height())
	{

		if( (flags&MK_LBUTTON) && (GetCapture()==this)) {

			//Calc new m_StartYPos from mouse pos
			int ClientHeight	= r.Height() - 4;
			int TargetPos			= point.y + m_SBOffset;
			m_StartYPos=-(TargetPos*m_PageHeight)/ClientHeight;

			//Update
			RecalLayout();
		}

	}
	
	CWnd::OnMouseMove(flags, point);
}

//---------------------------------------------------------------------------
// OnMouseWheel
//---------------------------------------------------------------------------
BOOL cRollupWindow::OnMouseWheel( UINT flags, short zDelta, CPoint pt)
{
	m_StartYPos+=(zDelta/4);
	RecalLayout();

	return CWnd::OnMouseWheel(flags, zDelta, pt);
}

//---------------------------------------------------------------------------
// OnContextMenu
//---------------------------------------------------------------------------
void cRollupWindow::OnContextMenu( CWnd* /*pWnd*/, CPoint pos )
{
	//TRACE("CRollupCtrl::OnContextMenu\n");

	if( m_cmenuCtxt.m_hMenu)		m_cmenuCtxt.DestroyMenu();

	if( m_cmenuCtxt.CreatePopupMenu())
	{
		GetCursorPos(&pos);	//Cursor position even with keyboard 'Context key'

		m_cmenuCtxt.AppendMenu(MF_STRING, RC_MID_EXPANDALL, _T("Open all pages...")	);
		m_cmenuCtxt.AppendMenu(MF_STRING, RC_MID_COLLAPSEALL, _T("Close all pages...") );
		m_cmenuCtxt.AppendMenu(MF_SEPARATOR, 0, _T("") );

		//Add all pages with checked style for expanded ones
		for (int i=0; i<m_PageList.GetSize(); i++) {
			CString cstrPageName;
			m_PageList[i]->pwndButton->GetWindowText(cstrPageName);
			m_cmenuCtxt.AppendMenu(MF_STRING, RC_MID_STARTPAGES+i, cstrPageName);	
			if( m_PageList[i]->bExpanded)
				m_cmenuCtxt.CheckMenuItem(RC_MID_STARTPAGES+i, MF_CHECKED);
		}

		m_cmenuCtxt.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pos.x, pos.y, this);
	}
}
