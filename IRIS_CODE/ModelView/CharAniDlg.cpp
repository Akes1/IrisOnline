// CharAniDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "CharAniDlg.h"

#include "resource.h"

#include "GameResourceManager.h"
#include "ResourceManager.h"
#include "ModelInstance.h"

#include "ModelView.h"
#include ".\Engine\Player_Common.h"
#include "SceneManager.h"
#include "CharacterForm.h"

// cCharAniDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cCharAniDlg, CDialog)

BEGIN_MESSAGE_MAP(cCharAniDlg, CDialog)
	ON_CBN_SELCHANGE( IDC_COMBO_CHAR_ANI, OnCbnSelchangeCombo)
END_MESSAGE_MAP()

cCharAniDlg::cCharAniDlg(CWnd* /*pParent=NULL*/)
{
}

cCharAniDlg::~cCharAniDlg()
{
}

void cCharAniDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_COMBO_CHAR_ANI, mAnimationCombo );
}

void cCharAniDlg::OpenData()
{
	UpdateList( 0, 0 );
}

void cCharAniDlg::UpdateList( unsigned char race, unsigned char gender )
{
	const char* filename = GAMERESOURCEMAN->GetModelFileName( race * eGENDER_MAX + gender );
	cModelInstance* inst = RESOURCEMAN->GetModelByName( filename );
	if( inst == 0 )
	{
		assert(0);
		return;
	}

	mAnimationCombo.ResetContent();
	cModelInstance::cStringMap* map = inst->GetSequenceNameMap();

	cModelInstance::cStringMap::cIterator i = map->Begin();
	cModelInstance::cStringMap::cIterator iend = map->End();
	for( ; i != iend; ++i )
	{
		DWORD seqIdx = (DWORD)(i->mFirst);
		cString str = (cString)(i->mSecond);

		int idx = mAnimationCombo.AddString( str.Cstr() );
		mAnimationCombo.SetItemData( idx, seqIdx );
	}
	mAnimationCombo.SetCurSel( 0 );
}

// cCharAniDlg 메시지 처리기입니다.
void cCharAniDlg::OnCbnSelchangeCombo()
{
	int sel = mAnimationCombo.GetCurSel();
	if( sel < 0 )
		return;

	SCENEMAN->SetDramaProcess( false );
	CHARACTERFORM->UpdateDrama();

	unsigned int seq = (unsigned int)mAnimationCombo.GetItemData( sel );
	VIEW->ChangeAnimation( seq );
}