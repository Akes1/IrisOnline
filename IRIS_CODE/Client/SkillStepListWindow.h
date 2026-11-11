#pragma once
#include "uiwindow.h"

class cSkillStepListBox;
class cSkillStepListWindow : public cUIWindow
{

public:
	cSkillStepListWindow( eUINodeType type = eUINODE_WINDOW );
	virtual ~cSkillStepListWindow();

protected:

	virtual bool OnCreate( cUINodeProperty* pProperty );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual bool SetSkin(const cUINodeSkin* pskin );
	virtual void OnListBoxSelected( cUINode* , unsigned int );
	virtual void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	virtual void OnIconDoubleClicked( cUINode* caller, unsigned int id );
	virtual void Open();

public:

	void			Clear();

	//				해당 N차 직업의 스킬을 Clear 후 재 삽입하여 설정한다.
	void			UpdateSkill();

	//				해당 캐릭터의 모든 기술을 Clear 후 재 설정
	//void			UpdateAllSkill();

	//				리스트 박스에 스킬 아이콘 설정 재 셋팅을 요청
	void			UpdateIConListInfo();

	//				현재 직업 인덱스 셋팅 또는 요청, 직업 갯수 요청
	void			SetCurrentJobStep( unsigned char jobStep );
	unsigned char	GetCurrentJobStep() const					{ return mCurrentJobStep;	}

	//				스킬 갱신 여부
	void			ResetSkillInfo() { mUpdateSkillInfo = true; }

protected:

	unsigned char		mCurrentJobStep;		// 현재 직업 탭에 대한 Index
	cSkillStepListBox	*mpSkillStepListBox;	// 단계별 스킬 리스트 박스

	bool				mUpdateSkillInfo;		// 스킬 갱신 여부

};
