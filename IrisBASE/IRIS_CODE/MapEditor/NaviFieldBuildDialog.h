/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.06
* 내    용 : 네비필드 건축 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/Button.h"

/// 네비필드 건축 다이얼로그
class cNaviFieldBuildDialog : public CDialog
{
	DECLARE_DYNAMIC(cNaviFieldBuildDialog)

public:
	cNaviFieldBuildDialog();
	virtual ~cNaviFieldBuildDialog();

protected:
	void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedSyncAllButton();

private:
	/// 버튼
	cButton mSyncAllButton;
};
