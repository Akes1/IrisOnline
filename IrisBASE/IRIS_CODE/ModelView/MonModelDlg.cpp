// MonModelDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MonModelDlg.h"

#include "resource.h"
#include ".\monmodeldlg.h"

#include "ModelView.h"
#include "MonsterForm.h"

#include "GameResourceManager.h"
#include "SceneManager.h"

// cMonModelDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cMonModelDlg, CDialog)

BEGIN_MESSAGE_MAP(cMonModelDlg, CDialog)
	ON_CBN_SELCHANGE( IDC_COMBO_MON_MODEL, OnCbnSelchangeCombo)
END_MESSAGE_MAP()

cMonModelDlg::cMonModelDlg(CWnd* /*pParent=NULL*/)
{
	mCurrentMonsterIdx = 0;
}

cMonModelDlg::~cMonModelDlg()
{
}

void cMonModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_COMBO_MON_MODEL, mMonsterModelCombo );
}

unsigned long cMonModelDlg::OpenData()
{
	mMonsterModelCombo.ResetContent();

	cArrayHashMap* map = GAMERESOURCEMAN->GetMonsterList();

	cArrayHashMap::cIterator i = map->Begin();
	cArrayHashMap::cIterator iend = map->End();
	for( ; i != iend; ++i )
	{
		sMonsterInfo* info = (sMonsterInfo*)(i->mSecond);

		int idx = mMonsterModelCombo.AddString( info->mName.Cstr() );
		mMonsterModelCombo.SetItemDataPtr( idx, info );
	}
	mMonsterModelCombo.SetCurSel( 0 );

	sMonsterInfo* p = (sMonsterInfo*)mMonsterModelCombo.GetItemDataPtr( 0 );
	if( p )
	{
		mCurrentMonsterIdx = p->mIndex;
		return p->mModelIndex;
	}

	return 0;
}

// cMonModelDlg 메시지 처리기입니다.

void cMonModelDlg::OnCbnSelchangeCombo()
{
	int sel = mMonsterModelCombo.GetCurSel();
	if( sel < 0 )
		return;

	sMonsterInfo* info = (sMonsterInfo*)mMonsterModelCombo.GetItemDataPtr( sel );
	if( info )
	{
		mCurrentMonsterIdx = info->mIndex;

		VIEW->CreateMonster( info->mIndex, info->mModelIndex );
		MONSTERFORM->UpdateDrama();
	}
	else
	{
		assert(0);
	}

	SCENEMAN->SetDramaProcess( true );
}

