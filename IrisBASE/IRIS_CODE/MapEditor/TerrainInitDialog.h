/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.06
* 내    용 : 지형 생성 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 지형 생성 다이얼로그
class cTerrainInitDialog : public CDialog
{
	DECLARE_DYNAMIC(cTerrainInitDialog)

public:
	cTerrainInitDialog( CWnd* parent = 0 );
	virtual ~cTerrainInitDialog();

	int GetResolution() const;
	int GetMetersPerVertex() const;

protected:
	void ChangeTerrainSize();

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

public:
	afx_msg void OnSelchangeResolution();
	DECLARE_MESSAGE_MAP()

private:
	/// 해상도
	int mResolution;

	/// 정점당 미터수
	int mMetersPerVertex;
};

inline
int cTerrainInitDialog::GetResolution() const
{
	return mResolution;
}

inline
int cTerrainInitDialog::GetMetersPerVertex() const
{
	return mMetersPerVertex;
}
