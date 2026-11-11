/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.08
* 내    용 : 오브젝트 속성 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Resource.h"
#include "UI/ColorBox.h"
#include "UI/Button.h"

class cObjectGroup;

/// 오브젝트 속성 다이얼로그
class cObjectPropertyDialog : public CDialog
{
	DECLARE_DYNAMIC(cObjectPropertyDialog)

public:
	cObjectPropertyDialog();
	virtual ~cObjectPropertyDialog();

	/// 오브젝트 정보를 설정
	void UpdateObject( cObjectGroup* obj );

	/// 활성화 여부를 설정
	void SetEnabled( bool enabled );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckedVisibleLevel0();
	afx_msg void OnCheckedVisibleLevel1();
	afx_msg void OnCheckedVisibleLevel2();
	afx_msg void OnCheckedVisibleLevel3();
	afx_msg void OnClickedVisibleLevelSibling();
	afx_msg void OnCheckedOccludeLevel0();
	afx_msg void OnCheckedOccludeLevel1();
	afx_msg void OnCheckedOccludeLevel2();
	afx_msg void OnClickedOccludeLevelSibling();
	afx_msg void OnCheckedFogApplied();
	afx_msg void OnClickedFogAppliedSibling();
	afx_msg void OnCheckedMaterialApplied();
	afx_msg void OnClickedMaterialAppliedSibling();
	afx_msg void OnClickedAmbient();
	afx_msg void OnClickedDiffuse();
	afx_msg void OnClickedEmissive();
	afx_msg void OnClickedAmbientSibling();
	afx_msg void OnClickedDiffuseSibling();
	afx_msg void OnClickedEmissiveSibling();

private:
	/// 색 상자
	cColorBox mAmbientBox;
	cColorBox mDiffuseBox;
	cColorBox mEmissiveBox;

	///
	cButton mVisibleLevelSibling;
	cButton mOccludeLevelSibling;
	cButton mFogAppliedSibling;
	cButton mMaterialAppliedSibling;
	cButton mAmbientSibling;
	cButton mDiffuseSibling;
	cButton mEmissiveSibling;
};

inline
void cObjectPropertyDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL0)->EnableWindow( TRUE );
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL1)->EnableWindow( TRUE );
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL2)->EnableWindow( TRUE );
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL3)->EnableWindow( TRUE );
		GetDlgItem(IDC_BUTTON_OPROP_VISIBLE_LEVEL_SIBLING)->EnableWindow( TRUE );
		GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL0)->EnableWindow( TRUE );
		GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL1)->EnableWindow( TRUE );
		GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL2)->EnableWindow( TRUE );
		GetDlgItem(IDC_BUTTON_OPROP_OCCLUDE_LEVEL_SIBLING)->EnableWindow( TRUE );
		GetDlgItem(IDC_CHECK_OPROP_FOG_APPLIED)->EnableWindow( TRUE );
		GetDlgItem(IDC_BUTTON_OPROP_FOG_APPLIED_SIBLING)->EnableWindow( TRUE );
		GetDlgItem(IDC_CHECK_OPROP_MATERIAL_APPLIED)->EnableWindow( TRUE );
		GetDlgItem(IDC_BUTTON_OPROP_MATERIAL_APPLIED_SIBLING)->EnableWindow( TRUE );
		GetDlgItem(IDC_BUTTON_OPROP_AMBIENT_SIBLING)->EnableWindow( TRUE );
		GetDlgItem(IDC_BUTTON_OPROP_DIFFUSE_SIBLING)->EnableWindow( TRUE );
		GetDlgItem(IDC_BUTTON_OPROP_EMISSIVE_SIBLING)->EnableWindow( TRUE );

		mAmbientBox.EnableWindow( TRUE );
		mDiffuseBox.EnableWindow( TRUE );
		mEmissiveBox.EnableWindow( TRUE );
	}
	else
	{
		GetDlgItem(IDC_EDIT_OPROP_NAME)->SetWindowText( "" );
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL0)->EnableWindow( FALSE );
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL1)->EnableWindow( FALSE );
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL2)->EnableWindow( FALSE );
		GetDlgItem(IDC_RADIO_OPROP_VISIBLE_LEVEL3)->EnableWindow( FALSE );
		GetDlgItem(IDC_BUTTON_OPROP_VISIBLE_LEVEL_SIBLING)->EnableWindow( FALSE );
		GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL0)->EnableWindow( FALSE );
		GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL1)->EnableWindow( FALSE );
		GetDlgItem(IDC_RADIO_OPROP_OCCLUDE_LEVEL2)->EnableWindow( FALSE );
		GetDlgItem(IDC_BUTTON_OPROP_OCCLUDE_LEVEL_SIBLING)->EnableWindow( FALSE );
		GetDlgItem(IDC_CHECK_OPROP_FOG_APPLIED)->EnableWindow( FALSE );
		GetDlgItem(IDC_BUTTON_OPROP_FOG_APPLIED_SIBLING)->EnableWindow( FALSE );
		GetDlgItem(IDC_CHECK_OPROP_MATERIAL_APPLIED)->EnableWindow( FALSE );
		GetDlgItem(IDC_BUTTON_OPROP_MATERIAL_APPLIED_SIBLING)->EnableWindow( FALSE );
		GetDlgItem(IDC_BUTTON_OPROP_AMBIENT_SIBLING)->EnableWindow( FALSE );
		GetDlgItem(IDC_BUTTON_OPROP_DIFFUSE_SIBLING)->EnableWindow( FALSE );
		GetDlgItem(IDC_BUTTON_OPROP_EMISSIVE_SIBLING)->EnableWindow( FALSE );

		mAmbientBox.SetColor( 0 );
		mDiffuseBox.SetColor( 0 );
		mEmissiveBox.SetColor( 0 );
		mAmbientBox.EnableWindow( FALSE );
		mDiffuseBox.EnableWindow( FALSE );
		mEmissiveBox.EnableWindow( FALSE );
	}
}
