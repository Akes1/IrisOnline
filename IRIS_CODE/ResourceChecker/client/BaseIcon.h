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
