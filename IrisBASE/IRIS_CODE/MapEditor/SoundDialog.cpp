#include "stdafx.h"
#include "SoundDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "SoundTransformDialog.h"
#include "SoundPropertyDialog.h"
#include "SoundInfoDialog.h"

IMPLEMENT_DYNAMIC(cSoundDialog, CDialog)

BEGIN_MESSAGE_MAP(cSoundDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cSoundDialog::cSoundDialog()
: mTransformDialog( 0 )
, mPropertyDialog( 0 )
, mInfoDialog( 0 )
{
}

cSoundDialog::~cSoundDialog()
{
}

int cSoundDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 사운드 배치 페이지 추가
	mTransformDialog = new cSoundTransformDialog;
	mTransformDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_SOUND_TRANSFORM), &mRollupWnd );
	mRollupWnd.InsertPage( "Transform", mTransformDialog );

	/// 사운드 속성 페이지 추가
	mPropertyDialog = new cSoundPropertyDialog;
	mPropertyDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_SOUND_PROPERTY), &mRollupWnd );
	mRollupWnd.InsertPage( "Property", mPropertyDialog );

	/// 장면 정보 페이지 추가
	mInfoDialog = new cSoundInfoDialog;
	mInfoDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_SOUND_INFO), &mRollupWnd );
	mRollupWnd.InsertPage( "Info", mInfoDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cSoundDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_SOUND, false );
	CDialog::OnClose();
}

void cSoundDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
