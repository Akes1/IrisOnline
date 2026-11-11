/* ==========================================================================
* 파    일 : NpcDealRegistSheet.h
* 목    적 : 판매대행 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2009-04-20
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "StyleListBox.h"

class cItemIcon;
class cNumberEditBox;
class cComboBox;
class cLabel;
class cComboBox;
class cButton;
class cSpinBox;
struct NPC_ITEM_AGENT_OPEN_RESULT;

const unsigned int CANCELBUTTON_ID = 10000;
const unsigned int MAX_REGIST_ITEM_COUNT = 10;

class cNpcDealRegistSheet : public cUIWindow
{
public:
	cNpcDealRegistSheet( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cNpcDealRegistSheet();

	void Clear();

	/// 아이템 등록
	bool RegistItem( unsigned short slotIndex );
	void DeleteRegistItem();

	/// 등록 아이템 리스트 세팅
	bool AddItem( NPC_ITEM_AGENT_OPEN_RESULT* data );

	void SetTotalPage( unsigned int total );
	void ChangePage( unsigned int page );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnComboSelected( cUINode* , unsigned int );
	void OnIconDragged(cUINode* , unsigned int , const cUIPos& , bool );
	void OnListBoxImageHorvered(cUINode* , unsigned int , const cUIPos& , void* );
	void OnListBoxImageLeft(cUINode* , unsigned int );
	void UpdateSkin();
	void OnHide();

	void UpdateCharge();
	void UpdatePage();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	// 삭제용 NPC_ITEM_AGENT_OPEN_RESULT
	typedef tPointerHashMap<unsigned long, void*> cNpcDealOpenMap;
	cNpcDealOpenMap	mDealOpenMap;

	cItemIcon*		mpIconSlot;

	/// 판매금액
	cNumberEditBox*	mpPriceEdit;

	/// 등록 수수료
	cLabel*		mpRegistCharge;

	/// 기한
	cComboBox*	mpRegistPeriod;

	/// 등록버튼
	cUINode*	mpRegist;

	/// 아이템 등록 리스트
	cImageAndFontDataListBox*	mpItemList;

	/// 취소버튼
	cButton*		mpCancelButton[10];

	/// 페이지 스핀
	cSpinBox*		mpPageSpin;
};
