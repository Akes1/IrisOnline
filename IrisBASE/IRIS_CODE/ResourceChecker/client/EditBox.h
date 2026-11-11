/* ==========================================================================
* 파    일 : EditBox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-10-12
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIIMENode.h"
#include "UINodeData.h"

class cPlaneObject;
class cEditBox : public cUIIMENode
{
public:
	cEditBox( eUINodeType type = eUINODE_EDITBOX );
	virtual ~cEditBox();

protected:
	/// 문자열 드래그시 백 이미지 
	cPlaneObject* mSelectionImage;

	/// 드래깅 시작 x 좌표
	cUIPos mSelectionStartPos;

protected:
	/// 텍스처 디폴드 영역
	cUIRect mOriginalTexRect;

	/// 에디트 박스에서 글자 출력 시작위치
	cUIPos mEditingPos;

	/// 에디트의 위치 
	int mEditX, mEditY;

	/// 에디트탭인덱스 (탭이 눌렸을시에 포커스를 가지기 위한 할당된 번호 )
	int mEditTabIndex;
 
	/// copy & paste 권한 주기 여부 
	bool mCopyFlag;

	/// 텍스트 정렬
	int mTextAlign;
	
	/// 캐럿 시간 플래그 
	bool mCaretTimeFlag;

	/// 캐럿 시간용
	unsigned long mLastTick;

	/// 캐럿 플래그
	bool mShowCaretFlag;

	/// 실제로 에디트에 뿌려지는 텍스트 
	TCHAR mShowText[ MAX_TEXT_LENGTH ];

	/// 현재 출력되는 문자열 길이 
	int mShowLen;

	unsigned long mTextColor;
};

///////////////////////////////////////////////////////////////////////////
class cEditBoxSkin : public cUINodeSkin
{
public:
	cEditBoxSkin( eUINodeType type = eUINODE_EDITBOX );
	virtual ~cEditBoxSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 텍스트 
	cStringT mText;
	/// 에디트 위치
	cUIPos mEditPos;
	/// 텍스트 정렬 ( 0 : 왼쪽, 1 : 가운데 )
	int mTextAlign;
	/// 에디트에 수용할수 있는 최대 길이 지정 ( 110을 넘을수 없음 )
	int mMaxEditLength;
	/// 에디트 타입 
	unsigned int mEditKind;

	/// 글자가 출력될 width
	unsigned int mEditWidth;

public:
	/// 에디트박스 드래그 영역 이미지 
	NiTexture* mpSelectionTexture;
	sSkinInfo* mSelectionSkin;
};
