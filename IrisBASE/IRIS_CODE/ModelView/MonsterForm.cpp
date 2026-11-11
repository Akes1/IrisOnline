// MonsterForm.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MonsterForm.h"

#include "resource.h"

#include "RollupWindow.h"

#include "MonModelDlg.h"
#include "MonAniDlg.h"
#include "MonDramaDlg.h"

#include "SceneManager.h"
#include ".\monsterform.h"

// cMonsterForm

IMPLEMENT_DYNCREATE(cMonsterForm, CFormView)

BEGIN_MESSAGE_MAP(cMonsterForm, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cMonsterForm* cMonsterForm::mSingleton = 0;


cMonsterForm::cMonsterForm()
: CFormView(IDD_DIALOG_MONSTER)
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mRollupWnd = 0;

	mpModelDlg = 0;
	mpAniDlg = 0;
	mpDramaDlg = 0;

	mBaseModelIndex = 0;
}

cMonsterForm::~cMonsterForm()
{
	mSingleton = 0;
	delete mRollupWnd;
}

// cMonsterForm 진단입니다.

#ifdef _DEBUG
void cMonsterForm::AssertValid() const
{
	CFormView::AssertValid();
}

void cMonsterForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

int cMonsterForm::OnCreate( LPCREATESTRUCT cs )
{
	if( CFormView::OnCreate( cs ) == -1 )
		return -1;

	/// 두루마리 윈도우를 생성
	mRollupWnd = new cRollupWindow;
	mRollupWnd->Create( WS_VISIBLE | WS_CHILD, CRect(1, 1, 231, 600), this, ID_ROLL2 );

	mpModelDlg = new cMonModelDlg;
	mpModelDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_MON_MODEL), mRollupWnd );
	mRollupWnd->InsertPage( "Model Info", mpModelDlg );

	mpAniDlg = new cMonAniDlg;
	mpAniDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_MON_ANI), mRollupWnd );
	mRollupWnd->InsertPage( "Animation Info", mpAniDlg );

	mpDramaDlg = new cMonDramaDlg;
	mpDramaDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_MON_DRAMA), mRollupWnd );
	mRollupWnd->InsertPage( "Drama Info", mpDramaDlg );

	mRollupWnd->ExpandAllPages();

	return 0;
}

// cMonsterForm 메시지 처리기입니다.
void cMonsterForm::OnSize( UINT type, int cx, int cy )
{
	CFormView::OnSize( type, cx, cy );

	if( mRollupWnd )
		mRollupWnd->Resize( type, cx, cy );
}

void cMonsterForm::Init()
{
	mBaseModelIndex = mpModelDlg->OpenData();

	mpAniDlg->OpenData( mBaseModelIndex );
	mpDramaDlg->OpenData( GetCurrentMonsterIdx() );

	SCENEMAN->SetDramaProcess( true );

}

unsigned int cMonsterForm::UpdateAnimationList( unsigned long modelIdx )
{
	return mpAniDlg->UpdateList( modelIdx );
}

void cMonsterForm::UpdateDrama()
{
	mpDramaDlg->OpenData( GetCurrentMonsterIdx() );
}

unsigned long cMonsterForm::GetCurrentMonsterIdx()
{
	return mpModelDlg->GetMonsterIndex();
}
