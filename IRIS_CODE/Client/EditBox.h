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

	/// 에디트에 출력할 문자열 적용 ( 외부에서 들어온 문자열 )
	void SetText( LPCTSTR text, int align = 1 );
	void SetTextColor( unsigned long color );

	/// copy & paste 권한 주기 여부 
	void SetCopyPaste( bool flag );

	/// 전체 선택 기능
	void SetAllSelect();

	bool IsFocus() const;	/// 포커스가 있는지 리턴 
	bool IsText() const;	/// 에디트 내부에 텍스트가 있는지 리턴

protected:
	/// 이벤트 처리
	virtual bool OnCreate( cUINodeProperty* );
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnImeStartComposition( unsigned int wparam, unsigned int lparam );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool /*AltPress*/, bool /*shiftPress*/ );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual void OnLButtonUp( const cUIPos& pos );
	virtual void OnKeyDown( eKeyCode code );
	virtual void OnKeyUp( eKeyCode code );
	virtual void OnFocusLost();
	virtual void OnMouseLeft(const cUIPos& pos );
	virtual void OnCaptureLost( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateRect();

	/// 문자열을 패스워드형으로 반환한다 
	void ChangePassword();

	/// 캐럿 보이는 시간 계산 
	void UpdateCaretTime( unsigned long accumTime );

	/// 텍스트 좌표 업데이트 
	void UpdateText();

	/// 텍스트 스크롤과, 에디트에 뿌려지는 텍스트 갱신 
	void UpdateShowText();

	/// 드래깅시 백이미지 위치, 사이즈 설정 
	void UpdateBackImage( const cUIPos& pos );

	/// 에디트상에 마우스 이벤트가 일어날시, 눌린좌표의 글자 인덱스를 구함
	void UpdateMouseMoveInEdit( const cUIPos& pos, int& viewcaret, int& editcaret, bool directionfla );

	/// 캐럿 계산 
	void RenderCaret(cUIFontItemKeeper* pKeeper);

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
	cStringT mShowText;

	/// 현재 출력되는 문자열 길이 
	int mShowLen;

	unsigned long mTextColor;
};

inline bool cEditBox::IsFocus() const
{
	if( GetFocus() == this )
	{
		return true;
	}
	return false;
}

inline bool cEditBox::IsText() const
{
	if( mTextLength > 0 )
		return true;
	return false;
}

inline void cEditBox::SetCopyPaste( bool flag )
{
	mCopyFlag = flag;
}

inline void cEditBox::SetTextColor( unsigned long color )
{
	mTextColor = color;
}

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
