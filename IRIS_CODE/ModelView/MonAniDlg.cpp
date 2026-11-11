// MonAniDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MonAniDlg.h"

#include "resource.h"

#include "GameResourceManager.h"
#include "ResourceManager.h"
#include "ModelInstance.h"

#include "ModelView.h"

#include "SceneManager.h"
#include "DynamicSceneNode.h"

// cMonAniDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cMonAniDlg, CDialog)

BEGIN_MESSAGE_MAP(cMonAniDlg, CDialog)
	ON_CBN_SELCHANGE( IDC_COMBO_MON_ANI, OnCbnSelchangeCombo)
END_MESSAGE_MAP()

cMonAniDlg::cMonAniDlg(CWnd* /*pParent=NULL*/)
{
}

cMonAniDlg::~cMonAniDlg()
{
}

void cMonAniDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_COMBO_MON_ANI, mAnimationCombo );
}

void cMonAniDlg::OpenData( unsigned long baseModelIdx )
{
	UpdateList( baseModelIdx );
}

// cMonAniDlg 메시지 처리기입니다.

unsigned int cMonAniDlg::UpdateList( unsigned long modelIdx )
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

void cMonAniDlg::OnCbnSelchangeCombo()
{
	int sel = mAnimationCombo.GetCurSel();
	if( sel < 0 )
		return;

	SCENEMAN->SetDramaProcess( false );

	unsigned int seq = (unsigned int)mAnimationCombo.GetItemData( sel );
	VIEW->ChangeAnimation( seq );
}
