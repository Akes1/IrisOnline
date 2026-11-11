/* ==========================================================================
* 파    일 : ItemReceipt.h
* 목    적 : 아이템 수령함 윈도우
* 작 성 자 :
* 작 성 일 : 2009-12-14
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemManager.h"

class cItemReceiptWindow : public cUIWindow
{
public:
	cItemReceiptWindow();
	virtual ~cItemReceiptWindow();

	void Open() {}
	void Close();
	void Show( bool onsound = true );
	void Hide( bool onsound = true );

	void ClearTempInven();
	void AddTempInven( sCashInventory* inven );

	void SaveResult( int result ) { mSaveResult = result; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnIconDragged( cUINode* pcaller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconRightClicked(  cUINode* caller, unsigned int id, bool shift );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	cItemIcon*	mIcon[RECEIPT_SLOT_COUNT];

	//
	cUINode*	mpGetBtn;
	cUINode*	mpCloseBtn;

	// 메세지 요청에 대한 결과
	int				mSaveResult;
	unsigned long	mDelayTime;

	// 아이템 정보 임시저장 (sCashInventory*)
	typedef tPointerHashMap<unsigned long, void*> cTempInvenMap;
	cTempInvenMap	mTempInvenMap;
};
