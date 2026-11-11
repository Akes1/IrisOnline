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

#pragma once

class CDualSplitWnd;

// extended create context
struct CMPICreateContext : public CCreateContext
{
	int* m_pSplitData;
	DWORD m_dwViewParam;
};

// don't use these constants directly
// use the MPI_ macros from MPIDocTemplate.h
#define MPIT_VIEW		0
#define MPIT_HSPLIT		1
#define MPIT_VSPLIT		2
#define MPIT_TABS		3
#define MPIT_END		100

#define MPIS_BAR_LEFT	-1
#define MPIS_BAR_RIGHT	-2
#define MPIS_BAR_TOP	-1
#define MPIS_BAR_BOTTOM	-2

#define MPIBS_AUTO		-1


class CMPIChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CMPIChildFrame)
public:
	CMPIChildFrame();

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual void OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

public:
	CWnd* GetWndFromUID(int nUID);

public:
	virtual ~CMPIChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// recursive function for creating splitters and views
	BOOL CreateClientHelper(int& nPos, CWnd* pParent, int nID, CMPICreateContext* pContext);

protected:
	// automatically created splitter windows
	CTypedPtrList<CPtrList, CSplitterWnd*> m_listSplitters;

	// map UIDs to child windows
	CMapWordToPtr m_mapChildWnd;

protected:
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
};

