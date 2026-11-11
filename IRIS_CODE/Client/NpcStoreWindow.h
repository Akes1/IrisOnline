/* ==========================================================================
* 파    일 : NpcStoreWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-05-30
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemIcon.h"
#include "ItemManager.h"


const int MAX_RADIO = 7;
const int ITEMWIDTH = 162;

class cDoubleItemIconListBox;
class cRadioButton;
class cButton;
class cNpcStoreWindow : public cUIWindow
{
public:
	cNpcStoreWindow();
	virtual ~cNpcStoreWindow();

	virtual void Open();
	virtual void Close();

	void Show( unsigned long npcIndex );
	unsigned long GetTabIndex(){ return mTabIndex; }
	unsigned char GetCurrentItemSellType();
	unsigned long GetCurrentItemPriceRate();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* , unsigned int id );
	void OnListBoxSelected( cUINode* , unsigned int );
	void OnListBoxSelectedRButton(cUINode* , unsigned int , bool shift );
	void OnIconRightClicked( cUINode* caller, unsigned int /*id*/, bool shift );
	void OnIconDragged( cUINode* , unsigned int , const cUIPos& , bool );
	void OnRadioPressed( cUINode* , unsigned int );
	void UpdateRect();
	void UpdateSkin();
	void OnShow();
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

	void UpdateItemList( unsigned int tabIndex );

protected:
	unsigned long mNpcIndex;
	unsigned long mTabIndex;

	/// 판매 리스트박스
	cDoubleItemIconListBox*	mpList;

	/// 라디오 버튼 최대수
	cRadioButton*	mpRadio[MAX_RADIO];
	cUINodeSkin*	mpRadioSkin;
	cButton*		mpBuy;
};
