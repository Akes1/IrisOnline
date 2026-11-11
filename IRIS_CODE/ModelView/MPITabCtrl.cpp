/********************************************************************
* Multi-Page Interface, version 1.3 (August 20, 2004)
* Copyright (C) 2003-2004 Michal Mecinski.
*
* You may freely use and modify this code, but don't remove
* this copyright note.
*
* THERE IS NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, FOR
* THIS CODE. THE AUTHOR DOES NOT TAKE THE RESPONSIBILITY
* FOR ANY DAMAGE RESULTING FROM THE USE OF IT.
*
* E-mail: mimec@mimec.org
* WWW: http://www.mimec.org
********************************************************************/

#include "stdafx.h"
#include "MPITabCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CMPITabCtrl::CMPITabCtrl()
{
}

CMPITabCtrl::~CMPITabCtrl()
{
}


BEGIN_MESSAGE_MAP(CMPITabCtrl, CTabCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()


int CMPITabCtrl::LoadTabs(int nID)
{
	HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(nID), RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nID), RT_TOOLBAR);
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	struct TBDATA
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;
		WORD aItems[1];
	};

	// get toolbar data
	TBDATA* pData = (TBDATA*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	ASSERT(pData->wVersion == 1);

/*
	// create and load image list
	if (!m_ImgList.Create(pData->wWidth, pData->wHeight, AILS_NEW, pData->wItemCount)
		|| !m_ImgList.AddBitmap(nID))
	{
		UnlockResource(hGlobal);
		FreeResource(hGlobal);
		return 0;
	}

	// set tab control's image list
	SendMessage(TCM_SETIMAGELIST, 0, (LPARAM)m_ImgList.GetImageList(AIL_NORMAL));
*/
	int nCnt = 0;

	// create tabs
	for (int i=0; i<pData->wItemCount; i++)
	{
		int nID = pData->aItems[i];
		if (!nID)
			continue;
		CString strItem;
		strItem.LoadString(nID);
		InsertItem(nCnt, strItem, nCnt);
		nCnt++;
	}

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return nCnt;
}

int CMPITabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// hack to receive the new modern font (Tahoma)
	// the tab control doesn't use it automatically
	CTreeCtrl ctrl;
	ctrl.Create(WS_CHILD, CRect(), this, 1);
	CFont* pFont = ctrl.GetFont();
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	ctrl.DestroyWindow();

	m_Font.CreateFontIndirect(&lf);
	SetFont(&m_Font);

	// disable window theme if WindowXP is detected
	WORD wVersion = LOWORD(GetVersion());
	if (LOBYTE(wVersion) > 5 || LOBYTE(wVersion) == 5 && HIBYTE(wVersion) >= 1)
	{	
		HMODULE hUxTheme = LoadLibrary(_T("uxtheme.dll"));
		if (hUxTheme)
		{
			typedef HRESULT (__stdcall *PFSETWINDOWTHEME)(HWND, LPCWSTR, LPCWSTR);
			PFSETWINDOWTHEME pfSetWindowTheme = 
				(PFSETWINDOWTHEME)GetProcAddress(hUxTheme, "SetWindowTheme");
			if (pfSetWindowTheme)
				pfSetWindowTheme(m_hWnd, NULL, L"");
			FreeModule(hUxTheme);
		}
	}

	return 0;
}

