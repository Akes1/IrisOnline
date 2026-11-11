/* ==========================================================================
* 파    일 : ThemeEnterWindow.h
* 목    적 : 
* 작 성 자 : 조영인
* 작 성 일 : 2009-10-19
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Theme_Common.h"

class cListBox;
class cLabel;
class cButton;
class cComboBox;

struct MSG_SYN_THEME_READYQUESTION;

class cThemeEnterWindow : public cUIWindow
{
public:
	cThemeEnterWindow();
	virtual ~cThemeEnterWindow();

 	virtual void Open();
 	virtual void Close();

	void Clear();

	void SetThemeEnterInfo( MSG_SYN_THEME_READYQUESTION* pSynMsg );

protected:
	bool OnCreate( cUINodeProperty* pProperty );
	void OnCommand( cUINode* node, unsigned int id );
	void OnComboSelected( cUINode* caller, unsigned int id );
	void HideESC( bool onsound );

	virtual void OnRender( cUIFontItemKeeper* pKeeper );


private:

	/// 던젼 입장
	cButton* mpEnter;  

	/// 던젼 거절
	cButton* mpClose;   

	/// 테마던젼 이름
	cLabel*  mpThemeName;

	/// 테마던젼 요구 레벨
	cLabel*  mpThemeLevel;

	/// 테마던젼 최대 인원
	cLabel*  mpThemePeopleCnt;

	/// 테마던젼 초기화 시간
//	cLabel*  mpThemeRefreshTime;

	/// 테마던젼 요구 아이템
	cLabel*  mpThemeItem;

	/// 테마 귀속 상태
	cLabel*  mpThemeState;

	/// 테마 귀속상태일때 고유번호
	cLabel*  mpThemeUniqueIndex;

	/// 던젼 모드(난이도)
	cComboBox* mpThemeMode;

	/// 참여 인원 정보
	cListBox* mpMemberList; 

	unsigned short mThemeMapIndex;

	typedef tPointerHashMap< unsigned long, void* > cThemeEnterInfo;
	cThemeEnterInfo* mpThemeEnterInfo[eTHEME_MODE_MAX];
};
