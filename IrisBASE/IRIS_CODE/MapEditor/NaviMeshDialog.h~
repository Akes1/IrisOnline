/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 네비메시 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/RollupWindow.h"

class cNaviMeshBuildDialog;

/// 네비메시 다이얼로그
class cNaviMeshDialog : public CDialog
{
	DECLARE_DYNAMIC(cNaviMeshDialog)

public:
	cNaviMeshDialog();
	virtual ~cNaviMeshDialog();

	/// 자식 다이얼로그를 리턴
	cNaviMeshBuildDialog* GetBuildDialog() const;

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
	cNaviMeshBuildDialog* mBuildDialog;
};

inline
cNaviMeshBuildDialog* cNaviMeshDialog::GetBuildDialog() const
{
	return mBuildDialog;
}
