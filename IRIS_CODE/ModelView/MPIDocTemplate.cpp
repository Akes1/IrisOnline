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
#include "MPIDocTemplate.h"
#include "MPIChildFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CMPIDocTemplate, CMultiDocTemplate)

CMPIDocTemplate::CMPIDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass)
	: CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, NULL)
{
}

CMPIDocTemplate::~CMPIDocTemplate()
{
	WORD nID;	// destroy shared menus and accelerators
	POSITION pos = m_mapMenu.GetStartPosition();
	while (pos)
	{
		HMENU hMenu;
		m_mapMenu.GetNextAssoc(pos, nID, (void*&)hMenu);
		DestroyMenu(hMenu);
	}
	pos = m_mapAccel.GetStartPosition();
	while (pos)
	{
		HACCEL hAccel;
		m_mapAccel.GetNextAssoc(pos, nID, (void*&)hAccel);
		DestroyAcceleratorTable(hAccel);
	}
}

BEGIN_MESSAGE_MAP(CMPIDocTemplate, CMultiDocTemplate)
END_MESSAGE_MAP()


CDocument* CMPIDocTemplate::OpenDocumentFile(LPCTSTR /*lpszPathName*/, BOOL /*bMakeVisible*/)
{
	// don't create new document
	// NOTE: no support for serialization
	POSITION pos = GetFirstDocPosition();
	return GetNextDoc(pos);
}

CDocument* CMPIDocTemplate::CreateNewDocument()
{
	CDocument* pDoc = CMultiDocTemplate::CreateNewDocument();
	if (pDoc)
	{
		if (pDoc->OnNewDocument())
		{
			return pDoc;
		}
		delete pDoc;
	}
	return NULL;
}

CMPIChildFrame* CMPIDocTemplate::CreateMPIFrame(int nIDResource, ...)
{
	POSITION pos = GetFirstDocPosition();
	CDocument* pDoc = GetNextDoc(pos);

	if (pDoc)
	{
		m_pViewClass = NULL;

		UINT nLastID = m_nIDResource;
		HMENU hLastMenu = m_hMenuShared;
		HACCEL hLastAccel = m_hAccelTable;
		
		m_nIDResource = nIDResource;

		HMENU hNewMenu;
		// check if menu already created
		if (m_mapMenu.Lookup((WORD)nIDResource, (void*&)hNewMenu))
			m_hMenuShared = hNewMenu;
		else
		{	// load menu from resources
			hNewMenu = LoadMenu(AfxGetInstanceHandle(), MAKEINTRESOURCE(nIDResource));
			if (hNewMenu)
			{
				m_hMenuShared = hNewMenu;
				m_mapMenu[nIDResource] = hNewMenu;
			}
		}

		HACCEL hNewAccel;
		// check if accelerators already created
		if (m_mapAccel.Lookup((WORD)nIDResource, (void*&)hNewAccel))
			m_hAccelTable = hNewAccel;
		else
		{	// load accelerators from resources
			hNewAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(nIDResource));
			if (hNewAccel)
			{
				m_hAccelTable = hNewAccel;
				m_mapAccel[nIDResource] = hNewAccel;
			}
		}

		// initialize the extented create context
		CMPICreateContext context;
		context.m_pCurrentFrame = NULL;
		context.m_pCurrentDoc = pDoc;
		context.m_pNewViewClass = NULL;
		context.m_pNewDocTemplate = this;
		context.m_pSplitData = &nIDResource + 1;	// remaining function arguments

		// create and load child frame
		CMPIChildFrame* pFrame = (CMPIChildFrame*)m_pFrameClass->CreateObject();
		ASSERT_KINDOF(CMPIChildFrame, pFrame);
		if (!pFrame->LoadFrame(m_nIDResource, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, &context))
			return NULL;

		// restore data (already obtained by the child frame)
		m_nIDResource = nLastID;
		m_hMenuShared = hLastMenu;
		m_hAccelTable = hLastAccel;

		if (pFrame)
		{
			// load frame's title from resources
			CString strTitle;
			strTitle.LoadString(nIDResource);
			pFrame->SetTitle(strTitle);

			// display the window
			InitialUpdateFrame(pFrame, pDoc, TRUE);

			m_arrChildFrm.Add(pFrame);

			return pFrame;
		}
	}

	return NULL;
}

int CMPIDocTemplate::FindChildFrame(CMPIChildFrame *pFrame)
{
	for (int i=0; i<m_arrChildFrm.GetSize(); i++)
	{
		if (m_arrChildFrm[i] == pFrame)
			return i;
	}
	return -1;
}

CMPIChildFrame* CMPIDocTemplate::GetChildFrame(int nIndex)
{
	return m_arrChildFrm[nIndex];
}


DWORD CMPIDocTemplate::GetViewParam(CCreateContext* pContext)
{
	return ((CMPICreateContext*)pContext)->m_dwViewParam;
}

