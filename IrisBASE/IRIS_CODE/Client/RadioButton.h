/* ==========================================================================
* 파    일 : RadioButton.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Button.h"
#include "UINodeData.h"

class cRadioButton : public cButton
{
public:
	cRadioButton( eUINodeType type = eUINODE_BUTTON );
	virtual ~cRadioButton();

	virtual void SetEnabled( bool enabled );

	/// 눌림 설정
	virtual void SetPress( bool press, bool onsound = true, bool exceptEnable = false );

	/// 이벤트 없이 눌림 설정
	void SetPressNoEvent( bool press, bool onsound = true );

	/// 라디오 버튼 그룹 연결
	void Link( cUINode* node );

	/// 그룹 넘버 리턴 
	int GetGroupNum() const;

	/// 라디오버튼의 텍스트 리턴 
	LPCTSTR GetText();

	cRadioButton* GetNext() { return mpNext; }

	void Hide(); 

private:
	/// 이벤트 처리
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnLButtonDoubleClick( const cUIPos& pos ) {}
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void OnMouseHovered( const cUIPos& /*pos*/ );
	virtual void OnMouseLeft( const cUIPos& /*pos*/ );
	virtual void OnLButtonUp( const cUIPos& /*pos*/ ) {}
	virtual void OnMouseMove( const cUIPos& /*pos*/ ) {}

	/// 텍스트 좌표 업데이트 
	virtual void UpdateText();

	/// 라디오 그룹 적용 
	void UpdateGroup();

	/// 눌림을 적용 
	void AdjustPress();

private:
	/// 라디오 버튼의 그룹 번호 지정 
	int mGroupNum;
	cRadioButton* mpPrev;
	cRadioButton* mpNext;

	unsigned long mDownSoundIdx;
};

inline int cRadioButton::GetGroupNum() const
{
	return mGroupNum;
}


////////////////////////////////////////////////////////////////////////////
/// 라디오 버튼 스킨
class cRadioButtonSkin : public cUINodeSkin
{
public:
	cRadioButtonSkin( eUINodeType type = eUINODE_RADIOBUTTON );
	virtual ~cRadioButtonSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 텍스트
	cStringT mText;

	/// 버튼의 이미지 변환좌표 저장 
	cUIPos mUpPos;
	cUIPos mDownPos;
	cUIPos mUpOverPos;

	/// 각 버튼상태에 따른 텍스트 컬러 
	unsigned long mUpColor;
	unsigned long mDownColor;

	/// 라디오 버튼의 그룹 번호 
	int mGroupNum;

	unsigned long mDownSoundIdx;
};

