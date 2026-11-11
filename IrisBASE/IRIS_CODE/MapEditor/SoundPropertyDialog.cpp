#include "stdafx.h"
#include "SoundPropertyDialog.h"

#include "SoundEditor.h"

IMPLEMENT_DYNAMIC(cSoundPropertyDialog, CDialog)

BEGIN_MESSAGE_MAP(cSoundPropertyDialog, CDialog)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedValue)
END_MESSAGE_MAP()

cSoundPropertyDialog::cSoundPropertyDialog()
{
}

cSoundPropertyDialog::~cSoundPropertyDialog()
{
}

void cSoundPropertyDialog::UpdateSound( const char* name, unsigned int loopCount, float interval, float gain )
{
	/// 파일 이름을 설정
	GetDlgItem(IDC_EDIT_SPROP_NAME)->SetWindowText( name );

	/// 반복 횟수
	mLoopCountEdit.SetValue( (float)loopCount );

	/// 재생 간격
	mIntervalEdit.SetValue( interval );

	/// 음량
	mVolumeRatioEdit.SetValue( gain );

	SetEnabled( true );
}

void cSoundPropertyDialog::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);
}

BOOL cSoundPropertyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 반복 횟수
	mLoopCountEdit.Create( 0.0f, 100.0f, 0.0f, 1.0f, this, IDC_EDIT_SPROP_LOOP_COUNT );

	/// 재생 간격
	mIntervalEdit.Create( 0.0f, 10.0f, 0.0f, 0.1f, this, IDC_EDIT_SPROP_LOOP_INTERVAL );

	/// 음량
	mVolumeRatioEdit.Create( 0.1f, 1.0f, 1.0f, 0.1f, this, IDC_EDIT_SPROP_VOLUME_RATIO );

	SetEnabled( false );
	return TRUE;
}

LRESULT cSoundPropertyDialog::OnChangedValue( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_EDIT_SPROP_LOOP_COUNT:
		{
			int v = (int)mLoopCountEdit.GetValue();
			SOUNDEDIT->SetSoundLoopCount( v );
		}
		break;

	case IDC_EDIT_SPROP_LOOP_INTERVAL:
		{
			float v = mIntervalEdit.GetValue();
			SOUNDEDIT->SetSoundLoopInterval( v );
		}
		break;

	case IDC_EDIT_SPROP_VOLUME_RATIO:
		{
			float v = mVolumeRatioEdit.GetValue();
			SOUNDEDIT->SetSoundVolumeRatio( v );
		}
		break;
	}
	return 0;
}
