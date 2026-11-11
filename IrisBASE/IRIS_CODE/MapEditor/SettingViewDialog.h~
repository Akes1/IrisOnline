/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.02
* 내    용 : 뷰 설정 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Resource.h"

/// 뷰 설정 다이얼로그
class cSettingViewDialog : public CDialog
{
	DECLARE_DYNAMIC(cSettingViewDialog)

public:
	cSettingViewDialog();
	virtual ~cSettingViewDialog();

	void ToggleScene();
	void ToggleTerrainSelGroup();

	bool IsLodChecked() const;
	bool IsTerrainChecked() const;
	bool IsTerrainSelGroupChecked() const;
	bool IsSceneChecked() const;
	bool IsSceneSelGroupChecked() const;
	bool IsLightChecked() const;
	bool IsSoundChecked() const;
	bool IsAreaChecked() const;
	bool IsNaviMeshChecked() const;
	bool IsNaviFieldChecked() const;
	bool IsHUDChecked() const;

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckedButton( UINT id );
};

inline
bool cSettingViewDialog::IsLodChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_LOD ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsTerrainChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_TERRAIN ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsTerrainSelGroupChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_TERRAIN_SELGROUP ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsSceneChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SCENE ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsLightChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_LIGHT ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsSoundChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SOUND ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsAreaChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_AREA ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsSceneSelGroupChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_SCENE_SELGROUP ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsNaviMeshChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_NAVIMESH ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsNaviFieldChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_NAVIFIELD ))->GetCheck() != 0;
}

inline
bool cSettingViewDialog::IsHUDChecked() const
{
	return ((CButton*)GetDlgItem( IDC_CHECK_SETVIEW_HUD ))->GetCheck() != 0;
}
