/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.17
* 내    용 : 지형 옵션 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

enum eTerrainViewMode;

/// 지형 옵션 다이얼로그
class cTerrainOptionDialog : public CDialog
{
	DECLARE_DYNAMIC(cTerrainOptionDialog)

public:
	cTerrainOptionDialog();
	virtual ~cTerrainOptionDialog();

	void ToggleWireTextured();
	eTerrainViewMode GetViewMode() const;

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckedWireframe();
	afx_msg void OnCheckedSolid();
	afx_msg void OnCheckedTextured();
	afx_msg void OnCheckedWireTextured();

private:
	/// 뷰 모드
	eTerrainViewMode mViewMode;
};

inline
eTerrainViewMode cTerrainOptionDialog::GetViewMode() const
{
	return mViewMode;
}
