/* ==========================================================================
* 파    일 : CheckBox.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.13
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UINode.h"
#include "UINodeData.h"

/// 체크 박스
class cCheckBox : public cUINode
{
public:
	cCheckBox( eUINodeType type = eUINODE_CHECKBOX );
	virtual ~cCheckBox();

protected:
	/// 체크된 이미지 좌표
	cUIPos mCheckPos;
	/// 체크 안된 이미지 좌표
	cUIPos mNoCheckPos;

	/// 텍스처 너비
	unsigned int mOriginalTexWidth, mOriginalTexHeight;

	/// 사운드인덱스
	unsigned long mCheckSoundIdx;
	unsigned long mNoCheckSoundIdx;
};

/////////////////////////////////////////////////////////////////////
/// 체크 박스 스킨
class cCheckBoxSkin : public cUINodeSkin
{
public:
	cCheckBoxSkin( eUINodeType type = eUINODE_CHECKBOX );
	virtual ~cCheckBoxSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 체크 눌림 여부
	bool mPressed;

	/// 체크된 이미지 좌표
	cUIPos mCheckPos;
	/// 체크 안된 이미지 좌표
	cUIPos mNoCheckPos;

	unsigned long mCheckSoundIdx;
	unsigned long mNoCheckSoundIdx;
};
