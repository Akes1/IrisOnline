/* ==========================================================================
* 파    일 : ItemIcon.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-09-14
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Icon.h"

const unsigned int SELECT_INDEX = 2;
const unsigned int SEAL_INDEX = 218;

/// 아이콘
class cItemIcon : public cIcon
{
	friend class cItem;
	friend class cTradeItem;

public:
	cItemIcon( eUINodeType type = eUINODE_ICON );
	virtual ~cItemIcon();

	/// 대입
	void operator=( cItemIcon& other );

	/// 데이터 초기화
	void Clear();

	/// 인벤토리
	void SetSlotIndex( unsigned short index );
	unsigned short GetSlotIndex();

	/// 아이템
	void SetItemIndex( unsigned int itemIndex );
	unsigned int GetItemIndex();

	/// 아이템 고유 인덱스 ( DB )
	void SetDBIndex( unsigned long DBIndex );
	unsigned long GetDBIndex();

	/// 선택된 아이콘 
	void SetSelect( bool select );

	/// 참조 세팅 - 실제 인벤토리 내에 포함된 아이템 아이콘이 아니면 전부 이거 세팅해서 사용하도록한다!!!!!!!!!!!!
	/// 다만, 아이템 툴팁에서 define 참조만으로 표시된다면 세팅필요없다.
	/// 인벤의 item 속성을 그대로 출력해야하는경우는 반드시 참조 세팅!!!!!!!
	void SetLinked( bool link );
	bool IsLinked() { return mLinked; }

	//
	void SetSeal( bool seal ) { mSeal = seal; }
	bool IsSeal(){ return mSeal; }

protected:
	/// 이벤트
	virtual bool OnCreate(cUINodeProperty* pproperty );
	virtual void OnRender(cUIFontItemKeeper* pKeeper );
	virtual void OnMouseHovered(const cUIPos& pos );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void UpdateRect();

protected:
	/// 슬롯 인덱스
	unsigned short mSlotIndex;

	/// 아이템 인덱스
	unsigned int mItemIndex;

	/// 아이템 고유 인덱스 ( DB 인덱스 )
	unsigned long mDBIndex;

	/// 선택된 아이콘 표시 
	cUIImage* mpSelectImage;
	bool	  mIsSelect;

	/// 미인증 이미지
	cUIImage* mpSealImage;
	bool	mSeal;

	/// 참조된 아이콘 ( 실제 아이템이 아님 )
	bool	 mLinked;
};

inline
void cItemIcon::SetSlotIndex( unsigned short i )
{
	mSlotIndex = i;
}

inline
unsigned short cItemIcon::GetSlotIndex()
{
	return mSlotIndex;
}

inline
void cItemIcon::SetItemIndex( unsigned int i )
{
	mItemIndex = i;
}

inline
unsigned int cItemIcon::GetItemIndex()
{
	return mItemIndex;
}

inline 
void cItemIcon::SetDBIndex( unsigned long DBIndex )
{
	mDBIndex = DBIndex;
}

inline
unsigned long cItemIcon::GetDBIndex()
{
	return mDBIndex;
}

inline
void cItemIcon::SetSelect( bool select )
{
	mIsSelect = select;
}

inline
void cItemIcon::SetLinked( bool link )
{
	mLinked = link;
}

