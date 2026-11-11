/* ==========================================================================
* 파    일 : RowQuickSlot.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-06-19
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Icon.h"
#include "QuickManager.h"

/// 세로형 퀵슬롯 
class cRowQuickSlot : public cUIWindow
{
public:
	cRowQuickSlot( eQuickSlotType type );
	virtual ~cRowQuickSlot();

	virtual void Open();

	void ChangeShortcut( unsigned int slotIndex, int keyboardIndex );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* caller, unsigned int id );
	void OnIconHovered(cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );
	void OnIconDoubleClicked( cUINode* caller, unsigned int id );
	void UpdateRect();

protected:

	/// 퀵슬롯을 구분하기 위한 구분자.
	eQuickSlotType mType;

	cIcon* mIcon[QUICKSLOT_COUNT];

	/// 퀵슬롯 단축 키 이미지
	cUIImage*	mpKeyImage[QUICKSLOT_COUNT];

	/// 퀵슬롯 단축 키 스킨
	typedef tPointerHashMap< int, void* > sKeySkinMap;
	sKeySkinMap	mKeySkinMap;

};	

/// FS 퀵슬롯
class cFSRowQuickSlot : public cRowQuickSlot
{
public:
	cFSRowQuickSlot();
	virtual ~cFSRowQuickSlot();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* caller, unsigned int id );
	void OnIconHovered(cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );
	void OnIconDoubleClicked( cUINode* caller, unsigned int id );
};

/// 추가 세로형  퀵 슬롯
class cRowExtraSlot : public cRowQuickSlot
{
public:
	cRowExtraSlot();
	virtual ~cRowExtraSlot();

	virtual void Open();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* caller, unsigned int id );
	
};
