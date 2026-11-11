/* ==========================================================================
* 파    일 : TutorialBookWindow.h
* 목    적 : 게시판 
* 작 성 자 : 
* 작 성 일 : 2009-10-22
* 주의사항 :
*===========================================================================*/
#pragma once
#include "NpcWindow.h"

// 튜토리얼 book npc 창
class cNpcTutorialWindow : public cNpcWindow
{
public:
	cNpcTutorialWindow();
	virtual ~cNpcTutorialWindow();

	void Close();
	void Show( unsigned int npcClassIdx, unsigned long npcIndex, LPCTSTR npcName );

	void ShowDefaultPage();
	bool UpdateDialog( bool rewardItemSeleted = false );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* , unsigned int );
	void OnListBoxChanged( cUINode* , unsigned int );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void UpdateSkin();
	void OnShow();
	void OnHide();

	void SelectQuest();

	bool SetDialog( unsigned long quetIdx, unsigned int status );
	void UpdateScene();
};
