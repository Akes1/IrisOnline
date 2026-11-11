/* ==========================================================================
* 파    일 : CheckBox.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.13
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UINode.h"
#include "UINodeData.h"

/// 체크 박스
class cCheckBox : public cUINode
{
public:
	cCheckBox( eUINodeType type = eUINODE_CHECKBOX );
	virtual ~cCheckBox();

	virtual void SetPress( bool press );

protected:
	/// 적합 여부를 리턴
	virtual bool IsValid() const;

	/// 이벤트를 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime ) {}
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnMouseMove( const cUIPos& pos ) {}
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool /*AltPress*/, bool /*shiftPress*/ );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );

protected:
	/// 체크된 이미지 좌표
	cUIPos mCheckPos;
	/// 체크 안된 이미지 좌표
	cUIPos mNoCheckPos;

	/// 텍스처 너비
	unsigned int mOriginalTexWidth, mOriginalTexHeight;

	/// 사운드인덱스
	unsigned long mCheckSoundIdx;
	unsigned long mNoCheckSoundIdx;
};

/////////////////////////////////////////////////////////////////////
/// 체크 박스 스킨
class cCheckBoxSkin : public cUINodeSkin
{
public:
	cCheckBoxSkin( eUINodeType type = eUINODE_CHECKBOX );
	virtual ~cCheckBoxSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 체크 눌림 여부
	bool mPressed;

	/// 체크된 이미지 좌표
	cUIPos mCheckPos;
	/// 체크 안된 이미지 좌표
	cUIPos mNoCheckPos;

	unsigned long mCheckSoundIdx;
	unsigned long mNoCheckSoundIdx;
};
