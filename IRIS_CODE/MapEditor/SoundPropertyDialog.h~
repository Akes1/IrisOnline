/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.08
* 내    용 : 사운드 속성 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Resource.h"
#include "UI/NumericEdit.h"

class cSoundSceneNode;

/// 사운드 속성 다이얼로그
class cSoundPropertyDialog : public CDialog
{
	DECLARE_DYNAMIC(cSoundPropertyDialog)

public:
	cSoundPropertyDialog();
	virtual ~cSoundPropertyDialog();

	/// 사운드 정보를 설정
	void UpdateSound( const char* name, unsigned int loopCount, float interval, float volume );

	/// 활성화 여부를 설정
	void SetEnabled( bool enabled );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnChangedValue( WPARAM id, LPARAM );

private:
	/// 반복 횟수
	cNumericEdit mLoopCountEdit;

	/// 재생 간격
	cNumericEdit mIntervalEdit;

	/// 음량
	cNumericEdit mVolumeRatioEdit;
};

inline
void cSoundPropertyDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mLoopCountEdit.SetEnabled( true );
		mIntervalEdit.SetEnabled( true );
		mVolumeRatioEdit.SetEnabled( true );
	}
	else
	{
		GetDlgItem(IDC_EDIT_SPROP_NAME)->SetWindowText( "" );

		mLoopCountEdit.SetValue( 0.0f );
		mLoopCountEdit.SetEnabled( false );

		mIntervalEdit.SetValue( 0.0f );
		mIntervalEdit.SetEnabled( false );

		mVolumeRatioEdit.SetValue( 0.0f );
		mVolumeRatioEdit.SetEnabled( false );
	}
}
