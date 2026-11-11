/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.19
* 내    용 : 환경 하늘 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/ColorBox.h"
#include "UI/NumericEdit.h"

/// 환경 하늘 다이얼로그
class cEnvSkyDialog : public CDialog
{
	DECLARE_DYNAMIC(cEnvSkyDialog)

public:
	cEnvSkyDialog();
	virtual ~cEnvSkyDialog();

	void Update( const cString& fileName );
	void Update( const cString& fileName, bool enabled, float scale ); //, float height );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedEnabled();
	afx_msg void OnClickedOpen();
	afx_msg LRESULT OnChangedNumber( WPARAM id, LPARAM );
	//afx_msg LRESULT OnChangedHeight( WPARAM id, LPARAM );

private:
	/// 숫자 에디트
	cNumericEdit mScale;
	//cNumericEdit mHeight;
};
