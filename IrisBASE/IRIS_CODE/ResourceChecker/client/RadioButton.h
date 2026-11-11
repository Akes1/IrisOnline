/* ==========================================================================
* 파    일 : RadioButton.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Button.h"
#include "UINodeData.h"

class cRadioButton : public cButton
{
public:
	cRadioButton( eUINodeType type = eUINODE_BUTTON );
	virtual ~cRadioButton();
 
private:
	/// 라디오 버튼의 그룹 번호 지정 
	int mGroupNum;
	cRadioButton* mpPrev;
	cRadioButton* mpNext;

	unsigned long mDownSoundIdx;
};

////////////////////////////////////////////////////////////////////////////
/// 라디오 버튼 스킨
class cRadioButtonSkin : public cUINodeSkin
{
public:
	cRadioButtonSkin( eUINodeType type = eUINODE_RADIOBUTTON );
	virtual ~cRadioButtonSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 텍스트
	cStringT mText;

	/// 버튼의 이미지 변환좌표 저장 
	cUIPos mUpPos;
	cUIPos mDownPos;
	cUIPos mUpOverPos;

	/// 각 버튼상태에 따른 텍스트 컬러 
	unsigned long mUpColor;
	unsigned long mDownColor;

	/// 라디오 버튼의 그룹 번호 
	int mGroupNum;

	unsigned long mDownSoundIdx;
};

