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

	/// 문자열 클리어 
	void Clear();

	/// 외부에서 값을 세팅 ( 정수형, 실수형 등 )
	void SetValue( int value );

	/// 외부에서 텍스트를 세팅
	void SetText( LPCTSTR text );

	/// 텍스트 컬러 세팅 
	void SetTextColor( long textColor );

	/// 목표윈도우에 정렬방법을 정해 정렬
	void SetAlign( cUINode* pnode, int align = 0 );
	void SetTextPos( cUIPos pos );

	///
	LPCTSTR GetText() const	{ return mText.Cstr(); }
	
	cUINodeSkin* GetNodeSkin() { return mpSkin; }

	/// 회전
	virtual void SetTranslate( short x, short y );
	virtual void SetRotate( float angle );

	/// 해당스킨으로 이미지 바꾸기
	void ChangeImage( cUINodeSkin* pSkin );
	void ChangeImage( unsigned int texX, unsigned int texY, unsigned int texW, unsigned int texH );
	void ChangeImage( unsigned int texX, unsigned int texY );
	void SetTexture( NiTexture* pTexture, unsigned int texX, unsigned int texY );
	void SetHoverTip( unsigned long tipIndex );

	virtual void EnableBoldText( bool set );

	// 폰트 스타일
	void SetFontIndex( unsigned short fontIndex );

protected:
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void OnMouseHovered(const cUIPos& pos );
	virtual void OnMouseLeft(const cUIPos& pos );
	virtual void UpdateRect();

	void UpdateText();
	
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

inline void cLabel::SetTextColor( long textColor )
{
	mTextColor = textColor;
}

inline void cLabel::SetAlign( cUINode* pnode, int align )
{
	mDescNode = pnode;
	mTextAlign = align;
	UpdateText();
}

inline void cLabel::SetTextPos( cUIPos pos )
{
	mEditPos = pos;
	UpdateText();
}

inline void cLabel::SetFontIndex( unsigned short fontIndex )
{
	mFontIndex = fontIndex;
}

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
