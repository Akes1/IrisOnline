/* ====================================================================
 * 파    일 :	UIRoundGauge.h
 * 목    적 :	원형 게이지를 처리 한다.
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12
 * 주의사항 :	생성시의 크기를 변동하는 인터페이스를 제공하지 않는다.(필요시 추가!)
 * =================================================================== */
#pragma once

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

	/// 생성부 (모두 절대 좌표다!!)
	bool Create( NiTexture* pTexture, unsigned short texLeft, unsigned short texTop, unsigned short texRight,unsigned short texBottom );

	/// 처리부 (반환값 : 현재 상태로 End 이벤트를 체크해서 처리해라~)
	unsigned int Update( unsigned long accumtime );

	/// 출력부
	void Draw();

	/// 위치 셋팅 (절대 좌표를 넣자!!)
	void SetPos( short x, short y );

	void UpdateGauge( unsigned long startTime,  unsigned long elapsedTime, unsigned long maintainTime, bool active );

	void SetElapsedTime( unsigned long elapsedTime );

	/// 활성화 / 비활성화
	void Activate( unsigned long maintainTime );
	void DeActivate();	

	/// 대입
	void operator=( const cUIRoundGauge& other );

	inline bool IsActive() { return mActivate; }

	/// 남은시간
	unsigned long GetRestTime();

	unsigned long GetStartTime() { return mStartTime; }
	unsigned long GetElapsedTime() { return mElapsedTime; }
	unsigned long GetMaintainTime() { return mMaintainTime; }

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
