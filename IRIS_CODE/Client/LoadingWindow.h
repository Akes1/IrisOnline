/* ==========================================================================
* 파    일 : LoadingWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-27
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cGauge;
class cUIImage;

class cLoadingWindow : public cUIWindow
{
public:
	cLoadingWindow();
	virtual ~cLoadingWindow();

	void Open();
	void Close();

	void InitGauge( float maxValue = 100.0f );
	void IncreaseGaugeValue( float destValue );

	bool IsGaugeEnd();

	double GetCurrentValue();

protected:
	/// 이벤트
	bool OnCreate( cUINodeProperty* pproperty );
	void UpdateSkin();
	void OnRender( cUIFontItemKeeper* pKeeper );

private:
	/// 로딩 게이지 
	cGauge*	mpLoadGauge;

	/// 팁 이미지
	cUIImage*	mpTipImage;

	/// 
	cUINode*	mpLogo;
};

