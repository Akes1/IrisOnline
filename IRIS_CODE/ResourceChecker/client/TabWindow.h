/* ==========================================================================
* 파    일 : TabWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-16
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cTabButton;
class cUIWindow;

/// 탭 윈도우
class cTabWindow : public cUIWindow
{
public:
	cTabWindow( eUINodeType type = eUINODE_TABWINDOW );
	virtual ~cTabWindow();

 	
protected:
	/// 탭버튼
	typedef tPointerArray<void*> cTabButtonArray;
	cTabButtonArray mTabButtonArray;

	/// 탭시트
	typedef tPointerArray<void*> cTabSheetArray;
	cTabSheetArray mTabSheetArray;

	/// 현재 탭
	cTabButton* mCurrentButton;
	cUIWindow* mCurrentSheet;
};


/////////////////////////////////////////////////////////////////////////////
class cTabWindowSkin : public cUIWindowSkin
{
public:
	cTabWindowSkin( eUINodeType type = eUINODE_TABWINDOW );
	virtual ~cTabWindowSkin();

	const tArray<cString>& GetSheetList() const;

public:
	/// 탭시트 스킨 리스트
	typedef tArray<cString> cSheetList;
	cSheetList mSheetList;

public:
	/// 눌림 여부
	bool mPressed;
};

inline const tArray<cString>& cTabWindowSkin::GetSheetList() const
{
	return mSheetList;
}
