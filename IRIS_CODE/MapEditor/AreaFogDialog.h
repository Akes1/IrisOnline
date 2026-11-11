/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.17
* 내    용 : 영역 안개 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/ColorBox.h"
#include "UI/NumericEdit.h"

class cEnvCameraDialog;

/// 영역 안개 다이얼로그
class cAreaFogDialog : public CDialog
{
	DECLARE_DYNAMIC(cAreaFogDialog)

public:
	cAreaFogDialog();
	virtual ~cAreaFogDialog();

	void Update( bool enabled, const NiColor& color, float depth );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedEnabled();
	afx_msg void OnClickedColorBox();
	afx_msg LRESULT OnChangedDepth( WPARAM id, LPARAM );

private:
	/// 색 상자
	cColorBox mColorBox;

	/// 숫자 에디트
	cNumericEdit mDepth;
};
