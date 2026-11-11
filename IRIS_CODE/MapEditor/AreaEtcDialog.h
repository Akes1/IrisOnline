/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.17
* 내    용 : 영역 기타 속성 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 영역 기타 속성 다이얼로그
class cAreaEtcDialog : public CDialog
{
	DECLARE_DYNAMIC(cAreaEtcDialog)

public:
	cAreaEtcDialog();
	virtual ~cAreaEtcDialog();

	void Update( unsigned int userIndex );//, const cString& soundName );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedUserIndex();
};
