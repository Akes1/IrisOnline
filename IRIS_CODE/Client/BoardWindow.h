/* ==========================================================================
* 파    일 : BoardWindow.h
* 목    적 : 게시판 
* 작 성 자 : 
* 작 성 일 : 2009-09-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cBoardListWindow;
class cBoardQuestWindow;

enum 
{
	eBOARD_LIST = 0,
	eBOARD_QUEST,

	eBOARD_MAX,
};

class cBoardWindow : public cUIWindow
{
public:
	cBoardWindow();
	virtual ~cBoardWindow();

	virtual void Open() {}

	void Clear();
	virtual void Show( unsigned long npcIndex );
	virtual void Show( bool onsound ) { assert(0); }

	void ChangePage( unsigned char page );

	void AddQuest( unsigned long questIndex );

	// 리스트 선택
	void SelectList( unsigned int selectIdx );
	void SelectChanged( unsigned int selectIdx ) { mSelectIndex = selectIdx; }
	
	cBoardListWindow* GetBoardListWindow() { return mpListWindow; }
	tArray<unsigned long>* GetQuestArr() { return &mQuestArr; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void UpdateSkin();

	void OnShow();
	void OnHide();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	// 목록 윈도우
	cBoardListWindow*	mpListWindow;

	// 퀘스트 정보 윈도우
	cBoardQuestWindow*	mpQuestWindow;

	// 퀘스트 정보 관리
	typedef tArray<unsigned long> cQuestArr;
	cQuestArr	mQuestArr;

	//
	unsigned char mCurrentPage;

	// 선택된 배열인덱스
	unsigned int mSelectIndex;
};