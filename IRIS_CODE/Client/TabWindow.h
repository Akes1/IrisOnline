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

/// 탭 윈도우
class cTabWindow : public cUIWindow
{
public:
	cTabWindow( eUINodeType type = eUINODE_TABWINDOW );
	virtual ~cTabWindow();

	virtual void Close();

	/// 탭 쉬트
	void AddSheet( cUIWindow* sheet );

	/// 탭 선택
	void AddTab( cTabButton* tab );
	virtual void SetCurrent( unsigned int index );
 
	cUIWindow* GetCurrentSheet() { return mCurrentSheet; }

	cUIWindow* GetSheetByID( int nID );
	
protected:
	virtual bool SetSkin( const cUINodeSkin* pskin );


	/// 생성
	virtual bool OnCreate( cUINodeProperty* pproperty );

	/// 이벤트 처리
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnTabButtonPressed( cUINode* caller, unsigned int id );
	
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
