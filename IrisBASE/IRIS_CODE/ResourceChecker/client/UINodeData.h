/* ==========================================================================
* 파    일 : UINodeData.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-13
* 주의사항 :
*===========================================================================*/
#pragma once

#include "UIRect.h"
#include "UINode.h"

class cUISkin;
class cToken;
class cParser;

struct sSkinInfo
{
	/// 1024 * 768 해상도를 기준으로 한 위치 (단위: 픽셀)
	int mX;
	int mY;

	/// 해상도에 관계없이 일정한 크기 (단위: 픽셀)
	unsigned int mWidth;
	unsigned int mHeight;

	/// 텍스처 이미지 상의 위치, 크기 (단위: 픽셀)
	unsigned int mTexX;
	unsigned int mTexY;
	unsigned int mTexWidth;
	unsigned int mTexHeight;

	unsigned long	mHoverTipIndex;

	sSkinInfo()
	{
		mX = 0;
		mY = 0;

		mWidth = 0;
		mHeight = 0;

		mTexX = 0;
		mTexY = 0;
		mTexWidth = 0;
		mTexHeight = 0;

		mHoverTipIndex = 0;
	}

};

/// UI 노드 스킨
/// UI 노드 스킨 오브젝트들의 최상위 추상 기반 클래스이다.
class cUINodeSkin
{
	friend class cUISkin;

public:
	cUINodeSkin( eUINodeType type = eUINODE_NULL );
	virtual ~cUINodeSkin();

	/// 로딩
	virtual bool Load( cParser& parser ) = 0;

	/// 타입을 리턴
	eUINodeType GetType() const;

	/// 같은 종류인지 여부를 리턴
	bool IsKindof( eUINodeType type ) const;

	/// 적합 여부를 리턴
	virtual bool IsValid() const;

protected:
	/// 한 줄 파싱
	bool ParseLine( cParser& parser, cToken& token );

public:
	/// 색
	NiColorA mColor;

	/// 텍스처
	NiTexture* mpTexture;

	/// 스킨 정보
	sSkinInfo* mSkinInfo;

protected:
	/// 타입
	eUINodeType mType;

	/// 텍스트 기본컬러
	unsigned long mDefaultColor;
};

inline eUINodeType cUINodeSkin::GetType() const
{
	return mType;
}

inline bool cUINodeSkin::IsKindof( eUINodeType type ) const
{
	return (mType & type) == type;
}
