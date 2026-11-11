/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.04.30
* 내    용 : 지형 스케일 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 지형 스케일 다이얼로그
class cTerrainScaleZDialog : public CDialog
{
	DECLARE_DYNAMIC(cTerrainScaleZDialog)

public:
	cTerrainScaleZDialog( CWnd* parent = 0 );
	virtual ~cTerrainScaleZDialog();

	float GetScale() const;

protected:
	void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSelchangeScale();

private:
	/// 스케일
	float mScale;
};

inline
float cTerrainScaleZDialog::GetScale() const
{
	return mScale;
}
