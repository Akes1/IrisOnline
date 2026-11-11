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
//class cEditBox;
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
	
	/// 새로운 라인 추가용 
	int AddNewRow( LPCTSTR text, unsigned long textcolor, sInventory* inventory = 0 );

	/// 기존라인 연결 추가용
	void AddPasteRow( LPCTSTR text, unsigned long textcolor, sInventory* inventory = 0 );

	/// 기능성 텍스트 추가 ( npc_note, quest_note 등 )
	bool AddFucnText( cUIFuncTextNode* textNode, cUIFuncTextParam* textParam );

	/// 라인 삽입 ( 빈 공백 )
	void AddEmptyRow( unsigned int line );

	/// 해당 라인의 텍스트 갱신
	void UpdateRowText( int line, LPCTSTR text );

	/// 텍스트박스에 들어가는 최대 행수 
	unsigned int GetMaxRowInPage();

	/// 행높이 가져오기
	unsigned int GetRowHeight() const;

	/// 텍스트박스의 사이즈 변화에 따른 모든 정보 갱신 
	void UpdateTextBox( bool add );

	/// 텍스트박스 스타일 조정 관련부
	/// 스크롤바 Y 위치 설정 ( addh 값만큼 내려 찍기 )
	void SetScrollbar( unsigned int addh );

	/// 텍스트 출력위치를 아래서부터로 조정 ( 플래그값)
	void SetDrawTextBottom( bool down );

	/// 텍스트입력시 스크롤바를 맨아래로 갱신
	void SetScrollBottom( bool bottom );

	void SetChatMode( bool bChat );
	
protected:
	/// 이벤트 처리
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );   
	virtual void OnMouseMove( const cUIPos& pos ) {}
	virtual void OnLButtonDown(const cUIPos& pos, bool ctrl, bool alt, bool );
	virtual void OnMouseWheel( const cUIPos& pos, int wheel );
	virtual void OnScrollbarMoved( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateSkin();
	virtual void UpdateRect();

	/// 자식생성
	bool OnCreate( cUINodeProperty* pproperty );

	/// 버튼 상태 체크 
	void CheckButtonEvent( unsigned int id );

	/// 사이즈가 변하는 스크롤바를 그려줌.
	void UpdateScrollBarButton();

	/// topindex에 따른 스크롤바 위치 변화 
	void UpdateScrollBarPos();

	/// 텍스트 좌표 업데이트 
	void UpdateText();

	/// 파싱
	bool ParseCommand( cUITextLexerT& lexer );

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

	bool	mIncreseTopRowIdx;
	bool	mChatMode;
};

inline void cTextBox::SetDrawTextBottom( bool down )
{
	mDrawTextBottom = down;
}

inline void cTextBox::SetScrollBottom( bool bottom )
{
	mScrollBottom = bottom;
}

inline void cTextBox::SetChatMode( bool bChat )
{
	mChatMode = bChat;
	mScrollBottom = false;	
}

inline unsigned int cTextBox::GetRowHeight() const
{
	return mRowHeight;
}

inline unsigned int cTextBox::GetMaxRowInPage()
{
	mMaxRowInPage = (GetAbsoluteRect().GetHeight() / mRowHeight);
	return mMaxRowInPage;
}


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


