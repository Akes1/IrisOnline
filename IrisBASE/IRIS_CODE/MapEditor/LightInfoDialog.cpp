#include "stdafx.h"
#include "LightInfoDialog.h"

#include "resource.h"

IMPLEMENT_DYNAMIC(cLightInfoDialog, CDialog)

BEGIN_MESSAGE_MAP(cLightInfoDialog, CDialog)
END_MESSAGE_MAP()

cLightInfoDialog::cLightInfoDialog()
{
}

cLightInfoDialog::~cLightInfoDialog()
{
}

void cLightInfoDialog::UpdateNumLights( unsigned int numLights )
{
	/// Á¶¸í ¼ö
	SetDlgItemInt( IDC_EDIT_LINFO_NUM_LIGHTS, numLights );
}

void cLightInfoDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}
