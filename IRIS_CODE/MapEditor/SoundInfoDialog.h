/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.02
* 내    용 : 사운드 정보 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 사운드 정보 다이얼로그
class cSoundInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(cSoundInfoDialog)

public:
	cSoundInfoDialog();
	virtual ~cSoundInfoDialog();

	void UpdateNumSounds( unsigned int numSounds );

protected:
	virtual void DoDataExchange( CDataExchange* dx );

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
};
