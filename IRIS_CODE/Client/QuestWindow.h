/* ==========================================================================
* 파    일 : QuestWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-03-19
* 주의사항 :
*===========================================================================*/
#pragma once
#include "QuestDefine.h"
#include "UIWindow.h"
#include "Icon.h"
#include "ListFolderBox.h"

class cUIImage;
class cIcon;
class cLabel;
class cTextBox;
class cButton;
class cCheckBox;
class cQuestListFolder;

enum eQuestRewardBackImg
{
	eQUEST_REWARD_EXP_IMG,
	eQUEST_REWARD_SXP_IMG,
	eQUEST_REWARD_GOLD_IMG,
	eQUEST_REWARD_SP_IMG,

	eQUEST_REWARD_BASE_ITEM_IMG,
	eQUEST_REWARD_SELECT_ITEM_IMG,

	eQUEST_REWARD_IMG_MAX,
	eQUEST_REWARD_LABEL_MAX = eQUEST_REWARD_SP_IMG + 1,
};

struct sRewardIcon
{
	cIcon*		  icon;
	unsigned long index;	/// 스킬 or 아이템 인덱스
	bool		  select;	/// 선택되었는지 여부

	sRewardIcon()
	{
		icon = 0;
		index = 0;
		select = false;
	}
};

class cQuestWindow : public cUIWindow
{
public:
	cQuestWindow();
	virtual ~cQuestWindow();

	virtual void Open();
	virtual void Close();

	void Clear();

	/// 퀘스트 추가
    bool AddQuest( unsigned int arrIdx, unsigned long questIdx, bool complete, bool IsNewQuest = false );

	/// 갱신
	void SetCheck( unsigned int arrIdx, unsigned long questIdx, bool check );
	void SetCompleteQuest( unsigned int arrIdx, unsigned long questIdx );
	
	/// 퀘스트 설명 및 정보 업데이트
	void UpdateQuestInfo();				

	/// 진행중, 완료가능 퀘스트 갯수 갱신
	void UpdateCount( unsigned int progress, unsigned int complete );

	void SetShowInfo( bool show );
	bool IsShowInfo();

	//
	void UpdateSelect();

	void GetHideMapCategorySet( tHashSet<unsigned int>& mapArr );
	void SetHideMapCategory( unsigned long mapIndex );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void UpdateRect();
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnCheckBoxChanged( cUINode* , unsigned int );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnListBoxChanged( cUINode* , unsigned int );
	void OnListBoxSelected(cUINode* , unsigned int );
	void OnIconHovered(cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnImageClicked( cUINode* , unsigned int );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void OnShow();
	void OnRender( cUIFontItemKeeper* pKeeper );

	bool SetQuestReward();	/// 보상정보 업데이트
	bool UpdateRewardIcon( cQuestReward* reward, sRewardIcon* icon );

	void SetSelectCount( unsigned int count );

	void ClearIcon();
	void ClearLabels(); /// 보상 정보( exp, sxp, gold, sp 초기화 )

protected:
	cQuestListFolder*	mpListBox;

	/// 보상정보
	sRewardIcon mDefaultIcon[5];
	sRewardIcon mSelectIcon[5];
	
	cLabel* mpRewardLabel[eQUEST_REWARD_LABEL_MAX]; /// exp, sxp, gold, sp 수치 표시
	cLabel* mpCount;							    ///선택 지급창에서 개수 표시

	/// 갯수 정보
	cLabel* mpCompleteCount;
	cLabel* mpProgressCount;

	/// 퀘스트 정보 박스
	cTextBox* mpDescBox;

	/// 
	cButton* mpDelete;
	cCheckBox* mpViewInfo;	// press : 켜진상태 no press : 꺼진상태

	cUIImage*	mpHighImage;
	cUIImage*	mpLowImage;

	/// 보상 이미지들 좌표
	cUINodeSkin* mpRewordBack[eQUEST_REWARD_IMG_MAX];
	cUIImage*	 mpRewordBackImg[eQUEST_REWARD_IMG_MAX];
	bool		 mIsShowBackImg[eQUEST_REWARD_IMG_MAX];

	/// 남은시간 출력 라인
	int	mTimeLine;
	unsigned long mSelectQuestIndex;
};

// 퀘스트용 리스트 폴더 박스
class cPlaneObject;
class cQuestListFolder : public cListFolderBox
{
public:
	cQuestListFolder();
	virtual ~cQuestListFolder();

	void Clear();

	/// extraData를 가지고있는 하위열을 찾아서 리턴 - 접혀있으면 리턴 안함
	int FindColumnByExtra( unsigned short mapIndex, int arrIndex );

	/// 선택 이미지 좌표 업데이트 
	virtual void UpdateSelectImage();

	void ShowOn( bool set ) { mShowOn = set; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateRect();

private:
	cUINodeSkin*	mpOnSkin;
	cPlaneObject*	mpOnImage;

	// 추적 on 이미지 
	bool			mShowOn;
};