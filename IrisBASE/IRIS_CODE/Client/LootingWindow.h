/* ==========================================================================
* 파    일 : LootingWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-10-25
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "StyleListBox.h"
#include "Item_Common.h"

/// 네트워크 락
enum
{
	eLOCK_CLOSE,
	eLOCK_GETITEM,
};

class cUIImage;
class cButton;
class cImageAndFontDataListBox;

struct sLootData
{
	unsigned long  itemDefineIndex;					// 아이템 정의 인덱스
	unsigned short count;							// 수량

	sLootData()
	{
		itemDefineIndex = 0;
		count = 0;
	}
};

const unsigned long LINK_DELAYTIME = 1000;	/// 아이템 링크 버튼 딜레이 시간


///
class cLootingWindow : public cUIWindow
{
public:
	cLootingWindow();
	virtual ~cLootingWindow();

	virtual void Open();
	virtual void Close();
	void Clear();

	///
	bool AddItem( sItemData* data );

	sLootData* GetSelectData();
	float GetWeight();

	//
	void SetDropIndex( unsigned long dropIndex ) { mDropIndex = dropIndex; }

	///
	void OnCommand( cUINode* pcaller, unsigned int id );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnListBoxSelected(cUINode* , unsigned int );
	void OnListBoxImageHorvered(cUINode* , unsigned int , const cUIPos& , void* );
	void OnListBoxImageLeft(cUINode* , unsigned int );
	void OnListBoxSelectedRButton( cUINode* , unsigned int , bool shift );
	void UpdateRect();
	void UpdateSkin();
	void OnShow();
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:

	/// 딜레이 시간 체크
	unsigned long mLinkDelayTime;

	/// sItemData
	typedef tList<void*> cItemLootList;
	cItemLootList	mItemLootList;

	///
	cImageAndFontDataListBox*  mpItemList;

	///	습득버튼
	cButton*		mpGetButton;
	cButton*		mpCloseButton;
	cButton*		mpLinkButton;
	
	// 드랍 아이템 연결 인덱스
	unsigned long	mDropIndex;
};

// 아이템 분배 윈도우
class cItemDivideWindow : public cUIWindow
{
public:
	cItemDivideWindow();
	virtual ~cItemDivideWindow();

	virtual void Open() {}
	virtual void Close() {}
	void Clear();

	void Show( unsigned long dropIndex );
	void Show( bool onsound = true ) { assert(0); }

	unsigned long GetUserIndex() { return mTempPlayerIndex; }
	unsigned long GetItemIndex() { return mTempItemIndex; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnListBoxRowHovered( cUINode* , unsigned int , const cUIPos& , void* );
	void OnListBoxRowLeft(cUINode* , unsigned int );
	void UpdateSkin();
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	// 파티원 리스트
	cListBox*	mpListBox;

	// 지급
	cButton*	mpGive;

	// 임시 저장소
	unsigned long mTempPlayerIndex;
	unsigned long mTempItemIndex;
};
