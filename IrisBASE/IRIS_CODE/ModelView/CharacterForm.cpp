// CharacterForm.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "CharacterForm.h"

#include "Resource.h"

#include "RollupWindow.h"

#include "CharModelDlg.h"
#include "CharAniDlg.h"
#include "CharDramaDlg.h"

#include "SceneManager.h"
#include ".\characterform.h"

// cCharacterForm

IMPLEMENT_DYNCREATE(cCharacterForm, CFormView)

BEGIN_MESSAGE_MAP(cCharacterForm, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cCharacterForm* cCharacterForm::mSingleton = 0;


cCharacterForm::cCharacterForm()
: CFormView(IDD_DIALOG_CHARACTER )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mRollupWnd = 0;

	mpModelDlg = 0;
	mpAniDlg = 0;
	mpDramaDlg = 0;
}

cCharacterForm::~cCharacterForm()
{
	mSingleton = 0;
	delete mRollupWnd;
}

// cCharacterForm 진단입니다.

#ifdef _DEBUG
void cCharacterForm::AssertValid() const
{
	CFormView::AssertValid();
}

void cCharacterForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


int cCharacterForm::OnCreate( LPCREATESTRUCT cs )
{
	if( CFormView::OnCreate( cs ) == -1 )
		return -1;

	/// 두루마리 윈도우를 생성
	mRollupWnd = new cRollupWindow;
	mRollupWnd->Create( WS_VISIBLE | WS_CHILD, CRect(1, 1, 231, 600), this, ID_ROLL1 );

	mpModelDlg = new cCharModelDlg;
	mpModelDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_CHAR_MODEL), mRollupWnd );
	mRollupWnd->InsertPage( "Model Info", mpModelDlg );

	mpAniDlg = new cCharAniDlg;
	mpAniDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_CHAR_ANI), mRollupWnd );
	mRollupWnd->InsertPage( "Animation Info", mpAniDlg );

	mpDramaDlg = new cCharDramaDlg;
	mpDramaDlg->Create( MAKEINTRESOURCE(IDD_DIALOG_CHAR_DRAMA), mRollupWnd );
	mRollupWnd->InsertPage( "Drama Info", mpDramaDlg );

	mRollupWnd->ExpandAllPages();

	return 0;
}

// cCharacterForm 메시지 처리기입니다.
void cCharacterForm::OnSize( UINT type, int cx, int cy )
{
	CFormView::OnSize( type, cx, cy );

	if( mRollupWnd )
		mRollupWnd->Resize( type, cx, cy );
}

void cCharacterForm::Init()
{
	mpModelDlg->OpenData();
	mpAniDlg->OpenData();
	mpDramaDlg->OpenData();

	SCENEMAN->SetDramaProcess( true );

}

void cCharacterForm::UpdateAnimationList( unsigned char race, unsigned char gender)
{
	mpAniDlg->UpdateList( race, gender );
}

void cCharacterForm::UpdateDrama()
{
	mpDramaDlg->OpenData();
}

unsigned char cCharacterForm::GetCurrentRace()
{
	return mpModelDlg->GetRace();
}

unsigned char cCharacterForm::GetCurrentGender()
{
	return mpModelDlg->GetGender();
}

unsigned char cCharacterForm::GetCurrentWeaponState()
{
	return mpModelDlg->GetWeaponState();
}