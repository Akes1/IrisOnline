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

	/// 텍스트를 얻는다
	LPCTSTR GetText();

	/// 텍스트 길이 획득
	int GetLength();

	/// 현재 텍스트를 입력중이거나, 텍스트가 입력되어있는지 검사
	bool IsEdit();

	/// 텍스트를 초기화한다
	void Clear();

	void SetFocus();

	/// 에디트 자체에 최대 들어갈수 있는 문자열 길이
	void SetMaxEditLength( int maxlen );

	bool IsComposition() { return mCompFlag; }

protected:
	virtual bool OnCreate( cUINodeProperty* property = 0 );
	/// 영문 입력시 처리
	virtual void OnChar( unsigned int wparam );
	virtual void OnImeNotify( unsigned int wparam );
	virtual void OnImeComposition( unsigned int wparam, unsigned int lparam );
	virtual void OnImeEndComposition( unsigned int wparam, unsigned int lparam );
	/// 키 다운 시 처리
	virtual void OnKeyDown( eKeyCode code );
	/// 입력받은 문자가 유효한지 검사
	virtual bool IsValid( TCHAR c );

	/// 텍스트의 쓰레기 값을 없앤다.
	void ClearText( int index );

	/// 1 byte 문자 삽입 
	void AddChar( LPTSTR word );

	/// 조합중인 한글 삽입
	void AddCompWord( LPTSTR word, int len );

	/// 문자열에 기호등이 같이 삽입됬을 경우 처리하는 함수
	void AddString( LPTSTR string, int len );

	/// 캐럿을 중심으로 오른쪽 문자열을 얻는다
	int GetRightString( LPTSTR dest, unsigned int maxDestLen );

	/// 캐럿을 문자열 처음으로 옮긴다
	void CaretMoveHome();

	/// 캐럿을 문자열 끝으로 옮긴다
	void CaretMoveEnd();

	/// 캐럿을 왼쪽으로 한 자 옮긴다. (수정) 0226 웅주: 숫자 입력 시 쉼표를 넘기기 위해 오버라이딩
	virtual void CaretMoveLeft();

	/// 캐럿을 오른쪽으로 한 자 옮긴다. (수정) 0226 웅주: 숫자 입력 시 쉼표를 넘기기 위해 오버라이딩
	virtual void CaretMoveRight();

	/// 백스페이스 키 누름, (수정) 0226 웅주: 숫자 입력 시 쉼표를 넘기기 위해 오버라이딩
	virtual void KeyDownBackSpace();

	/// Delete 키 누름. (수정) 0226 웅주: 숫자 입력 시 쉼표를 넘기기 위해 오버라이딩
	virtual void KeyDownDelete();

	/// 한줄에 들어갈수 있는 문자수 세팅
	void SetOneLineWidth( int linelen );

	/// ctrl + c 누름
	void KeyDownCopy();

	/// ctrl + v 누름
	void KeyDownPaste();

	/// 드래그 영역이 있는 상태에서 backspace 다운 혹은 delete key 다운
	void DeleteSell();

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

	// 조립중
	cStringT mCompText;

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

inline LPCTSTR cUIIMENode::GetText()
{
	return mText;
}

inline void cUIIMENode::SetOneLineWidth( int linelen )
{
	mOneLineWidth = linelen;
}

inline void cUIIMENode::SetMaxEditLength( int maxlen )
{
	mMaxEditLength = maxlen;
}

inline int cUIIMENode::GetLength()
{
	return mTextLength;
}

inline bool cUIIMENode::IsEdit()
{
	if( mTextLength > 0 || mCompFlag )
		return true;
	return false;
}
