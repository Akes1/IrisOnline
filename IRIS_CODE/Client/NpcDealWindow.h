/* ==========================================================================
* 파    일 : NpcDealWindow.h
* 목    적 : 판매대행 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2009-04-20
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TabWindow.h"

class cNpcDealRegistSheet;
class cNpcDealSearchSheet;
class cLabel;

class cNpcDealWindow : public cTabWindow
{
public:
	cNpcDealWindow();
	virtual ~cNpcDealWindow();

	virtual void Open();
	virtual void Close();

	cNpcDealRegistSheet* GetRegistSheet() { return mpRegistSheet; }
	cNpcDealSearchSheet* GetSearchSheet() { return mpSearchSheet; }

	void UpdateHaveMoney();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnShow();
	void OnHide();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	/// 등록 시트
	cNpcDealRegistSheet*	mpRegistSheet;

	/// 검색 시트
	cNpcDealSearchSheet*	mpSearchSheet;

	/// 소지금
	cLabel*			mpHaveMoney;
};
