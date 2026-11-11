/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.02.13
* 내    용 : 레벨 거리 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 레벨 거리 다이얼로그
class cEnvLevelDistanceDialog : public CDialog
{
	DECLARE_DYNAMIC(cEnvLevelDistanceDialog)

public:
	cEnvLevelDistanceDialog();
	virtual ~cEnvLevelDistanceDialog();

	void Update( unsigned int dist0, unsigned int dist1, unsigned int dist2 );

protected:
	virtual void DoDataExchange(CDataExchange* dx);
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );
};
