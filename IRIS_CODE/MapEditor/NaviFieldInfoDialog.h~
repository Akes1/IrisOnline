/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.22
* 내    용 : 네비필드 정보 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 네비필드 정보 다이얼로그
class cNaviFieldInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(cNaviFieldInfoDialog)

public:
	cNaviFieldInfoDialog();
	virtual ~cNaviFieldInfoDialog();

	/// 갱신
	void UpdateNaviField( unsigned int resoluton, unsigned int leafResolution, float metersPerVertex, unsigned int unitsPerMeter );

protected:
	virtual void DoDataExchange( CDataExchange* dx );

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
};
