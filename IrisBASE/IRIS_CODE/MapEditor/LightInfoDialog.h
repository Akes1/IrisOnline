/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.02
* 내    용 : 조명 정보 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 조명 정보 다이얼로그
class cLightInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(cLightInfoDialog)

public:
	cLightInfoDialog();
	virtual ~cLightInfoDialog();

	void UpdateNumLights( unsigned int numLights );

protected:
	virtual void DoDataExchange( CDataExchange* dx );

protected:
	DECLARE_MESSAGE_MAP()
};
