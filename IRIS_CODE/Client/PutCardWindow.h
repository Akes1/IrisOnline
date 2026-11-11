/* ==========================================================================
* 파    일 : PutCardWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-05-13
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cItemIcon;
class cUIImage;
class cPutCardWindow : public cUIWindow
{
public:
	cPutCardWindow();
	virtual ~cPutCardWindow();

	virtual void Open();
	virtual void Close();

	/// 초기화
	void Clear();

	/// 강화 아이템 삽입
	bool AddItem( unsigned short slotIndex );

	/// 장착할 카드 삽입
	bool TestAddCard( unsigned short slot0, unsigned short slot1 );
	bool AddCard( sInventory& inv );

	///
	unsigned short GetItemSlotIndex() { return mItemSlotIndex; }

	void Show( bool onsound );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos );
	void UpdateRect();
	void UpdateSkin();
	void OnShow();
	void OnHide();

	/// 장착슬롯 갯수
	void SetCount( short count );

private:
	/// 카드장착할 슬롯
	cItemIcon*	mItemSlot;
	unsigned int mSlotCount;	// 사용가능한 슬롯 갯수

	/// 카드슬롯
	cItemIcon*	mCardSlot[MAX_CARD_SLOT];

	/// 슬롯 이미지 
	cUIImage*	mSlotImage[MAX_CARD_SLOT];
	cUINodeSkin* mSlotImageSkin[MAX_CARD_SLOT];

    /// 카드슬롯 텍스쳐좌표
	cUIPos		mTexPos[4];

	/// 슬롯갯수 라벨
	cLabel*		mpCount;

	/// 등록된 아이템 인덱스
	unsigned short mItemSlotIndex;
};