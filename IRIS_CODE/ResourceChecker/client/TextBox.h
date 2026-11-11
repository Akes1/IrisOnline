/* ==========================================================================
* 파    일 : TextBox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TextCtrl.h"
#include "UINodeData.h"

class cUITextLexerT;
class cScrollBar;
class cEditBox;
class cUIFuncTextNode;
class cUIFuncTextParam;
struct sInventory;

/// 스크롤바 위치 
enum eScrollbarPosType
{
	eRIGHT_SCROLLBAR,
	eLEFT_SCROLLBAR,
};

/// 문자열 상자
class cTextBox : public cTextCtrl
{
public:
	cTextBox( eUINodeType type = eUINODE_TEXTBOX );
	virtual ~cTextBox();

 	/// 텍스트 박스의 있는 내용을 모두 지운다 
 	void Clear();
 	

protected:
		/// 스크롤 스킨 네임 
    cString mScrollSkin;

	/// 스크롤바
	cScrollBar* mpScrollBar;

	/// 텍스트 출력 위치 
	int mTextPosX, mTextPosY, mNextPosY;

	/// 스크롤바를 위치타입 (텍스트박스기준)
	eScrollbarPosType mScrollbarPosType;

	/// 텍스트 출력위치를 아래부터 시작 
	bool mDrawTextBottom;

	/// 새 텍스트 입력시 스크롤바 위치 세팅
	bool mScrollBottom;

	/// 스크롤바 사이즈 보정 
	unsigned int mFixScrollH;

	/// 최대 행수
	unsigned int mMaxRowInPage;

	/// 아이템의 높이설정
	unsigned int mRowHeight;

	/// 표시 시작 행
	unsigned int mTopRowIdx;

	/// 글자색
	unsigned long mTextColor;
};

//////////////////////////////////////////////////////////////////////////////////
class cTextBoxSkin : public cUINodeSkin
{
public:
	cTextBoxSkin( eUINodeType type = eUINODE_TEXTBOX );
	virtual ~cTextBoxSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 스크롤바를 오른쪽에 둘것인지 여부 (텍스트박스기준으로 오른쪽)
	bool mScrollBarRightPos;

	/// 텍스트 박스 한줄 높이 
	unsigned int mRowHeight;

	/// 텍스트박스가 담을수 있는 최대 갯수
	int mTotalMaxNum;

	/// 스크롤 스킨 네임 
	cString mScrollSkin;
};


