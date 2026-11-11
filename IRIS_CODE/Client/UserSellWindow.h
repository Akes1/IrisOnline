/* ==========================================================================
* 파    일 : UserSellWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-08-29
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemIcon.h"
#include "ItemManager.h"

class cEditBox;
class cItemIcon;
class cButton;
class cPlaneObject;

enum
{
	eLOCK_NONE = 0,
	eLOCK_REGIST,
	eLOCK_RENAME,
	eLOCK_ITEMUPDATE,
	eLOCK_END,
	eLOCK_JOINEND,
	eLOCK_ADD,
	eLOCK_DELETE,
	eLOCK_GET,
};

struct sUserSellData
{
	cItemIcon*	  mIcon;
	cStringT	  mPriceStr;
	cStringT	  mItemName;
	unsigned long mColor;
	unsigned long mPrice;
	
	unsigned char mStatus;			/// 아이템의 상태
	union
	{
		sInventory	   mThatInven;	/// 상대방의 인벤
		unsigned short mSlotIndex;	/// 자신의 인벤 슬롯 번호
	};

	sUserSellData()
	{
		mIcon = 0;
		mStatus = StallSellItemNone;
		mSlotIndex = USHRT_MAX;
		mColor = COLOR_WHITE;
	}
};

enum 
{
	eUSERSELL_NONE = 0,
	eUSERSELL_SELL,			/// 판매자 상태
	eUSERSELL_JOIN,			/// 구입자 상태
};

/// 판매상점
class cUserSellWindow : public cUIWindow
{
public:
	cUserSellWindow();
	virtual ~cUserSellWindow();

	virtual void Open();
	virtual void Close();

	void Clear();
	void SetStatus( unsigned char status );							/// 상태 세팅
	
	bool AddSellItem( unsigned short slot, unsigned long money );		/// 판매 상태 - 아이템
	bool DelSellItem( unsigned int invenSlot, bool onSound = false );
	void ModSellItem( unsigned short slot, unsigned int count, bool onSound = false );

	bool AddJoinItem( sStallSellData* sell );						/// 구매 상태 - 아이템
	bool DelJoinItem( sInventory* inv );
	void PauseJoinItem( sInventory* inv );							/// 해당 아이템 수정중

	sUserSellData* GetSelectSellData();
	sUserSellData* GetSelectJoinData();

	/// 오픈상태로 세팅
	void SetOpen();
	bool IsOpen() { return mUserSellOpen; }

	bool IsNetLock();

	///
	void SetName( LPCTSTR title );
	void SetTargetIndex( unsigned long targetIdx );
	unsigned long GetTargetIndex() { return mTargetIndex; }

	unsigned short GetSelectInvenSlot();		/// 참조한 인벤의 슬롯인덱스 획득
	int IsRegistSellSlot( unsigned int slot );
	int IsRegistJoinSlot( unsigned int slot );
	unsigned char GetStatus() { return mStatus; }

	virtual void HideESC( bool onsound = true );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* pnode, unsigned int id );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	void OnLButtonDoubleClick( const cUIPos& pos );
	void OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	void OnIconClicked(cUINode* caller, unsigned int );
	void OnIconDoubleClicked( cUINode* , unsigned int );
	void OnIconRightClicked( cUINode* , unsigned int , bool );
	void OnIconDragged(cUINode* caller, unsigned int, const cUIPos&, bool );
	void OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnShow();
	void OnHide();
	void UpdateRect();
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

	/// 판매관련 데이터 초기화
	void ClearSellData( sUserSellData* data, unsigned int index );
	void UpdateData( sUserSellData* data, unsigned int index, bool enalbe );
	void UpdateSelect( int i );
	int FindSlot( const cUIPos& pos );	/// 선택된 아이템 리턴

	/// 해당 노점 슬롯을 더블클릭을 했을때.
	void EventSelectItem( int select );
	
private:
	/// 상태 
	unsigned char	mStatus;
	unsigned int	mLastSlot;		///  ( 0 ~ MAX_STALL_ITEM )
	
	sUserSellData	mSellData[MAX_STALL_ITEM];	/// 판매 데이터
	sUserSellData	mJoinData[MAX_STALL_ITEM];	/// 구입 데이터
	
	cUINodeSkin* mNameSkin;			/// 아이템 이름 출력 기본 위치
	cUINodeSkin* mPriceSkin;		/// 아이템 가격 출력 기본 위치
	cUINodeSkin* mImageSkin;		/// 아이템 선택 이미지

	///
	cEditBox*		mpName;
	cButton*		mpNameRegist;
	cButton*		mpItemUpdate;
	cButton*		mpEnd;			/// 노점종료
	cButton*		mpJoinEnd;		/// 이용종료

	/// 판매노점상을 개설하고 있는중인지 여부
	bool			mUserSellOpen;
	
	/// usersell 순서 
	unsigned int	mArrayIndex;

	/// 선택 이미지
	bool			mShowSelect;
	cPlaneObject*	mpSelectImg;
	cUIRect			mRect[MAX_STALL_ITEM];

	/// join중인 노점개설자의 인덱스
	unsigned long	mTargetIndex;
};

inline
void cUserSellWindow::SetTargetIndex( unsigned long targetIdx )
{
	mTargetIndex = targetIdx;
}
