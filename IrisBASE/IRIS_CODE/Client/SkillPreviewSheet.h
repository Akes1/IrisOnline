/* ==========================================================================
* 파    일 : SkillPreviewSheet.h
* 목    적 : 스킬 찍어보기 시트
* 작 성 자 : 김가람
* 작 성 일 : 2009-08-27
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cButton;
class cSkillTree;
class cSkillIcon;
class cLevelButton;

// 버튼 묶음
struct sLevelButtons
{
	cLevelButton* upButton;
	cLevelButton* downButton;
};

class cSkillPreviewSheet : public cUIWindow
{
public: 
	cSkillPreviewSheet( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cSkillPreviewSheet();

	void Clear();

	void SetSkillTree( unsigned long jobIndex, cSkillTree* tree );
	void InitSkillTree();
	void SyncSkillTree();	// hero의 스킬과 동기화

	void ShowSkillUpButton();
	void HideSkillUpButton();

	unsigned long GetJobIndex(){ return mJobIndex; }

protected:
	void OnCommand( cUINode* , unsigned int );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool );
	void UpdateSkin();

	cSkillIcon*   GetSkillIcon( unsigned long idx );

	void OnShow();

private:
	// 스킬 아이콘 관리
	typedef tPointerHashMap<unsigned long, void*> cSkillIconList;
	cSkillIconList mManagedIcon;

	// 레벨 버튼 관리자 (sLevelButtons)
	typedef tPointerHashMap<unsigned long, void*> cLevelupButtonMap;
	cLevelupButtonMap mLevelupButtonMap;

	// 스킬 트리
	cSkillTree*			mpTreeInfo;

	// 배경 이미지
	NiTexture*			mBackTex;

	//
	unsigned long		mJobIndex;
};

inline cSkillIcon* cSkillPreviewSheet::GetSkillIcon( unsigned long idx )
{
	return (cSkillIcon*)mManagedIcon.GetAt( idx );
}
