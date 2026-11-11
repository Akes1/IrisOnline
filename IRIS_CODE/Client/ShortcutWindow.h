/* ==========================================================================
* 파    일 : ShortcutWindow.h
* 목    적 : 
* 작 성 자 : 조영인
* 작 성 일 : 2009-09-01
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ShortcutManager.h"

class cBaseIcon;
class cUIImage;

/// ※주의:path_shortcut.txt 파일의 키보드 인덱스와 동일해야함
enum eKEYBOARD_KEY
{
	eKEYBOARD_TAB	   = 1025,
	eKEYBOARD_CTRLLEFT = 1026,
	eKEYBOARD_CTRLRIGHT= 1071,
	eKEYBOARD_ALTRIGHT = 1070,
	eKEYBOARD_SPACEBAR = 1028,
	eKEYBOARD_NUMPAD0  = 1074,
};

const unsigned int SHORTCUT_ICON_BASE_INDEX = 1000000;

class cShortcutWindow : public cUIWindow 
{
public:
	cShortcutWindow();
	virtual ~cShortcutWindow();
	
	virtual void Open();
	virtual void Close();

	void SetShortcutIcon( int shortcutIndex );						 /// 해당 shortcutIndex 에 기본 할당된 아이콘을 그림.
	void UpdateKeyboardIcon( int keyboardIndex, int shortcutIndex ); /// 해당 keyboradIndex 에 shortcutIndex 아이콘이 할당됨.
	void UpdateAllIcon(){};

	void ClearKeyboardIcon( int iconIndex );
	void ClearShortcutIcon( int iconIndex );
	void ClearAllKeyboardIcon();
	void ClearAllShortcutIcon();

	bool UpdateSpecialKeyRect();
	cUINode* GetSpecialKeyNode( const cUIPos& pos );

	void HideSelectImage(); ///선택 이미지 지우기
	void UpdateSelect( int iconID );

	void SetDefaultShortcutIcon();
	

protected:
	virtual void OnIconClicked( cUINode* , unsigned int );
	virtual void OnIconDragged( cUINode* , unsigned int , const cUIPos& , bool );
	virtual void HideESC( bool onsound = true );

private:
	bool OnCreate(cUINodeProperty* pproperty );
	void OnCommand(cUINode* , unsigned int );
	void OnShow();
	void OnHide();
	void UpdateSkin();
	void UpdateRect();
	void OnRender( cUIFontItemKeeper* pKeeper );


private:
	typedef tPointerHashMap< int, void* > sIconMap;
	sIconMap mShortcutIcon;
	sIconMap mKeyboardIcon;

	/// 선택된 아이콘 표시 
	cBaseIcon*  mSelectIcon;
	cUIImage*	mpSelectImage;
	bool		mShowSelect;

	/// 특수 아이콘들 영역(다른 아이콘과 크기가 다른 Tab, Ctrl, Alt, Space Bar, 숫자키패드 0 )
	cUIRect mTabAbsoluteRect;
	cUIRect mRCtrlAbsoluteRect;
	cUIRect mLCtrlAbsoluteRect;
	cUIRect mRAltAbsoluteRect;
	cUIRect mSpaceBarAbsoluteRect;
	cUIRect mNumPad0AbsoluteRect;

};
