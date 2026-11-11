/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.02
* 내    용 : 오브젝트 정보 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 오브젝트 정보 다이얼로그
class cObjectInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(cObjectInfoDialog)

public:
	cObjectInfoDialog();
	virtual ~cObjectInfoDialog();

	void UpdateScene( const NiPoint3& center, float minRadius, float maxRadius, unsigned int numObjects );
	void UpdateNumObjects( unsigned int numObjects );

protected:
	virtual void DoDataExchange( CDataExchange* dx );

	virtual void OnOK() {}
	virtual void OnCancel() {}
protected:
	DECLARE_MESSAGE_MAP()
};
