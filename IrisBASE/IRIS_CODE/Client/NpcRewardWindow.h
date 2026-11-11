/* ==========================================================================
* 파    일 : NpcRewardWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-06-20
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "QuestWindow.h"

class cLabel;
class cUIImage;
struct sRewardIcon;

/// 보상 배경 이미지 관리
struct sRewardBack
{
	cUIImage*		mpImage;
	cUINodeSkin*	mpNodeSkin;
	bool			mShow;

	sRewardBack()
	{
		mpImage = 0;
		mpNodeSkin = 0;
		mShow = false;
	}
};

enum eRewardBack
{
	eBACK_DEFAULT = 0,
	eBACK_SELECT,
	eBACK_EXP,
	eBACK_SXP,
	eBACK_MONEY,
	eBACK_SKILLPOINT,

	eBACK_MAX,
};

class cNpcRewardWindow : public cUIWindow
{
public:
	cNpcRewardWindow();
	virtual ~cNpcRewardWindow();

	virtual void Open();
	virtual void Close();

	/// 퀘스트 정보 세팅
	bool SetQuestReward( long questIdx );
	unsigned int GetSelectCount() { return mCount; }

	/// 보상 선택 가능한지 세팅
	void UpdateUseReward( long questIdx );

	sRewardIcon* GetSelectData() { return mSelectIcon; }

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnIconHovered(cUINode* caller, unsigned int id, const cUIPos& pos );
	void OnIconClicked( cUINode*, unsigned int );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void UpdateRect();
	void OnHide();

	bool UpdateReward( cQuestReward* reward, sRewardIcon* icon );

	void SetExp( unsigned int exp );
	void SetSxp( unsigned int sxp );
	void SetMoney( unsigned long money );
	void SetSkillPoint( unsigned int sp );
	void SetSelectCount( unsigned int count );

	void ClearIcon();

protected:
	/// 보상 선택 가능 여부
	bool	mUseReward;

	cLabel* mpCount;
	cLabel* mpEXP;
	cLabel* mpSXP;
	cLabel* mpSkillPoint;
	cLabel* mpMoney;

	/// 각 배경이미지
	sRewardBack*	mBackData[eBACK_MAX];
	
	/// 보상정보
	sRewardIcon mDefaultIcon[5];
	sRewardIcon mSelectIcon[5];

	/// 선택용 이미지 
	cUIImage* mpSelectImg[5];
	cUISize	mSelectSize;
	unsigned int mMaxCount;	/// 선택 가능 갯수
	unsigned int mCount;	
};
