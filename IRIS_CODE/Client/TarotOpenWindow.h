/* ==========================================================================
* 파    일 : TarotOpenWindow.h
* 목    적 : 타롯 개설 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2007-08-02
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cMultiEditBox;
class cNumberEditBox;

class cTarotWindow;

class cTarotOpenWindow : public cUIWindow
{
public:
	cTarotOpenWindow();
	virtual ~cTarotOpenWindow();

	/// 카드수치 총합
	void SetTotalValue( unsigned int total );

	/// 점횟수 세팅
	void SetTarotCount( unsigned int count, unsigned int total );

	/// 재료수 세팅
	void SetStuffNum( unsigned int num );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnShow();
	void OnHide();

private:
	/// 카드수치총합
	cLabel*		mpTotal;

	/// 점봐주기 횟수
	cLabel*		mpTarotCount;

	/// 재료갯수
	cLabel*		mpStuffNum;

	/// 가격 에디트
	cNumberEditBox*	mpPrice;

	/// 점집이름 멀티 에디트
	cMultiEditBox*	mpName;

	cTarotWindow*		mpMain;
};
