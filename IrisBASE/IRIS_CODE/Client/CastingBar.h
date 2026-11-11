/* ==========================================================================
* 파    일 : CastingBar.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-01-09
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cGauge;
class cLabel;
class cUIImage;

enum eCASTINGTYPE
{
	eCasting_Skill = 0,
	eCasting_Gather,

	eCasting_Max,
};


class cCastingBar : public cUIWindow
{
public:
	cCastingBar();
	virtual ~cCastingBar();

	virtual void Open();
	virtual void Close();

	float GetPercent();

	/// 캐스팅바의 시간 지정
	void SetTime( unsigned long index, eCASTINGTYPE type, unsigned long sec );
	void Clear( eCASTINGTYPE type );

	/// 기본함수 배제
	virtual void Show( bool ) { assert(0); }
	virtual void Hide( bool ) { assert(0); }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateRect();
	void OnGaugeEnd(cUINode* , unsigned int );
	void OnShow();
	void OnHide();
	
	void SetTimeText( float time );

protected:
	/// 캐스팅 게이지 틀
	cGauge*			mpCastingBar;
	cLabel*			mpName;	/// 스킬 이름
	cLabel*			mpTime;	/// 캐스팅 남은 시간

	cPlaneObject*	mpCastingBarEndEffect;

	eCASTINGTYPE	mType;
	unsigned long	mCastingIndex;

	unsigned long	mStartTime;	
	unsigned long	mRestTime;		/// 남은시간

	cUIImage*	mpIconImage;
	cUIPos				mIconPos;
};


