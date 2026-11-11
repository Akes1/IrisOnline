/* ==========================================================================
* 파    일 : UIIMENode.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-10-31
* 주의사항 :
*===========================================================================*/
#pragma once
#pragma comment(lib, "imm32.lib")
#include "UINode.h"

const int MAX_TEXT_LENGTH = 512;

/// 에디트 종류
enum eEditKind
{
	eEDIT_NORMAL,		/// 일반 
	eEDIT_NOINPUT,		/// 입력만 안받음.
	eEDIT_NUMBER,		/// 숫자용
	eEDIT_ACCOUNT_ID,	/// ID 계정용
	eEDIT_ACCOUNT_PASS,	/// PASS 계정용 
	eEDIT_EXCEPT_SPECIAL /// 특수문자만 제외
};

class cUIIMENode : public cUINode
{
public:
	cUIIMENode( eUINodeType type );
	virtual ~cUIIMENode();

private:
	/// 핸들
	static HWND mHWnd;

public:
	/// shift key
	static bool mShiftKey;

	/// 컨트롤 키가 눌렷는지 저장
	static bool mControlKey;

protected:
	/// 에디트박스가 가진 고유의 문자열
	/// 유니코드 버전 : 2byte 문자배열, 길이는 1
	TCHAR mText[ MAX_TEXT_LENGTH ];

	/// 현재 조립중인 문자를 저장
	TCHAR mCompText[ MAX_TEXT_LENGTH ];

	/// 에디트 SHOW 텍스트 갱신여부
	bool mChangeText;

	/// 에디트박스가 가진 고유의 문자열 길이 ( byte )
	int mTextLength;

	/// 고유 문자열상의 캐럿 
	int mTextCaretX;

	/// 조합중임
	bool mCompFlag;

	/// 보정플래그 
	bool mFixFlag;

	/// 스크롤기능을 위한 인덱스( 화면에 뿌릴 텍스트의 시작인덱스)
	int mScrollIndex;

	/// 스크롤을 위한 캐럿( 화면상에 찍히는 캐럿 )
	int mScrollCaretX;

	/// 한줄에 들어갈수 있는 최대 문자 폭
	int mOneLineWidth;

	/// 에디트에 들어갈수 있는 최대 글자 바이트 수
	int mMaxEditLength;

	/// 드래그 시의 캐럿좌표 
	int mStartDragCaretX, mEndDragCaretX;

	/// 드래깅 플래그
	bool mDraggingFlag;

	bool mImeFlag;

	/// 에디트 종류 
	eEditKind mEditKind;
};
