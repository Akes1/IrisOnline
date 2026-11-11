/* ==========================================================================
* 파    일 : TradeWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-04-17
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemManager.h"

class cLabel;
class cPushButton;
class cButton;
class cItemIcon;

struct sThatInvetory
{
	sInventory mThatInv;
	cItemIcon* mThatIcon;

	sThatInvetory()
	{
		mThatIcon = 0;
		mThatInv.idx		= 0;
		mThatInv.ItemIndex	= 0;
		mThatInv.number		= USHRT_MAX;
		mThatInv.count		= 0;
		mThatInv.enhanced	= 0;
		mThatInv.seal		= 0;
		mThatInv.cash		= 0;
	}
};

/// 거래 윈도우
class cTradeWindow : public cUIWindow
{
public:
	cTradeWindow();
	virtual ~cTradeWindow();

	virtual void Open();
	virtual void Close();

	void Clear();

	void SetStatus( eItemExchangeStatus status, bool myTurn );
	eItemExchangeStatus GetStatus() { return mStatus; }

	/// 두명 아이디 세팅
	void SetNames( LPCTSTR otherName );

	/// 아이템 삽입
	bool AddMyItem( unsigned short slot );
	bool DeleteMyItem( unsigned short arrIndex );	/// 트레이드 슬롯

	/// 상대방아이템 삽입
	bool AddThatItem( sInventory* inv );
	bool DeleteThatItem( sInventory* inv );

	/// 
	void AddMyMoney( unsigned long money );
	void AddThatMoney( unsigned long money );

	/// 
	unsigned short GetMyLastSlot() { return mMyLastSlot; }
	unsigned short GetThatLastSlot() { return mThatLastSlot; }
	unsigned short GetMySlotIndex( int i );
	sThatInvetory* GetThatItem() { return mThatItem; }
	unsigned long  GetMyTotalWeight();
	/// 
	bool IsNetLock();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* node, unsigned int id );
	void OnPushButtonPressed( cUINode* , unsigned int id );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnHide();
	void HideESC( bool onsound );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void UpdateSkin();

	void UpdateMyData( unsigned short index );
	void UpdateThatData( unsigned short index );

protected:
	/// 내 등록아이템
	cItemIcon* mMyItem[MAX_ITEM_EXCHANGE];

	/// 상대방 등록 아이템
	sThatInvetory mThatItem[MAX_ITEM_EXCHANGE];

	/// 마지막으로 채워진 아이템 트레이드 슬롯인덱스
	unsigned short mMyLastSlot;
	unsigned short mThatLastSlot;

	/// 아이디
	cLabel* mpName;
	cLabel* mpOtherName;

	/// 금액
	cLabel* mpMyMoney;
	cLabel* mpThatMoney;

	/// 버튼 : [press: 물품완료상태, not press: 물품수정상태]
	cButton* mTradeButton;
	cPushButton* mpMyOk;
	cPushButton* mpThatOk;

	/// 거래상태 (내상태, 상대방상태)
	eItemExchangeStatus mStatus;
	eItemExchangeStatus mThatStatus;

	// 물품확인버튼 딜레이
	unsigned long mOKAccumTime;
};

