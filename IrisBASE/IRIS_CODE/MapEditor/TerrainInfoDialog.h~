/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.22
* 내    용 : 지형 정보 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 지형 정보 다이얼로그
class cTerrainInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(cTerrainInfoDialog)

public:
	cTerrainInfoDialog();
	virtual ~cTerrainInfoDialog();

	/// 갱신
	void Update( unsigned int resoluton, unsigned int leafResolution, float metersPerVertex, unsigned int unitsPerMeter );

protected:
	virtual void DoDataExchange( CDataExchange* dx );

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
};
