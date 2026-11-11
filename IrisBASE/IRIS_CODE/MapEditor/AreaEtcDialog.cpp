#include "stdafx.h"
#include "AreaEtcDialog.h"

#include "resource.h"
#include "AreaEditor.h"

IMPLEMENT_DYNAMIC(cAreaEtcDialog, CDialog)

BEGIN_MESSAGE_MAP(cAreaEtcDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_AETC_GROUP_INDEX, OnClickedUserIndex)
END_MESSAGE_MAP()

cAreaEtcDialog::cAreaEtcDialog()
{
}

cAreaEtcDialog::~cAreaEtcDialog()
{
}

void cAreaEtcDialog::Update( unsigned int groupIndex )//, const cString& soundName, const cString& markName )
{
	cString str;
	str.Format( "%d", groupIndex );
	GetDlgItem( IDC_EDIT_AETC_GROUP_INDEX )->SetWindowText( str.Cstr() );
}

void cAreaEtcDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

}

BOOL cAreaEtcDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

void cAreaEtcDialog::OnClickedUserIndex()
{
	char buffer[64];
	GetDlgItem( IDC_EDIT_AETC_GROUP_INDEX )->GetWindowText( buffer, 64 );
	buffer[63] = 0;
	int groupIndex = ::atoi( buffer );

	AREAEDIT->SetAreaGroupIndex( groupIndex );
}
