/* ==========================================================================
* 파    일 : LookWindow.h
* 목    적 : 살펴보기 윈도우
* 작 성 자 : 
* 작 성 일 : 2010-03-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemManager.h"

//
class cItemIcon;
class cLabel;

struct sLookInventory
{
	sInventory inven;
	cItemIcon* mpIcon;

	sLookInventory()
	{
		memset( &inven, 0, sizeof(inven) );
		mpIcon = 0;
	}
};

class cLookWindow : public cUIWindow
{
public:
	cLookWindow();
	virtual ~cLookWindow();

	void Clear();
	void Open() {}
	void Close();

	//
	void AddInfo( LPCTSTR name, unsigned char level, unsigned short job, unsigned short wearEnd );

	//
	void SwitchWeapon( eItemActiveWeapon activeWeapon );
	void AddInven( sInventory* inven );
	
	void Show( bool onsound = true );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnIconHovered( cUINode* , unsigned int , const cUIPos& );
	void OnHide();

	// 양손등 슬롯 모양새 맞춰주기
	void UpdateSlot( unsigned short slot );

private:
	sLookInventory mInven[EQUIP_SLOT_COUNT];

	// 이름
	cLabel*	mpName;

	// 정보
	cLabel*	mpInfo;

	//
	bool mIsSet;

	// 무기 스위칭 좌표
	cUIRect mFrontLeft;
	cUIRect mFrontRight;
	cUIRect mBackLeft;
	cUIRect mBackRight;
};
