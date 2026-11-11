/* ==========================================================================
* 파    일 : TarotControlWindow.h
* 목    적 : 타롯 관리 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2007-08-02
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cNumberEditBox;
class cPushButton;

class cTarotControlWindow : public cUIWindow
{
public:
	cTarotControlWindow();
	virtual ~cTarotControlWindow();

	virtual void Open();
	virtual void Close();

	/// 점집이름 세팅
	void SetShopName( LPCTSTR name );

	/// 이용요금 세팅
	void SetPay( unsigned int pay );

	/// 점 횟수 세팅 
	void SetTarotCount( unsigned int count );

	/// 재료갯수 세팅
	void SetStuffNum( unsigned int num );

	/// 현재 상태 세팅
	void SetStatus( LPCTSTR name );
	void ReleaseStatus( LPCTSTR name );

	/// 이용자수 세팅
	void SetUserNum( unsigned int num );

	/// 획득 돈 세팅
	void SetMoney( unsigned int money );

	/// 획득 포인트 세팅
	void SetPoint( unsigned int point );

	void SetReserve( bool press );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnHide();
	void OnPushButtonPressed( cUINode* , unsigned int );

	void Clear();

private:
	/// 점집이름
	cLabel*			mpShopName;

	/// 이용요금
	cNumberEditBox*	mpPay;

	/// 점 횟수
	cLabel*			mpTarotCount;

	/// 재료갯수
	cLabel*			mpStuffNum;

	/// 현재상태
	cLabel*			mpStatus;
	/// 이용자 이름
	cStringT		mUserName;

	/// 이용자수
	cLabel*			mpUserNum;

	/// 돈
	cNumberEditBox*	mpMoney;

	/// 포인트
	cLabel*			mpPoint;

	unsigned int	mPrice;

	/// 타로 닫기/예약 버튼
	cPushButton*	mpClose;
};