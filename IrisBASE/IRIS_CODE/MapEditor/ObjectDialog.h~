/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 오브젝트 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cObjectTransformDialog;
class cObjectPropertyDialog;
class cObjectInfoDialog;

/// 오브젝트 다이얼로그
class cObjectDialog : public CDialog
{
	DECLARE_DYNAMIC(cObjectDialog)

public:
	cObjectDialog();
	virtual ~cObjectDialog();

	/// 자식 다이얼로그를 리턴
	cObjectTransformDialog* GetTransformDialog() const;
	cObjectPropertyDialog* GetPropertyDialog() const;
	cObjectInfoDialog* GetInfoDialog() const;

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
	cObjectTransformDialog* mTransformDialog;
	cObjectPropertyDialog* mPropertyDialog;
	cObjectInfoDialog* mInfoDialog;
};

inline
cObjectTransformDialog* cObjectDialog::GetTransformDialog() const
{
	return mTransformDialog;
}

inline
cObjectPropertyDialog* cObjectDialog::GetPropertyDialog() const
{
	return mPropertyDialog;
}

inline
cObjectInfoDialog* cObjectDialog::GetInfoDialog() const
{
	return mInfoDialog;
}
