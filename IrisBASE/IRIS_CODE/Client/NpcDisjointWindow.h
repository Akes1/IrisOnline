/* ==========================================================================
* 파    일 : NpcDisjointWindow.h
* 목    적 : 아이템 분해창 
* 작 성 자 : 김가람
* 작 성 일 : 2008-04-10
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

const int DISJOINT_COUNT = 6;
const unsigned long DISJOINT_DELAYTIME = 3000;

class cGauge;
class cLabel;
class cButton;
class cItemIcon;
class cNpcDisjointWindow : public cUIWindow
{
public:
	cNpcDisjointWindow();
	virtual ~cNpcDisjointWindow();

	virtual void Open();
	virtual void Close();

	/// 초기화
	void Clear();

	/// 락 된 아이템 슬롯
	unsigned short GetSlotIndex();

	/// 인벤토리의 아이템 등록
	bool AddItem( unsigned short slotIndex );

private:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnGaugeEnd(cUINode* , unsigned int );
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

	void SetPrice( unsigned int price );

private:
	/// 분해할 아이템
	cItemIcon* mItemIcon;

	/// 분해 결과 
	cItemIcon* mResultIcon[DISJOINT_COUNT];

	///
	cGauge*	mpGaugeBar;

	/// 비용
	cLabel* mpPrice;

	///
	cButton* mpDisjointButton;
};

