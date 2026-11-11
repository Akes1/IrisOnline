// NpcAniDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "NpcAniDlg.h"

#include "resource.h"

#include "GameResourceManager.h"
#include "ResourceManager.h"
#include "ModelInstance.h"
#include "SceneManager.h"

#include "ModelView.h"
// cNpcAniDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cNpcAniDlg, CDialog)

BEGIN_MESSAGE_MAP(cNpcAniDlg, CDialog)
	ON_CBN_SELCHANGE( IDC_COMBO_NPC_ANI, OnCbnSelchangeCombo)
END_MESSAGE_MAP()

cNpcAniDlg::cNpcAniDlg(CWnd* /*pParent=NULL*/)
{
}

cNpcAniDlg::~cNpcAniDlg()
{
}

void cNpcAniDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_COMBO_NPC_ANI, mAnimationCombo );
}

void cNpcAniDlg::OpenData( unsigned long baseModelIdx )
{
	UpdateList( baseModelIdx );
}

// cNpcAniDlg 메시지 처리기입니다.

unsigned int cNpcAniDlg::UpdateList( unsigned long modelIdx )
{
	const char* filename = GAMERESOURCEMAN->GetModelFileName( modelIdx );
	cModelInstance* inst = RESOURCEMAN->GetModelByName( filename );
	if( inst == 0 )
	{
		assert(0);
		return 0;
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

	return (unsigned int)mAnimationCombo.GetItemData( 0 );
}

void cNpcAniDlg::OnCbnSelchangeCombo()
{
	int sel = mAnimationCombo.GetCurSel();
	if( sel < 0 )
		return;

	unsigned int seq = (unsigned int)mAnimationCombo.GetItemData( sel );
	VIEW->ChangeAnimation( seq );

	SCENEMAN->SetDramaProcess( false );
}
