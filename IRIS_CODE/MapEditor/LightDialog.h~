/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 조명 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cLightTransformDialog;
class cLightPropertyDialog;
class cLightInfoDialog;

/// 조명 다이얼로그
class cLightDialog : public CDialog
{
	DECLARE_DYNAMIC(cLightDialog)

public:
	cLightDialog();
	virtual ~cLightDialog();

	/// 자식 다이얼로그를 리턴
	cLightTransformDialog* GetTransformDialog() const;
	cLightPropertyDialog* GetPropertyDialog() const;
	cLightInfoDialog* GetInfoDialog() const;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	/// 자식 다이얼로그
	cLightTransformDialog* mTransformDialog;
	cLightPropertyDialog* mPropertyDialog;
	cLightInfoDialog* mInfoDialog;
};

inline
cLightTransformDialog* cLightDialog::GetTransformDialog() const
{
	return mTransformDialog;
}

inline
cLightPropertyDialog* cLightDialog::GetPropertyDialog() const
{
	return mPropertyDialog;
}

inline
cLightInfoDialog* cLightDialog::GetInfoDialog() const
{
	return mInfoDialog;
}
