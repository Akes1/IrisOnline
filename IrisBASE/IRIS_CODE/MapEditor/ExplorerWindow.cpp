#include "stdafx.h"
#include "ExplorerWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(cExplorerWindow, CView)

BEGIN_MESSAGE_MAP(cExplorerWindow, CView)
	//{{AFX_MSG_MAP(cExplorerWindow)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	//ON_COMMAND(ID_FILE_PROPERTIES, OnFileProperties)
	//ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateFileProperties)
	//ON_COMMAND(ID_FILE_RENAME, OnFileRename)
	//ON_UPDATE_COMMAND_UI(ID_FILE_RENAME, OnUpdateFileRename)
	//ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	//ON_COMMAND(ID_GO_UPONELEVEL, OnGoUponelevel)
	//ON_UPDATE_COMMAND_UI(ID_GO_UPONELEVEL, OnUpdateGoUponelevel)
	//ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	//ON_COMMAND(ID_TOOLS_ALLOWDELETE, OnToolsAllowdelete)
	//ON_UPDATE_COMMAND_UI(ID_TOOLS_ALLOWDELETE, OnUpdateToolsAllowdelete)
	//ON_COMMAND(ID_TOOLS_ALLOWOPEN, OnToolsAllowopen)
	//ON_UPDATE_COMMAND_UI(ID_TOOLS_ALLOWOPEN, OnUpdateToolsAllowopen)
	//ON_COMMAND(ID_TOOLS_ALLOWRENAME, OnToolsAllowrename)
	//ON_UPDATE_COMMAND_UI(ID_TOOLS_ALLOWRENAME, OnUpdateToolsAllowrename)
	//ON_COMMAND(ID_TOOLS_ALLOWSPROPERTIES, OnToolsAllowsproperties)
	//ON_UPDATE_COMMAND_UI(ID_TOOLS_ALLOWSPROPERTIES, OnUpdateToolsAllowsproperties)
	//ON_COMMAND(ID_TOOLS_SHOWFILES, OnToolsShowfiles)
	//ON_UPDATE_COMMAND_UI(ID_TOOLS_SHOWFILES, OnUpdateToolsShowfiles)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

cExplorerWindow::cExplorerWindow()
{
}

cExplorerWindow::~cExplorerWindow()
{
}

BOOL cExplorerWindow::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void cExplorerWindow::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	mFileTree.PopulateTree();
}

void cExplorerWindow::OnDraw(CDC* /*dc*/)
{
}

#ifdef _DEBUG
void cExplorerWindow::AssertValid() const
{
	CView::AssertValid();
}

void cExplorerWindow::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


int cExplorerWindow::OnCreate(LPCREATESTRUCT cs)
{
	if (CView::OnCreate(cs) == -1)
		return -1;

	//Create the tree view
	CRect r;
	if (!mFileTree.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_EDITLABELS | TVS_SHOWSELALWAYS, r, this, 100))
	{
		assert( 0 && "failed to create file tree ctrl" );
		return -1;
	}

	/// 현재 디렉토리를 루트 폴더로 설정
	char path[MAX_PATH];
	GetCurrentDirectory( MAX_PATH, path );
	mFileTree.SetRootFolder( CString(path) );
	return 0;
}

void cExplorerWindow::OnSize(UINT type, int cx, int cy)
{
	//Let the parent class do its thing
	CView::OnSize(type, cx, cy);

	//Let the tree control occupy all the space of the client area
	CRect r;
	GetClientRect(&r);
	mFileTree.MoveWindow(r);
}


void cExplorerWindow::OnSetFocus(CWnd* poldWnd)
{
	//Let the parent class do its thing
	CView::OnSetFocus(poldWnd);

	mFileTree.SetFocus();
}

/*
void cExplorerWindow::OnFileProperties()
{
	mFileTree.OnFileProperties();
}

void cExplorerWindow::OnUpdateFileProperties(CCmdUI* pCmdUI)
{
	mFileTree.OnUpdateFileProperties(pCmdUI);
}

void cExplorerWindow::OnFileRename()
{
	mFileTree.OnFileRename();
}

void cExplorerWindow::OnUpdateFileRename(CCmdUI* pCmdUI)
{
	mFileTree.OnUpdateFileRename(pCmdUI);
}

void cExplorerWindow::OnFileOpen()
{
	mFileTree.OnFileOpen();
}

void cExplorerWindow::OnUpdateFileOpen(CCmdUI* pCmdUI)
{
	mFileTree.OnUpdateFileOpen(pCmdUI);
}

void cExplorerWindow::OnGoUponelevel()
{
	mFileTree.UpOneLevel();
}

void cExplorerWindow::OnUpdateGoUponelevel(CCmdUI* pCmdUI)
{
	mFileTree.OnUpdateUpOneLevel(pCmdUI);
}

void cExplorerWindow::OnViewRefresh()
{
	mFileTree.OnViewRefresh();
}

void cExplorerWindow::OnToolsAllowdelete()
{
	mFileTree.SetAllowDelete(!mFileTree.GetAllowDelete());
}

void cExplorerWindow::OnUpdateToolsAllowdelete(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(mFileTree.GetAllowDelete());
}

void cExplorerWindow::OnToolsAllowopen()
{
	mFileTree.SetAllowOpen(!mFileTree.GetAllowOpen());
}

void cExplorerWindow::OnUpdateToolsAllowopen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(mFileTree.GetAllowOpen());
}

void cExplorerWindow::OnToolsAllowrename()
{
	mFileTree.SetAllowRename(!mFileTree.GetAllowRename());
}

void cExplorerWindow::OnUpdateToolsAllowrename(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(mFileTree.GetAllowRename());
}

void cExplorerWindow::OnToolsAllowsproperties()
{
	mFileTree.SetAllowProperties(!mFileTree.GetAllowProperties());
}

void cExplorerWindow::OnUpdateToolsAllowsproperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(mFileTree.GetAllowProperties());
}

void cExplorerWindow::OnToolsShowfiles()
{
	mFileTree.SetShowFiles(!mFileTree.GetShowFiles());
}

void cExplorerWindow::OnUpdateToolsShowfiles(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(mFileTree.GetShowFiles());
}
*/
