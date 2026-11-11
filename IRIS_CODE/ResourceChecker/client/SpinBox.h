/* ==========================================================================
* 파    일 : SpinBox.h
* 목    적 : 스핀 컨트롤 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-24
* 주의사항 :
*===========================================================================*/
#pragma once
#include "SpinCtrl.h"
#include "UINodeData.h"

class cButton;
class cLabel;
class cSpinBox : public cSpinCtrl
{
public:
	cSpinBox( eUINodeType type = eUINODE_SPINBOX );
	virtual ~cSpinBox();
    
private:
	// 왼쪽 버튼 스킨 네임 
	cString mLeftButtonSkin;
	/// 오른쪽 버튼 스킨 네임
	cString mRightButtonSkin;
	/// 라벨 스킨 네임
	cString mLabelSkin;

	/// 왼쪽 오른쪽 버튼
	cButton* mpLeftButton;
	cButton* mpRightButton;

	/// 스핀박스의 라벨
	cLabel* mpLabel;

	/// 원소 인덱스
	int	mSelectedIdx;

	/// 스핀박스 유형 타입 ( rotation true: 계속 돈다 / false : 한방향만 간다 )
	bool mRotation;
};


//////////////////////////////////////////////////////////////////////////
class cSpinBoxSkin : public cUINodeSkin
{
public:
	cSpinBoxSkin( eUINodeType type = eUINODE_SPINBOX );
	virtual ~cSpinBoxSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 텍스트 정렬
	int mTextAlign;

	/// 스핀박스 타입
	bool mRotation;

	/// 왼쪽 버튼 스킨 네임 
	cString mLeftButtonSkin;
	/// 오른쪽 버튼 스킨 네임
	cString mRightButtonSkin;
	/// 라벨 스킨 
	cString mLabelSkin;
};

