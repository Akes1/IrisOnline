/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.05
* 내    용 : 네비필드 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cNaviFieldBuildDialog;
class cNaviFieldPaintDialog;
class cNaviFieldInfoDialog;

/// 네비필드 다이얼로그
class cNaviFieldDialog : public CDialog
{
	DECLARE_DYNAMIC(cNaviFieldDialog)

public:
	cNaviFieldDialog();
	virtual ~cNaviFieldDialog();

	/// 자식 다이얼로그를 리턴
	cNaviFieldBuildDialog* GetBuildDialog() const;
	cNaviFieldPaintDialog* GetPaintDialog() const;
	cNaviFieldInfoDialog* GetInfoDialog() const;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnSize( UINT type, int cx, int cy );

	virtual void OnOK() {}
	virtual void OnCancel() {}

private:
	/// 두루마리 윈도우
	cRollupWindow mRollupWnd;

	/// 자식 다이얼로그
	cNaviFieldBuildDialog* mBuildDialog;
	cNaviFieldPaintDialog* mPaintDialog;
	cNaviFieldInfoDialog* mInfoDialog;
};

inline
cNaviFieldBuildDialog* cNaviFieldDialog::GetBuildDialog() const
{
	return mBuildDialog;
}

inline
cNaviFieldPaintDialog* cNaviFieldDialog::GetPaintDialog() const
{
	return mPaintDialog;
}

inline
cNaviFieldInfoDialog* cNaviFieldDialog::GetInfoDialog() const
{
	return mInfoDialog;
}
