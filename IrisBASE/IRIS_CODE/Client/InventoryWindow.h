/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.12
* 내    용 : 인벤토리 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "TabWindow.h"

const unsigned short MAX_INVENTAB = 2;

class cPushButton;
class cGauge;
class cItemIcon;
class cNumberEditBox;
class cUIImage;
class cRadioButton;
/// 인벤토리 윈도우

enum
{
	ePVP_FORCE_FIRE,
	ePVP_FORCE_WARTER,
	ePVP_FORCE_WIND,
	ePVP_FORCE_EARTH,

	ePVP_FORCE_MAX
};

class cInventoryWindow : public cTabWindow
{
public:
	cInventoryWindow();
	virtual ~cInventoryWindow();

	virtual void Open();
	virtual void Close();

	/// 가방 버튼
	void SetBagButtonPressed( unsigned int index, bool pressed );

	/// 돈
	void SetMoney( unsigned long money );
	unsigned long GetMoney();

	///
	void ChangeSelect( unsigned short slotIndex );
	void HideSelect();

	bool IsShowBag( unsigned int bagIdx );

	void BackupWindowPos();

	/// 가방 제한 설정
	bool SetBagEndSlot( unsigned short slotIndex );
	bool SetWearCardSlot( unsigned short slotEnd );

	void Show( bool onsound = true );

	/// 전장 
	void UpdateForcePoint();
	void InitForcePoint();
	
protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnPushButtonPressed( cUINode* caller, unsigned int id );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void OnIconDoubleClicked(cUINode* , unsigned int );
	void OnIconRightClicked(cUINode* , unsigned int , bool );
	void OnTabButtonPressed( cUINode* caller, unsigned int id );
	void OnRadioPressed( cUINode* , unsigned int id );	
	void UpdateRect();
	void UpdateSkin();
	void OnHide();
	void HideESC( bool onsound );

	void UpdateSelect( cItemIcon* icon );

	void ApplyBackupPos();

	void UpdateUIByInven( bool backup );

	void SetForcePoint( unsigned char force );

protected:
	/// 무게 게이지
	cGauge* mWeightGauge;

	/// 선택된 아이콘 표시 
	cItemIcon*  mSelectIcon;
    cUIImage*	mpSelectImage;
	bool		mShowSelect;

	/// 가방
	cPushButton* mBagButton[3];

	/// 소유금액
	cNumberEditBox* mpMoney;

	/// 전장 세력별 버튼
	cRadioButton*	mpForceRadio[4];

	/// 전장 포인트
	cNumberEditBox* mpForcePointEdit;

	/// 가방의 show 상태 저장
	BYTE mShowBit;

	/// npc상점 이전의 위치를 저장한다.
	bool		mIsBackup;
	cUIPos		mBackupPos[4];
	bool		mBackupBagBtn[3];

	//
	cUIWindow*	mpEquipWindow;
};

inline void cInventoryWindow::HideSelect()
{
	mShowSelect = false;
}
