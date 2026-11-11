/* ==========================================================================
* 파    일 : TodayWordWindow.h
* 목    적 : 오늘의 한마디
* 작 성 자 : 김가람 
* 작 성 일 : 2009-01-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"


const unsigned long WORD_WHITE			= 0xFFFFFFFF;		//흰색		255, 255, 255
const unsigned long WORD_BROWN			= 0xFF501E00;		//갈색		80, 30, 0
const unsigned long WORD_RED			= 0xFFFF0000;		//빨간색	255, 0, 0
const unsigned long WORD_ORANGE			= 0xFFFF8000;		//주황색	255, 128, 0
const unsigned long WORD_YELLOW			= 0xFFFFFF00;		//노란색	255, 255, 0
const unsigned long WORD_BRIGHTGREEN	= 0xFF00FF00;		//연두색	0, 255, 0
const unsigned long WORD_GREEN			= 0xFF008000;		//녹색		0, 128, 0
const unsigned long WORD_BRIGHTBLUE		= 0xF00FFFFF;		//하늘색	0, 255, 255
const unsigned long WORD_BLUE			= 0xFF0000FF;		//파란색	0, 0, 255
const unsigned long WORD_PINK			= 0xFFFF00FF;		//분홍색	255, 0, 255
const unsigned long WORD_PURPLE			= 0xFF8000FF;		//보라색	128, 0, 255
const unsigned long WORD_BLACK			= 0xFF111111;		//검정색	0, 0, 0

const int COLOR_COUNT = 12;

class cPlaneObject;
class cEditBox;
class cTodayWordWindow : public cUIWindow
{
public:
	cTodayWordWindow();
	virtual ~cTodayWordWindow();

	virtual void Open();
	virtual void Close();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode*, unsigned int id );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnLButtonDoubleClick( const cUIPos& pos );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool );
	void UpdateRect();
	void OnShow();
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
    
	///
	void UpdateSelectImage();
	unsigned long GetWordColor( int arrIdx );

private:
	cUIRect			mColorRect[COLOR_COUNT];
	unsigned short	mSelectIndex;

	/// 선택이미지
	cPlaneObject*	mpSelectImage;

	///
	cUINode*		mpOkBtn;
	cEditBox*		mpEdit;
};