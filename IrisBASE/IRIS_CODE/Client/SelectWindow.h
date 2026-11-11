/* ==========================================================================
* 파    일 : SelectWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-04
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cButton;
class cLabel;

/// 선택된 캐릭터 정보 윈도우
class cSelectInfoWindow : public cUIWindow
{
public:
	cSelectInfoWindow();
	virtual ~cSelectInfoWindow();

	void Open();

	void SetLevel( int level );
	void SetName( LPCTSTR name );
	void SetJob( LPCTSTR job );
	void SetMapName( LPCTSTR mapName );
	void ClearInfo();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void UpdateSkin();

private:
	/// 레벨 
	cLabel*		mpLevel;

	/// 이름
	cLabel*		mpName;

	/// 직업
	cLabel*		mpJob;

	/// 접속지역
	cLabel*		mpMap;

	/// 초기화면 가기 버튼
	cButton*	mpBack;
};


/// 캐릭터 선택 윈도우 
class cSelectWindow : public cUIWindow
{
public:
	cSelectWindow();
	virtual ~cSelectWindow();

	void Open();

	enum
	{
		eShow_None = 0,
		eShow_StartButton,
		eShow_MakeButton,
	};

	void ShowFrame( unsigned char showOpt );

	virtual void SetEnabled( bool enabled );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );

private:
	/// 접속 버튼
	cButton*	mpConnect;

	/// 생성 버튼
	cButton*	mpMake;

	/// 이전/다음 캐릭터 선택
	cButton*	mpBeforeCharacter;
	cButton*	mpNextCharacter;
};

/// 캐릭터 버튼 윈도우
class cSelectButtonWindow : public cUIWindow
{
public:
	cSelectButtonWindow();
	virtual ~cSelectButtonWindow();

	void Open();

	void ShowDelete( bool show );
	void ShowDeleteCancel( bool show );
	void SetEnableButtons( bool enabled );
	void ShowButtons( bool show );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );

private:
	cButton*	mpDelete;
	cButton*	mpDeleteCancel;
	cButton*	mpBegin;
	cButton*	mpExit;
};
