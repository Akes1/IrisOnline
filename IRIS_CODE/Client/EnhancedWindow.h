/* ==========================================================================
* 파    일 : EnhancedWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-04-16
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemIcon.h"

const unsigned char ITEM_ENHANCE_SLOT		=	1;
const unsigned char CARD_ENHANCE_SLOT		=	2;
const unsigned char CASHITEM_ENHANCE_SLOT	=	3;

/// 아이템 강화 윈도우 
class cButton;
class cGauge;
class cEnhancedWindow : public cUIWindow
{
public:
	cEnhancedWindow();
	virtual ~cEnhancedWindow();

	virtual void Open();
	virtual void Close();

	void Clear();
	void ClearItemSlot();
	void ClearCardSlot();
	void ClearCahProtectCardSlot();

	bool AddItem( unsigned short slotIndex, bool onSound = false, bool autoAdd = false );		/// 강화 아이템 삽입
	bool AddCard( unsigned short slotIndex, bool onSound = false );		/// 강화 카드 삽입
	bool AddCashProtectCard( unsigned short slotIndex, bool onSound = false );		/// 캐시 아이템 강화 보호 카드 삽입

	/// 카드와 묶인 인벤토리 검사 후 카드 자동 추가,삭제
	void UpdateCardSlot();
	void UpdateCardCount();
	void UpdateCashProtectSlot();
	void UpdateCashProtectCount();
	
	///
	void StartEnhanced();
	void StopEnhanced();
	void SetStartBtnEnable( bool bEnable );

	/// 결과세팅
	void SetResult( bool success );

	void Show(bool onsound = true);
	void HideESC(bool onsound = true);

	bool IsEnableEnhance();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnShow();
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );

	void HideResult();

private:
	/// 강화아이템 용 슬롯
	cItemIcon*	mItemSlot;

	/// 강화카드용 슬롯
	cItemIcon*	mCardSlot;

	/// 아이템 강화 보호 카드용 슬롯
	cItemIcon*	mCashProtectCardSlot;

	///
	cButton*	mpStart;
	cButton*	mpCancel;
	cGauge*		mpGaugeBar;

	/// 성공/실패 이미지
	cUINode*	mpSuccess;
	cUINode*	mpFailure;

	bool		mIsEnhanceNow;
};

