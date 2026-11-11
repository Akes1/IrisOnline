#pragma once
#include "uiwindow.h"
#include "Icon.h"
#include "Button.h"

class cSkillTree;
class cSkillIcon;

/// eMBEVENT_SKILL_BUY 메시지 박스에 전해줄 추가 정보
struct sExtraData 
{
	unsigned int  skillIdx; 
	unsigned char skillStep;
};

class cSkillSheetWindow : public cUIWindow
{
public:
	cSkillSheetWindow( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cSkillSheetWindow();

	void Clear();

	void InitSkillSheetInfo( unsigned long jobIdx, cSkillTree* pTree );
	void InitSkillData();

	cSkillIcon*   GetSkillIcon( unsigned long idx );
	
	void		  ShowSkillUpButton();
	void		  HideSkillUpButton();

	unsigned long GetJobIdx() { return mJobIdx; }

	void UpdateTree();
	
	void ClearSkillCoolTime( unsigned long exceptSkillIdx ); 

protected:
	void OnIconDragged( cUINode* pcaller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconDoubleClicked( cUINode* pcaller, unsigned int id );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool /*shiftPress*/ );
	void UpdateSkin();
	void OnCommand( cUINode* pcaller, unsigned int id );
 	void OnNetLock( int lockTry );
 	void OnNetUnLock( int lockTry );

protected:
	// 스킬 아이콘 관리
	typedef tPointerHashMap<unsigned long, void*> cSkillIconList;
	cSkillIconList mManagedIcon;

	// 레벨 업 버튼 관리자
	typedef tPointerHashMap<unsigned long, void*> cLevelupButtonMap;
	cLevelupButtonMap mLevelupButtonMap;

	NiTexture*			mBackTex;

	unsigned long		mJobIdx;

	cSkillTree*			mpTreeInfo;

	sExtraData          mExtraData;
};	

inline cSkillIcon* cSkillSheetWindow::GetSkillIcon( unsigned long idx )
{
	return (cSkillIcon*)mManagedIcon.GetAt( idx );
}

/////// 생활스킬관련 시트
class cCommuSheetWindow : public cUIWindow
{
public:
	cCommuSheetWindow( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cCommuSheetWindow();

	/// 커뮤니티 스킬 발동 요청
	void RequestCommuSkill( unsigned long skillIndex );

	void RequestLifeCommand( unsigned long comIdx, LPCTSTR value1, LPCSTR value2 );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconDoubleClicked( cUINode* caller, unsigned int id );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool /*shiftPress*/ );
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

protected:
	typedef tPointerHashMap<unsigned long, void*> cCommuIconList;
	cCommuIconList mManagedIcon;
};

/// LButtonDown 메시지를 받았을 경우 해당 버튼이 탭 버튼이 내려가는 것을 방지하기위해 만든 버튼 
class cLevelButton : public cButton
{
protected:
	void OnLButtonDoubleClick( const cUIPos& pos );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
};
