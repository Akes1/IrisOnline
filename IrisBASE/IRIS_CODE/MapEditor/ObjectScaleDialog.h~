/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.04.30
* 내    용 : 오브젝트 스케일 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/NumericEdit.h"

/// 오브젝트 스케일 다이얼로그
class cObjectScaleDialog : public CDialog
{
	DECLARE_DYNAMIC(cObjectScaleDialog)

public:
	cObjectScaleDialog( CWnd* parent = 0 );
	virtual ~cObjectScaleDialog();

	float GetScale() const;

protected:
	void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

private:
	/// 스케일
	cNumericEdit mScaleEdit;
};

inline
float cObjectScaleDialog::GetScale() const
{
	return mScaleEdit.GetValue();
}
