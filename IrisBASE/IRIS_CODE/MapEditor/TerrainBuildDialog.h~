/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.06
* 내    용 : 지형 건축 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/Dialog.h"
#include "UI/Button.h"
#include "UI/CheckButton.h"
#include "UI/CheckButtonGroup.h"

#include "Resource.h"

/// 지형 건축 다이얼로그
class cTerrainBuildDialog : public cDialog
{
	DECLARE_DYNAMIC(cTerrainBuildDialog)

public:
	cTerrainBuildDialog();
	virtual ~cTerrainBuildDialog();

	/// 갱신
	void UpdateInnerRadius( float radius );
	void UpdateOuterRadius( float radius );

	/// 버튼
	void SetCheckedButton( int id );
	unsigned int GetCheckedButton() const;

	/// 픽킹 위치
	void SetPickPos( const NiPoint3& pos );

	/// 반지름 (단위: meter)
	float GetInnerRadius() const;
	float GetOuterRadius() const;

	/// 강도 (단위: percent)
	unsigned int GetStrength() const;

	/// 보간 비율 (단위: percent)
	unsigned int GetSmoothRatio() const;

protected:
	void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedButton();
	afx_msg void OnClickedSyncAllButton();
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );

private:
	/// 버튼
	cCheckButton mRaiseButton;
	cCheckButton mLowerButton;
	cCheckButton mFlattenButton;
	cCheckButton mSmoothButton;
	cCheckButton mSyncButton;
	cCheckButton mSyncToPickPosButton;
	cCheckButtonGroup mGroup;

	cButton mSyncAllButton;
};

inline
unsigned int cTerrainBuildDialog::GetCheckedButton() const
{
	cCheckButton* btn = mGroup.GetCheckedButton();

	if( btn == 0 )
		return 0;
	else
		return btn->GetDlgCtrlID();
}

inline
float cTerrainBuildDialog::GetInnerRadius() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_INNER_RADIUS );
	return slider->GetPos() / 10.0f;
}

inline
float cTerrainBuildDialog::GetOuterRadius() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_OUTER_RADIUS );
	return slider->GetPos() / 10.0f;
}

inline
unsigned int cTerrainBuildDialog::GetStrength() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_STRENGTH );
	return slider->GetPos();
}

inline
unsigned int cTerrainBuildDialog::GetSmoothRatio() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TBUILD_SMOOTH_RATIO );
	return slider->GetPos();
}
