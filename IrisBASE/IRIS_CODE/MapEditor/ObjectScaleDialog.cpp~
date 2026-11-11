#include "stdafx.h"
#include "ObjectScaleDialog.h"

#include "resource.h"
#include "ObjectEditor.h"

IMPLEMENT_DYNAMIC(cObjectScaleDialog, CDialog)

cObjectScaleDialog::cObjectScaleDialog( CWnd* parent )
: CDialog(IDD_DIALOG_OBJECT_SCALE, parent)
{
}

cObjectScaleDialog::~cObjectScaleDialog()
{
}

void cObjectScaleDialog::DoDataExchange( CDataExchange*  dx )
{
	CDialog::DoDataExchange( dx );
}

BOOL cObjectScaleDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 크기 배율
	mScaleEdit.Create( 0.1f, 10.0f, 1.0f, 0.01f, this, IDC_EDIT_OSCALE );
	return TRUE;
}
