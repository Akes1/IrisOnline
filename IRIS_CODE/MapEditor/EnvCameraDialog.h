/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.09
* 내    용 : 환경 카메라 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/NumericEdit.h"

class cAreaFogDialog;

/// 환경 카메라 다이얼로그
class cEnvCameraDialog : public CDialog
{
	DECLARE_DYNAMIC(cEnvCameraDialog)

public:
	cEnvCameraDialog();
	virtual ~cEnvCameraDialog();

	void Update( float farDist );

	float GetFarDistance() const;

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnChangedFarDist( WPARAM id, LPARAM );

private:
	/// 숫자 에디트
	cNumericEdit mFarDist;
};

inline
float cEnvCameraDialog::GetFarDistance() const
{
	return mFarDist.GetValue() * 100.0f;
}
