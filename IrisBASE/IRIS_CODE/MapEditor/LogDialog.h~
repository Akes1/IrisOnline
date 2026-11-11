/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.29
* 내    용 : 로그 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 로그 다이얼로그
class cLogDialog : public CDialog
{
	DECLARE_DYNAMIC(cLogDialog)

public:
	cLogDialog(CWnd* parent = 0);
	virtual ~cLogDialog();

	int GetResolution() const;

protected:
	void ChangeDevisionSize();

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

public:
	afx_msg void OnSelchangeResolution();
	DECLARE_MESSAGE_MAP()

private:
	/// 해상도
	int mResolution;
};

inline
int cLogDialog::GetResolution() const
{
	return mResolution;
}
