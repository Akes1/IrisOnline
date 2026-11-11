/* ==========================================================================
* 파    일 : NpcWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-06-19
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Npc_Common.h"

struct sQuestStatus
{
	unsigned long questIdx;		/// 퀘스트 인덱스
	unsigned int status;		/// 진행상태
};

// 기능 버튼 사이 갭
const unsigned int FUNCBUTTON_GAPW = 4;
const unsigned int FUNCBUTTON_GAPH = 2;

class cUINpcTelling;
class cButton;
class cLabel;
class cTextBox;
class cListBox;
class cUIImage;

/// Npc 윈도우
class cNpcWindow : public cUIWindow
{
public:
	cNpcWindow();
	virtual ~cNpcWindow();

	virtual void Open();
	virtual void Close();

	virtual	void Show( bool onsound = true ) { assert(0); }
	virtual	void Show( unsigned int npcClassIdx, unsigned long npcIndex, LPCTSTR npcName );

	/// 새로운 퀘스트 리스트 삽입
	virtual	void Clear();
	virtual	void AddQuest( unsigned long questIdx, unsigned int status );
	virtual	void ShowDefaultPage();

	/// 장면 정보 세팅
	virtual	bool UpdateDialog( bool rewardItemSeleted = false );

	///
	virtual	void HideESC( bool onsound );

	virtual void NetLock( int lockTry = 0 );
	virtual void NetUnLock( int lockTry = 0 );

protected:
	virtual	bool OnCreate( cUINodeProperty* property );
	virtual	void OnRender( cUIFontItemKeeper* pKeeper );
	virtual	void UpdateSkin();
	virtual	void OnShow();
	virtual	void OnHide();
	virtual	void OnCommand( cUINode* node, unsigned int id );
	virtual	void OnListBoxChanged( cUINode* , unsigned int );
	virtual	void OnNetLock( int lockTry );
	virtual	void OnNetUnLock( int lockTry );

	virtual	void SelectQuest();

	/// 대사 장면 정보 세팅
	virtual	bool SetDialog( unsigned long quetIdx, unsigned int status );
	virtual	void UpdateScene();

	// 기능 버튼 위치 갱신
	virtual	void ShowFuncButton();
	
protected:
	/// 데이터 삭제용
	typedef tPointerArray<void*> cDelArr;
	cDelArr	mDelArr;

	/// 이름 라벨
	cLabel* mNameLabel;

	/// 대사 상자
	cTextBox* mpTextBox;

	/// 퀘스트 리스트
	cListBox* mpQuestList;

	cUINode* mpClose;

	/// 기능 버튼
	cUINode* mpFuncButton[NPC_FUNC_MAX];

	/// Npc 아이디
	unsigned int mNpcClassIndex;	/// 클래스 인덱스
	unsigned long mNpcIndex;		/// 오브젝트 인덱스

	/// 대사
	cUINpcTelling* mTelling;

	/// 뒷 배경 지우기 플래그
	bool	mHideBack;

	///	퀘스트 대화중인지 여부
	bool	mIsUseQuest;

	/// 현재 대사정보와 관련있는 퀘스트
	unsigned long mQuestIndex;
	unsigned int mStatus;
	unsigned int mNextPageIdx;

	/// NPC 이미지
	cUIImage*	mSceneImage;
	cUINodeSkin*	mSceneSkin;
	unsigned long mSceneIndex;
	bool	mShowScene;
};
