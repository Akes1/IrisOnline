#include "stdafx.h"
#include "SoundInfoDialog.h"

#include "resource.h"

IMPLEMENT_DYNAMIC(cSoundInfoDialog, CDialog)

BEGIN_MESSAGE_MAP(cSoundInfoDialog, CDialog)
END_MESSAGE_MAP()

cSoundInfoDialog::cSoundInfoDialog()
{
}

cSoundInfoDialog::~cSoundInfoDialog()
{
}

void cSoundInfoDialog::UpdateNumSounds( unsigned int numSounds )
{
	/// 사운드 수
	SetDlgItemInt( IDC_EDIT_SINFO_NUM_SOUNDS, numSounds );
}

void cSoundInfoDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}
