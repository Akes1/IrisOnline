// NpcForm.cpp : 구현 파일입니다.
//
#include "stdafx.h"
#include "NpcForm.h"

#include "resource.h"

#include "RollupWindow.h"

#include "NpcModelDlg.h"
#include "NpcAniDlg.h"

#include "SceneManager.h"
#include ".\npcform.h"

// cNpcForm

IMPLEMENT_DYNCREATE(cNpcForm, CFormView)

BEGIN_MESSAGE_MAP(cNpcForm, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cNpcForm* cNpcForm::mSingleton = 0;

cNpcForm::cNpcForm()
: CFormView(IDD_DIALOG_NPC)
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mRollupWnd = 0;

	mpModelDlg = 0;
	mpAniDlg = 0;

	mBaseModelIndex = 0;
}

cNpcForm::~cNpcForm()
{
	mSingleton = 0;

	delete mRollupWnd;
}

// cNpcForm 진단입니다.

#ifdef _DEBUG
void cNpcForm::AssertValid() const
{
	CFormView::AssertValid();
}

void cNpcForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


int cNpcForm::OnCreate( LPCREATESTRUCT cs )
{
	if( CFormView::OnCreate( cs ) == -1 )
		return -1;

	/// 두루마리 윈도우를 생성
	mRollupWnd = new cRollupWindow;
	mRollupWnd->Create( WS_VISIBLE | WS_CHILD, CRect(1, 1, 231, 600), this, ID_ROLL3 );

	mpModelDlg = new cNpcModelDlg;
	mpModelDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_NPC_MODEL), mRollupWnd );
	mRollupWnd->InsertPage( "Model Info", mpModelDlg );

	mpAniDlg = new cNpcAniDlg;
	mpAniDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_NPC_ANI), mRollupWnd );
	mRollupWnd->InsertPage( "Animation Info", mpAniDlg );

	mRollupWnd->ExpandAllPages();

	return 0;
}
// cNpcForm 메시지 처리기입니다.

void cNpcForm::OnSize( UINT type, int cx, int cy )
{
	CFormView::OnSize( type, cx, cy );

	if( mRollupWnd )
		mRollupWnd->Resize( type, cx, cy );
}

void cNpcForm::Init()
{
	mBaseModelIndex = mpModelDlg->OpenData();
	mpAniDlg->OpenData( mBaseModelIndex );

	SCENEMAN->SetDramaProcess( false );

}

unsigned int cNpcForm::UpdateAnimationList( unsigned long modelIdx )
{
	return mpAniDlg->UpdateList( modelIdx );
}
