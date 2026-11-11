/* ====================================================================
 * 파    일 :	UIRoundGauge.h
 * 목    적 :	원형 게이지를 처리 한다.
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12
 * 주의사항 :	생성시의 크기를 변동하는 인터페이스를 제공하지 않는다.(필요시 추가!)
 * =================================================================== */
#pragma once

class cUINode;
class cPlaneObject;
class cUIRoundGauge
{
public:
	cUIRoundGauge();
	virtual ~cUIRoundGauge();

	enum 
	{
		eGaugeState_Start = 0,
		eGaugeState_Process,
		eGaugeState_End,

		eGaugeState_None,
	};


private:
	///
	cPlaneObject*	mpPlane;

	bool mActivate;
	unsigned int mGaugeState;

	/// 게이지 활성화 시작 시간..
	unsigned long mStartTime;

	/// 
	unsigned long mElapsedTime;

	/// 게이지가 활성화 되어 있어야 할 지속시간
	unsigned long mMaintainTime;
};
