/* ==========================================================================
* 파    일 : ItemTipWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-09-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TipWindow.h"

const int SLOT_GAP = 2;

class cItemDefine;
class cUIImage;

//////////////////////////////////////////////////////////////////////////
/// 아이템용 팁 윈도우
class cItemTipWindow : public cTipWindow
{
public:
	cItemTipWindow( bool equipItem = false );
	virtual ~cItemTipWindow();

	virtual void Open() {}

	bool SetItemInfo( eTip_Kind tipkind, const cUIPos& pos, sInventory& inven, sItemBill* bill, bool checkTime );

	///
	void SetTipPos( unsigned int x, unsigned int y );

protected:
	bool OnCreate( cUINodeProperty* prop );
	void OnRender( cUIFontItemKeeper* fontKeeper );
	void OnTimer();

	/// 툴팁 설정 
	bool SetName( cItemDefine* itemDefine, unsigned char enhance );
	bool SetRare( cItemDefine* itemDefine );
	bool SetBasic( cItemDefine* itemDefine, unsigned char enhance );

	bool SetType( unsigned char type, unsigned char subtype, short defenseType );
	bool SetSpecial( cItemDefine* itemDefine );
	bool SetSpecialCard( cItemDefine* itemDefine );
	void SetLimit( cItemDefine* itemDefine );
	bool SetEffect( cItemDefine* itemDefine );
	bool SetUseTime( cItemDefine* itemDefine );
	void SetPrice( unsigned long type, cItemDefine* itemDefine, unsigned long priceTooltipIdx, unsigned long price );
	void SetItemSetInfo( cItemDefine* itemDefine, unsigned int setIndex );
	unsigned long GetTooltipIdxByOpt( short type );

	bool AddSpecial( bool usePrint, unsigned long tooltipIdx, short value, bool usePercent );
	bool AddSpecialEquipCard( bool addfront, unsigned long tooltipIdx, short value, bool usePercent );

	/// 툴팁 배경 이미지 세팅
	virtual void SetBackImg( const cUIPos& pos );
	virtual void Clear();

	///
	void SetCard( sInventory& inven );
	bool SetCardData( int i, unsigned long cardIndex );

	//
	unsigned long GetMixGradeIndex( char step );

protected:
	/// 임시
	unsigned long mItemIndex;

	bool mCheckTime;
	sItemBill*		mItemBill;

	/// 카드 이미지
	bool			mShowImage[MAX_CARD_SLOT];
	cUIImage*		mImage[MAX_CARD_SLOT];
	cUIImage*		mSlotImage[MAX_CARD_SLOT];
	cUISize			mSlotSize;
	
	cUIPos			mTexPos[4];
	unsigned int	mSlotCount;
	
	/// 이미지 찍힐 좌표
	unsigned int	mImagePosY;

	/// 장착중 아이템 여부
	bool			mIsEquipItem;
};


//////////////////////////////////////////////////////////////////////////
/// 채팅 팁 윈도우
class cChatTipWindow : public cItemTipWindow
{
public:
	cChatTipWindow();
	virtual ~cChatTipWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* prop );
	void OnRender( cUIFontItemKeeper* fontKeeper );
	void OnCommand(cUINode* , unsigned int );
	void UpdateRect();
	void UpdateSkin();

	void SetBackImg( const cUIPos& pos );

private:
	cUINode*		mpClose;
};
