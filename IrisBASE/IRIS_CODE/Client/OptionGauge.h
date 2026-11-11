/* ==========================================================================
* 파    일 : OptionGauge.h
* 목    적 : 옵션게이지
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-22
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "UINodeData.h"

class cBarButton;
class cOptionGauge : public cUIWindow
{
public:
	cOptionGauge( eUINodeType type = eUINODE_OPTIONGAUGE );
	virtual ~cOptionGauge();
	
	/// 현재 게이지를 업데이트 
	void UpdateCurGauge();

	/// 게이지 퍼센트 리턴 
	inline float GetPercent() { return mCurPercent; }
	void SetPercent( float per );

protected:
	/// 적합 여부를 리턴
	virtual bool IsValid() const;
	/// 이벤트 처리
	virtual bool HandleEvent( const cUIEvent& event );
	virtual bool SetSkin( const cUINodeSkin* pskin );

	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );

	/// 생성
	bool OnCreate( cUINodeProperty* pproperty );
	/// 바 버튼 이동 이벤트 발생시 
	void UpdateBarButtonPos( const cUIPos& pos );

private:
	/// 바 버튼 스킨 이름 
	cString mBarButtonSkin;

	/// 바 버튼 
	cBarButton* mpBarButton;

	/// 현재 옵션게이지의 퍼센트 
	float mCurPercent;
};


/////////////////////////////////////////////////////////////////////////
class cOptionGaugeSkin : public cUINodeSkin
{
public:
	cOptionGaugeSkin( eUINodeType type = eUINODE_OPTIONGAUGE );
	virtual ~cOptionGaugeSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 바 버튼 스킨 이름
	cString mBarButtonSkin;
};