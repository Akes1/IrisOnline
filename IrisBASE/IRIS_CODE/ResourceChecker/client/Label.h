/* ==========================================================================
* 파    일 : Label.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-10-12
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UINode.h"
#include "UINodeData.h"

class cLabel : public cUINode
{
public:
	cLabel( eUINodeType type = eUINODE_LABEL );
	virtual ~cLabel();
	
protected:
	/// 텍스트저장 
	typedef tArray<cStringT> cTextLen;
	cTextLen mTextArr;

	/// 텍스트
	cStringT mText;

	/// 정렬할때 기준이 되는 노드
	cUINode* mDescNode;

	/// 텍스트 정렬
	int mTextAlign;

	/// 텍스트 출력위치 
	int mTextPosX, mTextPosY;

	/// 텍스트 수동 출력위치
	cUIPos mEditPos;

	/// 컬러
	unsigned long mTextColor;

	/// 자신의 스킨 저장
	cUINodeSkin* mpSkin;

	//
	unsigned short mFontIndex;
};

//////////////////////////////////////////////////////////////////////////
/// Label 스킨 ( static 스킨 )
class cLabelSkin : public cUINodeSkin
{
public:
	cLabelSkin( eUINodeType type = eUINODE_LABEL );
	virtual ~cLabelSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 텍스트
	cStringT mText;
	/// 텍스트 정렬
	int mTextAlign;
	/// 텍스트 출력위치설정
	cUIPos mEditPos;
};
