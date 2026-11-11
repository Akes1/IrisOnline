/* ==========================================================================
* 파    일 : MultiEditBox.h
* 목    적 : 멀티라인 에디트박스 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-15
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIIMENode.h"

class cMultiEditBox : public cUIIMENode
{
public:
	cMultiEditBox( eUINodeType type = eUINODE_MULTIEDITBOX );
	virtual ~cMultiEditBox();

	/// 텍스트 세팅 
	void SetText( LPCTSTR text, eText_Align align = eALIGN_LEFT );

	void SetTextColor( unsigned long color ){ mDefaultColor = color; }

protected:
	/// 적합 여부를 리턴
	virtual bool OnCreate( cUINodeProperty* );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnKeyDown( eKeyCode code );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool /*AltPress*/, bool /*shiftPress*/ );
	virtual void OnMouseLeft( const cUIPos& pos );
	
	/// 텍스트 관련 데이터 저장 
	void SetTextData();

	/// 텍스트 렌더 및 뿌릴 위치 설정 
	void RenderText( cUIFontItemKeeper* pKeeper );

	/// 캐럿 계산 
	void RenderCaret(cUIFontItemKeeper* pKeeper );

	/// 캐럿 보이는 시간 계산 
	void UpdateCaretTime( unsigned long accumTime );

	/// 해당 좌표에 있는 문자를 찾아낸다. ( 마우스로 편집 좌표 옮기기 )
	void FindTextCaret( const cUIPos& pos );

private:
	/// 텍스트 라인 저장 ( 각 글자마다 어느 라인에 속하는지 저장 )
	typedef tArray<int> cTextLine;
	cTextLine mTextLineArray;

	/// 각 라인별 포함하는 인덱스 저장 
	/// ex ) 0 번라인은 11번 인덱스까지, 1번라인은 12번부터 23번까지 
	typedef tArray<int> cTextLen;
	cTextLen mTextIndexArray;
	
	/// 캐럿 시간 플래그 
	bool mCaretTimeFlag;

	/// 캐럿 플래그
	bool mShowCaretFlag;

	/// 캐럿 시간용
	unsigned long mLastTick;

	/// 에디트 박스에서 글자 출력 시작위치
	cUIPos mEditingPos;

	/// 에디트의 위치 
	int mEditX, mEditY;

	/// 에디트의 출력되는 문자열 최대 width
	unsigned int mEditWidth;

	/// 텍스트 정렬
	int mTextAlign;

	/// 실제로 에디트에 뿌려지는 텍스트 
	TCHAR mShowText[MAX_TEXT_LENGTH];

	/// 에디트탭인덱스 (탭이 눌렸을시에 포커스를 가지기 위한 할당된 번호 )
	int mEditTabIndex;
};
