// NpcModelDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "NpcModelDlg.h"

#include "resource.h"

#include "ModelView.h"

#include "GameResourceManager.h"

// cNpcModelDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cNpcModelDlg, CDialog)

BEGIN_MESSAGE_MAP(cNpcModelDlg, CDialog)
	ON_CBN_SELCHANGE( IDC_COMBO_NPC_MODEL, OnCbnSelchangeCombo)
END_MESSAGE_MAP()

cNpcModelDlg::cNpcModelDlg(CWnd* /*pParent=NULL*/)
{
}

cNpcModelDlg::~cNpcModelDlg()
{
}

void cNpcModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_COMBO_NPC_MODEL, mNpcModelCombo );
}

unsigned long cNpcModelDlg::OpenData()
{
	mNpcModelCombo.ResetContent();

	cArrayHashMap* map = GAMERESOURCEMAN->GetNpcList();

	cArrayHashMap::cIterator i = map->Begin();
	cArrayHashMap::cIterator iend = map->End();
	for( ; i != iend; ++i )
	{
		sNpcInfo* info = (sNpcInfo*)(i->mSecond);

		int idx = mNpcModelCombo.AddString( info->mName.Cstr() );
		mNpcModelCombo.SetItemDataPtr( idx, info );
	}
	mNpcModelCombo.SetCurSel( 0 );

	sNpcInfo* p = (sNpcInfo*)mNpcModelCombo.GetItemDataPtr( 0 );
	if( p )
		return p->mModelIndex;

	return 0;
}

// cNpcModelDlg 메시지 처리기입니다.
void cNpcModelDlg::OnCbnSelchangeCombo()
{
	int sel = mNpcModelCombo.GetCurSel();
	if( sel < 0 )
		return;

	sNpcInfo* info = (sNpcInfo*)mNpcModelCombo.GetItemDataPtr( sel );
	if( info )
	{
		VIEW->CreateNpc( info->mModelIndex );
	}
}


