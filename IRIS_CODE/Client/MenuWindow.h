/* ==========================================================================
* 파    일 : MenuWindow.h
* 목    적 : 하단메뉴
* 작 성 자 : 김가람
* 작 성 일 : 2007-02-20
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

enum eMenuType
{	
	eMENU_RECEIPT,
	eMENU_HERO,
	eMENU_COMMU,
	eMENU_OPTION,
	eMENU_LIFE,
	eMENU_TWITTER,

	eMENU_MAX,
};

/// 하단 버튼 메뉴 윈도우
class cMenuButtonWindow : public cUIWindow
{
public:
	cMenuButtonWindow();
	virtual ~cMenuButtonWindow();

	void AddMenu( unsigned int i, cUIWindow* p );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );

private:
	cUIWindow*	mpMenu[eMENU_MAX];
};

/// 메뉴 캐릭터 윈도우
class cMenuHeroWindow : public cUIWindow
{
public:
	cMenuHeroWindow();
	virtual ~cMenuHeroWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnMouseOtherClick( const cUIPos& pos );
	bool SetSkin( const cUINodeSkin* pskin );
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	cUINode*	mpReceipt;
};

/// 메뉴 생활 윈도우
class cMenuLifeWindow : public cUIWindow
{
public:
	cMenuLifeWindow();
	virtual ~cMenuLifeWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnMouseOtherClick( const cUIPos& pos );
	bool SetSkin( const cUINodeSkin* pskin );
	void UpdateSkin();
};

/// 메뉴 커뮤니티 윈도우
class cMenuCommuityWindow : public cUIWindow
{
public:
	cMenuCommuityWindow();
	virtual ~cMenuCommuityWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnMouseOtherClick( const cUIPos& pos );
	bool SetSkin( const cUINodeSkin* pskin );
	void UpdateSkin();
};

/// 메뉴 옵션 윈도우
class cMenuOptionWindow : public cUIWindow
{
public:
	cMenuOptionWindow();
	virtual ~cMenuOptionWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnMouseOtherClick( const cUIPos& pos );
	bool SetSkin( const cUINodeSkin* pskin );
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	cUINode*	mpChange;
	cUINode*	mpExit;
};
