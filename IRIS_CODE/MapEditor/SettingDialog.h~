/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 설정 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cSettingViewDialog;
class cSettingControlDialog;

/// 설정 다이얼로그
class cSettingDialog : public CDialog
{
	DECLARE_DYNAMIC(cSettingDialog)

public:
	cSettingDialog();
	virtual ~cSettingDialog();

	/// 자식 다이얼로그를 리턴
	cSettingViewDialog* GetViewDialog() const;
	cSettingControlDialog* GetControlDialog() const;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

	virtual void OnOK() {}
	virtual void OnCancel() {}

private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	/// 자식 다이얼로그
	cSettingViewDialog* mViewDialog;
	cSettingControlDialog* mControlDialog;
};

inline
cSettingViewDialog* cSettingDialog::GetViewDialog() const
{
	return mViewDialog;
}

inline
cSettingControlDialog* cSettingDialog::GetControlDialog() const
{
	return mControlDialog;
}
