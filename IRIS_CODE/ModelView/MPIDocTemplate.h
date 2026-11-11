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

class CMPIChildFrame;

// Macros describing the layout of the views
#define BEGIN_MPI_FRAME(templ, id)         if (!(templ)->CreateMPIFrame(id,
#define END_MPI_FRAME                      MPIT_END)) return FALSE;

// New style macros with unique identifiers
#define MPID_VIEW(uid, _class)             uid, MPIT_VIEW, RUNTIME_CLASS(_class), 0,
#define MPID_VIEW_EX(uid, _class, ex)      uid, MPIT_VIEW, RUNTIME_CLASS(_class), ex,

#define MPID_VSPLIT(uid, prop)             uid, MPIT_VSPLIT, prop,
#define MPID_HSPLIT(uid, prop)             uid, MPIT_HSPLIT, prop,

#define MPID_BAR_TOP_A(uid)                uid, MPIT_HSPLIT, MPIS_BAR_TOP, MPIBS_AUTO,
#define MPID_BAR_TOP(uid, size)            uid, MPIT_HSPLIT, MPIS_BAR_TOP, size,
#define MPID_BAR_BOTTOM_A(uid)             uid, MPIT_HSPLIT, MPIS_BAR_BOTTOM, MPIBS_AUTO,
#define MPID_BAR_BOTTOM(uid, size)         uid, MPIT_HSPLIT, MPIS_BAR_BOTTOM, size,
#define MPID_BAR_LEFT_A(uid)               uid, MPIT_VSPLIT, MPIS_BAR_LEFT, MPIBS_AUTO,
#define MPID_BAR_LEFT(uid, size)           uid, MPIT_VSPLIT, MPIS_BAR_LEFT, size,
#define MPID_BAR_RIGHT_A(uid)              uid, MPIT_VSPLIT, MPIS_BAR_RIGHT, MPIBS_AUTO,
#define MPID_BAR_RIGHT(uid, size)          uid, MPIT_VSPLIT, MPIS_BAR_RIGHT, size,

#define MPID_TABS(uid, id)                 uid, MPIT_TABS, id,

// Classic macros without unique identifiers
#define MPI_VIEW(_class)                   MPID_VIEW(0, _class)
#define MPI_VIEW_EX(_class, ex)            MPID_VIEW_EX(0, _class, ex)

#define MPI_VSPLIT(prop)                   MPID_VSPLIT(0, prop)
#define MPI_HSPLIT(prop)                   MPID_HSPLIT(0, prop)

#define MPI_BAR_TOP_A                      MPID_BAR_TOP_A(0)
#define MPI_BAR_TOP(size)                  MPID_BAR_TOP(0, size)
#define MPI_BAR_BOTTOM_A                   MPID_BAR_BOTTOM_A(0)
#define MPI_BAR_BOTTOM(size)               MPID_BAR_BOTTOM(0, size)
#define MPI_BAR_LEFT_A                     MPID_BAR_LEFT_A(0)
#define MPI_BAR_LEFT(size)                 MPID_BAR_LEFT(0, size)
#define MPI_BAR_RIGHT_A                    MPID_BAR_RIGHT_A(0)
#define MPI_BAR_RIGHT(size)                MPID_BAR_RIGHT(0, size)

#define MPI_TABS(id)                       MPID_TABS(0, id)


class CMPIDocTemplate : public CMultiDocTemplate
{
	DECLARE_DYNAMIC(CMPIDocTemplate)

public:
	CMPIDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass);

public:
	// Get child frame from index
	CMPIChildFrame* GetChildFrame(int nIndex);
	// Get index of the child frame
	int FindChildFrame(CMPIChildFrame* pFrame);

	// Create child frame with given layout parameters (use macros above)
	CMPIChildFrame* CreateMPIFrame(int nIDResource, ...);

	// Get view param from the create context
	static DWORD GetViewParam(CCreateContext* pContext);

public:
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE);
	virtual CDocument* CreateNewDocument();
protected:
	virtual ~CMPIDocTemplate();

protected:
	CMapWordToPtr m_mapMenu;
	CMapWordToPtr m_mapAccel;
	CArray<CMPIChildFrame*, CMPIChildFrame*> m_arrChildFrm;

	DECLARE_MESSAGE_MAP()
};

