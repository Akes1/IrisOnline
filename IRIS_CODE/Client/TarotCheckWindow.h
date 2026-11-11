/* ==========================================================================
* 파    일 : TarotCheckWindow.h
* 목    적 : 점집확인 윈도우
* 작 성 자 : 김가람 
* 작 성 일 : 2007-08-02
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cTarotWindow;

class cTarotCheckWindow : public cUIWindow
{
public:
	cTarotCheckWindow();
	virtual ~cTarotCheckWindow();

	void Show( bool onsound = true );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnShow();
	void OnHide();
	
private:
	/// 카드수치총합
	cLabel*		mpTotal;

    /// 점집이름
	cLabel*		mpShopName;

	/// 이용요금
	cLabel*		mpPrice;

	cUINode*	mpNameBack;
	cUINode*	mpPayBack;
	cUINode*	mpNameTextBack;
	cUINode*	mpPriceTextBack;

	cTarotWindow*		mpMain;
};

//------------------------------------------------------------------
class cNpcTarotCheckWindow : public cUIWindow
{
public:
	cNpcTarotCheckWindow();
	virtual ~cNpcTarotCheckWindow();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnShow();
	void OnHide();

private:
	cTarotWindow*		mpMain;
	cLabel*				mpPrice;		/// 이용요금
};

