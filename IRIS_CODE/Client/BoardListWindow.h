/* ==========================================================================
* 파    일 : BoardListWindow.h
* 목    적 : 게시판 
* 작 성 자 : 
* 작 성 일 : 2009-09-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

const int MAXTITLE_LEN = 20;

class cListBox;
class cBoardListWindow : public cUIWindow
{
public:
	cBoardListWindow();
	virtual ~cBoardListWindow();

	void UpdateQuestList();

	void SelectRow( int row );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnListBoxChanged( cUINode* , unsigned int );
	void OnListBoxSelected( cUINode* , unsigned int );
	void OnCommand( cUINode* , unsigned int );
	
	void AddQuest( unsigned long questIndex );

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	//
	cListBox*	mpListBox;

	// 수락 버튼
	cUINode*	mpAccept;
};