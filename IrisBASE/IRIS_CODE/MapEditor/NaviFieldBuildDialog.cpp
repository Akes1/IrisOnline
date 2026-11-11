#include "stdafx.h"
#include "NaviFieldBuildDialog.h"

#include "Resource.h"
#include "NaviFieldEditor.h"

IMPLEMENT_DYNAMIC(cNaviFieldBuildDialog, CDialog)

BEGIN_MESSAGE_MAP(cNaviFieldBuildDialog, CDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_FBUILD_SYNCALL, OnClickedSyncAllButton)
END_MESSAGE_MAP()

cNaviFieldBuildDialog::cNaviFieldBuildDialog()
{
}

cNaviFieldBuildDialog::~cNaviFieldBuildDialog()
{
}

void cNaviFieldBuildDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_FBUILD_SYNCALL, mSyncAllButton );
}

BOOL cNaviFieldBuildDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

void cNaviFieldBuildDialog::OnClickedSyncAllButton()
{
	int mb = MessageBox( "Do you really want to sync all to NaviMesh?", "NaviField", MB_YESNO );

	if( mb == IDYES )
	{
		NAVIFIELDEDIT->SyncAllToNaviMesh();
	}
}
