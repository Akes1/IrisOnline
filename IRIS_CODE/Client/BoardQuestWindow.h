/* ==========================================================================
* 파    일 : BoardQuestWindow.h
* 목    적 : 게시판 
* 작 성 자 : 
* 작 성 일 : 2009-09-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cTextBox;
class cLabel;

// 보상 출력 위치좌표
enum
{
	ePOS1 = 0,
	ePOS2,
	ePOS3,
	ePOS4,

	ePOS_ITEM1,		// 기본 아이템
	ePOS_ITEM2,		// 선택 아이템

	ePOS_MAX,
};

enum
{
	eLABEL_EXP = 0,
	eLABEL_SXP,
	eLABEL_GOLD,
	eLABEL_SP,
	eLABEL_DEFAULT,		// 기본 아이템
	eLABEL_SELECT,		// 선택 아이템

	eLABEL_MAX,
};

class cIcon;
struct sBoardRewardIcon
{
	cIcon*		  icon;
	unsigned long index;	/// 스킬 or 아이템 인덱스
	int			  skinPosX;	/// 스킨 좌표
	int			  skinPosY;	/// 스킨 좌표

	sBoardRewardIcon()
	{
		icon = 0;
		index = 0;
		skinPosX = 0;
		skinPosY = 0;
	}
};

// 중앙정렬 위한 값
const int TEXT_X = 49;
const int TEXT_W = 59;

class cQuestReward;
class cBoardQuestWindow : public cUIWindow
{
public:
	cBoardQuestWindow();
	virtual ~cBoardQuestWindow();

	void SetPage( unsigned long questIndex );
	void UpdatePage( unsigned int selectIdx, unsigned int maxSize );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnIconHovered( cUINode* caller, unsigned int, const cUIPos& pos );
	void UpdateSkin();

	void Clear();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

	void SetReward();
	void SetDefaultReward( unsigned int i, unsigned long value, unsigned int posIdx );
	bool SetIconReward( cQuestReward* reward, sBoardRewardIcon* icon, unsigned int posIdx );

private:
	// 퀘스트 설명 박스
	cTextBox*		mpDescBox;

	// 페이지
	cLabel*			mpPage;

	// 
	cUINode*		mpPreviewButton;
	cUINode*		mpNextButton;
	cUINode*		mpAcceptButton;

	//
	cUIPos			mRewardPos[ePOS_MAX];
	cLabel*			mpReward[eLABEL_MAX];

	//
	sBoardRewardIcon		mDefaultIcon[5];
	sBoardRewardIcon		mSelectIcon[5];

	unsigned long mQuestIndex;
};