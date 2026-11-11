/* ==========================================================================
* 파    일 : TarotSelectWindow.h
* 목    적 : 타롯 카드 선택 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2007-08-02
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cButton;
class cNumberEditBox;

class cTarotWindow;

class cTarotSelectWindow : public cUIWindow
{
public:
	cTarotSelectWindow();
	virtual ~cTarotSelectWindow();

	/// 선택카드 장수
	void SetSelectCardCount( unsigned int count );

	void Show( bool onsound = true );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnHide();

private:
	/// 카드선택 장수
	cLabel*		mpCount;

	/// 점집이름
	cLabel*		mpShopName;

	cButton*		mpComplete;		/// 선택완료
	cButton*		mpRandomSelect;	/// 임의선택

	cUINode*		mpNameBack;
	cUINode*		mpNameTextBack;

	cTarotWindow*		mpMain;
};


//------------------------------------------------------------------------
class cNpcTarotSelectWindow : public cUIWindow
{
public:
	cNpcTarotSelectWindow();
	virtual ~cNpcTarotSelectWindow();

	/// 선택 갯수 세팅
	void SelectCount( unsigned int count, unsigned int max );
	void SetEnableComplete( bool enable );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnShow();

private:
	cTarotWindow*	mpMain;

	cLabel*			mpCount;		/// 카드선택 장수
	cButton*		mpComplete;		/// 선택완료
	cButton*		mpRandomSelect;	/// 임의선택
};