/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.10
* 내    용 : 카메라 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Button.h"
#include "NumericEdit.h"

/// 카메라 다이얼로그
class cCameraDialog : public CDialog
{
	DECLARE_DYNAMIC(cCameraDialog)

public:
	cCameraDialog();
	~cCameraDialog();

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	afx_msg void OnClickedZoomPersp();
	afx_msg void OnClickedZoomOrtho();
	afx_msg LRESULT OnChangedFarDist( WPARAM id, LPARAM );
	DECLARE_MESSAGE_MAP()

private:
	cButton mZoomPerspButton;
	cButton mZoomOrthoButton;
	cNumericEdit mFarDist;
};
