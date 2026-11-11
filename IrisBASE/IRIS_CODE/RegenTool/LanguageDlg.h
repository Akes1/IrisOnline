/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.10
* 내    용 : 맵 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Resource.h"

/// 맵 다이얼로그
class cLanguageDlg : public CDialog
{
	DECLARE_DYNAMIC(cLanguageDlg)

public:
	cLanguageDlg();
	~cLanguageDlg();

protected:
	virtual void DoDataExchange(CDataExchange* dx);

	BOOL OnInitDialog();

	virtual void OnCancel();

protected:
	DECLARE_MESSAGE_MAP()

	CComboBox mCombo;
};
