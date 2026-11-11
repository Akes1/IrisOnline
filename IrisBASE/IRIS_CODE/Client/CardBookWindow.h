/* ==========================================================================
* 파    일 : CardBookWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-06-10
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TabWindow.h"

enum eBookPage
{
	ePAGE_DATA = 0,
	ePAGE_TAROT, 
	ePAGE_MONSTER,
	ePAGE_STORY,
	ePAGE_MAX,
};

enum eBookLock
{
	eBook_Open,
	eBook_Close,
};

class cTarotSheetWindow;
class cCardBookWindow : public cTabWindow
{
public:
	cCardBookWindow();
	virtual ~cCardBookWindow();

	///
	void Clear();
	
	virtual void Open();

	/// 현재 페이지 리턴
	eBookPage GetCurrentPage() { return mPage; }

	/// sheet 리턴
	cTarotSheetWindow* GetTarotSheet() { return mTarotSheet; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnTabButtonPressed( cUINode*, unsigned int id );
	void OnRender( cUIFontItemKeeper* pkeeper );

	/// 페이지 변경
	void SetCurrent( unsigned int index );

private:
	eBookPage	mPage;

	cTarotSheetWindow*	mTarotSheet;
};