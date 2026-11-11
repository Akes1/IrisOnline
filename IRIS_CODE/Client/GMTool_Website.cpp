#include "stdafx.h"
#include "GMTool.h"
#include "resource.h"
#include <shellapi.h>

void cGMTool::OnShowWebSite()
{

	HWND hWnd = mSubDlg[GMTOOL_SUBDLG_WEBSITE];

	/// 위치 셋팅
	RECT rc0;
	::GetWindowRect( mMainDlg, &rc0 );
	int x = rc0.right;
	int y = rc0.top + GetSystemMetrics(SM_CYCAPTION);
	::SetWindowPos( hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

	mSubDlgPos[GMTOOL_SUBDLG_WEBSITE].x = 0;
	mSubDlgPos[GMTOOL_SUBDLG_WEBSITE].y = GetSystemMetrics(SM_CYCAPTION);
}

void cGMTool::OnCommandWebSite( int id, int msg )
{
	switch( id )
	{
	case IDC_BUTTON_WEBSITE_CANCEL:
		::ShowWindow( mSubDlg[GMTOOL_SUBDLG_WEBSITE], SW_HIDE );
		::SetActiveWindow( mParentWnd );
		break;
	case IDC_BUTTON_WEBSITE_HOME:
		{
			TCHAR website[MAX_PATH] = _T("");
			LoadString( mInst, IDS_WEBSITE_HOME, website, MAX_PATH );
			ShellExecute( 0, 0, _T("Iexplore.exe"), website, 0, SW_SHOWNORMAL );
		}
		break;
	case IDC_BUTTON_WEBSITE_MANAGER:
		{
			TCHAR website[MAX_PATH];
			LoadString( mInst, IDS_WEBSITE_MANAGER, website, MAX_PATH );
			ShellExecute( 0, 0, _T("Iexplore.exe"), website, 0, SW_SHOWNORMAL );
		}
		break;
	}
}