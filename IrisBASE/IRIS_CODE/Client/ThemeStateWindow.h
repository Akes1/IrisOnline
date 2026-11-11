/* ==========================================================================
* 파    일 : ThemeStateWindow.h
* 목    적 : 
* 작 성 자 : 조영인
* 작 성 일 : 2009-06-04
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cButton;
class cListFolderBox;
class cTextBox;
class cUIImage;

struct MSG_SYN_THEME_LIST;
struct sPlayerThemeList;

class cThemeStateWindow : public cUIWindow
{
public:
	cThemeStateWindow();
	virtual ~cThemeStateWindow();

	virtual void Open() {}
	virtual void Close();

	virtual void Clear();

	//
	virtual void AddInfo( sPlayerThemeList* info );
	virtual void UpdateInfo();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* node, unsigned int id );
	void OnListBoxChanged( cUINode*, unsigned int );
	void OnShow();

protected:
	/// 테마 인던 리스트 박스
	cListFolderBox* mpFolderBox;

	/// 테마 인던 정보 박스
	cTextBox*	mpInfoBox;
 	
	cUIImage* mpHighImage;

	/// 테마 인던 정보 박스에서 초기화 잔여 시간 갱신 라인
	int mTimeLineNum;
};

// 테마 던전 귀속 초기화
class cThemeResetWindow : public cThemeStateWindow
{
public:
	cThemeResetWindow();
	virtual ~cThemeResetWindow();

	void Open() {}
	void Close();

	void Show( bool onsound = true ) { assert(0); }
	void Show( unsigned short slotIndex );

	virtual void UpdateInfo();

	sPlayerThemeList* GetSelectData();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnListBoxChanged( cUINode*, unsigned int );
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	cUINode*	mpReset;

	// 해당 윈도우 연 아이템 슬롯
	unsigned short mInventorySlotIndex;
};