/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.02
* 내    용 : 컨트롤 설정 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 컨트롤 설정 다이얼로그
class cSettingControlDialog : public CDialog
{
	DECLARE_DYNAMIC(cSettingControlDialog)

public:
	cSettingControlDialog();
	virtual ~cSettingControlDialog();

	int GetMouseWheel() const;

protected:
	virtual void DoDataExchange(CDataExchange* dx);
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );
};
