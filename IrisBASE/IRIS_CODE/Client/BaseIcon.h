/* ==========================================================================
* 작 성 자 : 김가람
* 작 성 일 : 2009.06.05
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UINode.h"
#include "UIImage.h"
#include "UINodeData.h"

enum eIconType
{
	eICON_NORMAL = 0,	/// 일반타입
	eICON_ITEM,
	eICON_SKILL,
	eICON_COMMUNITY,
	eICON_UNION,		/// 파티연합용
	eICON_SHORTCUT,		/// 단축키용

	eICON_MAX,
};

/// 아이콘 인자
class cIconParam
{
public:
	/// 텍스처
	NiTexture* mpTexture;

	/// 텍스처 위치 좌표
	cUIPos mTexPos;

	cIconParam()
	{
		mpTexture = 0;
		mTexPos.mX = 0;
		mTexPos.mY = 0;
	}
};

/// 기본 아이콘
class cBaseIcon : public cUINode
{
public:
	cBaseIcon( eUINodeType type = eUINODE_ICON );
	virtual ~cBaseIcon();

	/// 아이콘 활성화 (이벤트는 받되, 비활성화 )
	void SetUsedFlag( bool enable, bool colorchange = true );
	inline bool IsEnableUse() { return mEnableUse; }

	/// 이미지
	void ChangeImage( cIconParam* param, unsigned int texW = 0, unsigned int texH = 0 );
	void ClearImage();

	/// 색상
	void SetColor( const NiColorA& color );
	void SetColor( float r, float g, float b, float a );
	const NiColorA& GetColor() const;

	/// 아이콘 인자
	inline cIconParam* GetIconParam() { return mParam; }

	void SetIconType( eIconType type );
	unsigned int GetIconType();

protected:
	virtual bool SetSkin( const cUINodeSkin* skin );
	virtual void UpdateRect();

	/// 이벤트
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime ) {}
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnMouseHovered(const cUIPos& pos );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnMouseLeft(const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnLButtonUp( const cUIPos& pos );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual void OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );

protected:
	/// 아이콘 타입 
	int mIconType;

	/// 아이콘 인자
	cIconParam* mParam;

	/// 쉬프트 눌림
	bool mShiftDown;

	/// 아이콘활성화 여부 (이동가능)
	bool mEnableUse;

	NiColorA mColor;
};

inline void cBaseIcon::SetIconType( eIconType type )
{
	mIconType = type;
}

inline unsigned int cBaseIcon::GetIconType()
{
	return mIconType;
}

inline void cBaseIcon::SetColor( float r, float g, float b, float a )
{
	SetColor( NiColorA(r,g,b,a) );
}

inline void cBaseIcon::SetColor( const NiColorA& color )
{
	mColor = color;

	if( mpImage )
		mpImage->SetColor( color );
}

inline const NiColorA& cBaseIcon::GetColor() const
{
	if( mpImage )
		return mpImage->GetColor();

	return NiColorA::WHITE;
}

////////////////////////////////////////////////////////////////////////////////
/// 아이콘 스킨
class cIconSkin : public cUINodeSkin
{
public:
	cIconSkin( eUINodeType type = eUINODE_ICON );
	virtual ~cIconSkin();

	/// 로딩
	virtual bool Load( cParser& parser );
};
