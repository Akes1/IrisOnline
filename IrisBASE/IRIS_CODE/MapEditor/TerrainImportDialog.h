/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.11
* 내    용 : 지형 임포트 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 지형 임포트 다이얼로그
class cTerrainImportDialog : public CDialog
{
	DECLARE_DYNAMIC(cTerrainImportDialog)

public:
	cTerrainImportDialog( CWnd* parent = 0 );
	virtual ~cTerrainImportDialog();

	float GetScale() const;

protected:
	void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );

private:
	/// 배율 [1, 100]
	float mScale;
};

inline
float cTerrainImportDialog::GetScale() const
{
	return mScale;
}
